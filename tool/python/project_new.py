"""Generate an independent FLY application by composing local template packs."""

from __future__ import annotations

import argparse
from importlib.util import find_spec
from itertools import combinations
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
from urllib.parse import urlsplit

from build_environment import prepare_python
from mk_mcu_firmware_factory_data import FC_FIRMWARE_NAME_MAX_SIZE
from template_packs import (
    ACTIONS, DEFCONFIG_TOKEN, WINDOWS_RESERVED, AvailablePacks, assignment, board_values,
    dependency_order, discover, from_registry, local_path, project_pack_path, require,
)
from vscode_project import write_vscode_scaffold


PROJECT_NAME = re.compile(r"[A-Za-z0-9][A-Za-z0-9_-]*")
GIT_COMMIT = re.compile(r"[0-9a-f]{40}(?:[0-9a-f]{24})?")
SSH_ORIGIN = re.compile(r"[\w.-]+@([\w.-]+):([^\s]+)")


def add_target_options(parser, registry):
    parser.add_argument("--board", choices=[p.id.split("/")[1] for p in registry.values() if p.kind == "board"])
    parser.add_argument("--template", choices=[p.id.split("/")[1] for p in registry.values() if p.kind == "app"])


def add_parser(subparsers, registry):
    parser = subparsers.add_parser("new", help="Generate an independent FLY project.")
    parser.add_argument("name", nargs="?", help="Project name. Omit all selections to open the project menu.")
    add_target_options(parser, registry)
    tools = parser.add_mutually_exclusive_group()
    tools.add_argument("--pack", action="append", choices=[p.id for p in registry.values() if p.kind == "tool"],
                       help="Choose a tool pack. Repeat to select more.")
    tools.add_argument("--no-tools", action="store_true", help="Generate without any tool packs.")
    parser.add_argument("--default", metavar="ACTION=PROVIDER", action="append", default=[],
                        help="Set an explicit provider default. Repeat for different actions.")
    parser.add_argument("--output", type=Path,
                        help="Destination directory (default: FLY's sibling fly-project directory). "
                             "In the interactive menu this is the initial project path.")
    catalog = subparsers.add_parser("packs", help="List discovered packs, capabilities and actions.")
    add_target_options(catalog, registry)


def parse_defaults(items):
    defaults = {}
    for item in items:
        action, sep, provider = item.partition("=")
        if not sep or action not in ACTIONS or not provider or action in defaults:
            raise ValueError(f"Expected a unique ACTION=PROVIDER default, got {item!r}")
        defaults[action] = provider
    return defaults


def list_packs(registry, args, scaffold):
    packs = registry.values()
    if args.board or args.template:
        available = AvailablePacks(registry, lambda composition: project_inputs(scaffold, composition))
        pairs = [
            (board.id, app.id)
            for board in packs if board.kind == "board" and (not args.board or board.id == f"board/{args.board}")
            for app in packs if app.kind == "app" and (not args.template or app.id == f"app/{args.template}")
            if available.is_available(board.id, app.id)
        ]
        if not pairs:
            print("No compatible board and application combination.", file=sys.stderr)
            return 1
        ids = {item for pair in pairs for item in pair}
        ids.update(
            tool for tool in available.tools if any(available.is_available(*pair, tool) for pair in pairs)
        )
        ids.update(pack.id for pack in dependency_order(registry.__getitem__, sorted(ids)))
        packs = (pack for pack in packs if pack.id in ids)
    for pack in packs:
        print(f"{pack.id} — {pack.label}")
        for key in ("provides", "requires", "dependencies"):
            if pack.data.get(key):
                print(f"  {key}: {', '.join(pack.data[key])}")
        for action in pack.data.get("actions", ()):
            print(f"  {action['name']} --via {action['provider']}: {action['help']}")
    return 0


def project_destination(path):
    if not str(path).strip():
        raise ValueError("Enter a project path.")
    try:
        output = Path(path).expanduser()
    except RuntimeError as error:
        raise ValueError(str(error)) from error
    if not output.name or output.name == "..":
        raise ValueError("The project path must name a new directory.")
    return output.parent.resolve() / output.name


def project_name_from_path(destination):
    name = destination.name
    if not PROJECT_NAME.fullmatch(name):
        name = re.sub(r"[^A-Za-z0-9_-]+", "-", name).strip("-_")
    name = name[:FC_FIRMWARE_NAME_MAX_SIZE]
    return name if name and not WINDOWS_RESERVED.fullmatch(name) else "fly-app"


def write_menu(path, registry, scaffold):
    def quote(label):
        return json.dumps(label, ensure_ascii=False)

    # Read the initial path as data so Kconfig does not expand macros in it.
    lines = ['mainmenu "FLY New Application"', '', 'config FLY_NEW_INITIAL_PATH', '    string',
             '    option env="FLY_NEW_INITIAL_PATH"', '', 'config NEW_PATH', '    string "Project path"',
             '    default FLY_NEW_INITIAL_PATH', '    help',
             "      Use an absolute path, a path relative to the current directory, or ~/path.", ""]
    choices = [key for key, pack in registry.items() if pack.kind != "support"]
    symbols = {key: f"FLY_NEW_PACK_{index}" for index, key in enumerate(choices)}
    boards = [pack for pack in registry.values() if pack.kind == "board"]
    apps = [pack for pack in registry.values() if pack.kind == "app"]

    def condition(pairs):
        return " || ".join(f"({symbols[b]} && {symbols[a]})" for b, a in pairs) or "n"

    def add_help(pack, actions=()):
        details = [
            f"{label}: {', '.join(pack.data[key])}"
            for key, label in (("provides", "Provides"), ("requires", "Requires"))
            if pack.data.get(key)
        ]
        details.extend(action["help"] for action in actions)
        if details:
            lines.extend(("    help", *(f"      {hint}" for hint in details)))

    def choice(title, packs, filters=None):
        lines.extend(("choice", f"    prompt {quote(title)}"))
        for pack in packs:
            label = pack.label
            if pack.kind == "board":
                label += f" ({pack.data['board']['core']['id']})"
            lines.extend((f"config {symbols[pack.id]}", f"    bool {quote(label)}"))
            if filters:
                lines.append(f"    depends on {filters(pack)}")
            add_help(pack)
        lines.extend(("endchoice", ""))

    available = AvailablePacks(registry, lambda composition: project_inputs(scaffold, composition))
    valid_applications = {}
    for board in boards:
        for app in apps:
            if available.is_available(board.id, app.id):
                valid_applications.setdefault(app.id, []).append(board.id)

    valid_boards = {board for compatible_boards in valid_applications.values() for board in compatible_boards}
    if not valid_boards:
        raise ValueError("No compatible board and application combination.")
    choice("Board", boards, lambda board: "y" if board.id in valid_boards else "n")
    choice("Application", apps, lambda app: " || ".join(
        symbols[board] for board in valid_applications.get(app.id, ())
    ) or "n")

    providers = {}
    prior_tools = {}
    tool_options = []
    for pack in (pack for pack in registry.values() if pack.kind == "tool"):
        valid_pairs = [
            (board, app) for app, compatible_boards in valid_applications.items()
            for board in compatible_boards if available.is_available(board, app, pack.id)
        ]
        depends = f"({condition(valid_pairs)})"
        for earlier, earlier_pairs in prior_tools.items():
            invalid = [
                pair for pair in valid_pairs
                if pair in earlier_pairs and not available.is_available(*pair, earlier, pack.id)
            ]
            if invalid:
                depends += f" && !({symbols[earlier]} && ({condition(invalid)}))"
        prior_tools[pack.id] = set(valid_pairs)
        tool_options.append((pack, depends))
    if tool_options:
        tool_pairs = sorted(set().union(*prior_tools.values()))
        lines.extend(('menu "Tools"', f"    visible if {condition(tool_pairs)}"))
        for pack, depends in tool_options:
            lines.extend((f"config {symbols[pack.id]}", f"    bool {quote(pack.label)}", f"    depends on {depends}"))
            actions = [action for dependency in dependency_order(registry.__getitem__, [pack.id])
                       for action in dependency.data.get("actions", ())] if prior_tools[pack.id] else []
            add_help(pack, actions)
            for action in actions:
                providers.setdefault(action["name"], {}).setdefault(action["provider"], []).append(symbols[pack.id])
        lines.extend(("endmenu", ""))
    defaults = {}
    default_choices = []
    visible_choices = []
    for index, (action, action_providers) in enumerate(sorted(providers.items())):
        if len(action_providers) < 2:
            continue
        selected = {provider: f"({' || '.join(packs)})"
                    for provider, packs in sorted(action_providers.items())}
        multiple_selected = " || ".join(f"({left} && {right})"
                                        for left, right in combinations(selected.values(), 2))
        visible_choices.append(f"({multiple_selected})")
        auto = f"FLY_NEW_AUTO_{index}"
        default_choices.extend((
            "choice", f'    prompt "Default provider for {action}"',
            f"    depends on {multiple_selected}", f"    default {auto}",
            f"config {auto}", '    bool "Automatic"',
            "    help", "      Leave the default unset and choose with --via when running this action.",
        ))
        for j, (provider, dependency) in enumerate(selected.items()):
            symbol = f"FLY_NEW_DEFAULT_{index}_{j}"
            defaults[symbol] = action, provider
            default_choices.extend((f"config {symbol}", f"    bool {quote(provider)}", f"    depends on {dependency}"))
        default_choices.extend(("endchoice", ""))
    if default_choices:
        lines.extend(('menu "Default providers by action"', f"    visible if {' || '.join(visible_choices)}",
                      *default_choices, "endmenu", ""))
    path.write_text("\n".join(lines), encoding="utf-8")
    return symbols, defaults


def select_interactively(registry, scaffold, default_path=None):
    try:
        import kconfiglib
        import menuconfig
    except ImportError as error:
        raise ValueError(f"Project menu unavailable ({error}). On Windows install windows-curses for this Python.") from error

    if default_path is None:
        default_path = scaffold.parents[2] / "fly-project" / "my-app"
    with tempfile.TemporaryDirectory(prefix="fly-new-") as temp:
        config = Path(temp) / "Kconfig"
        saved = Path(temp) / ".config"
        symbols, available_defaults = write_menu(config, registry, scaffold)
        menu_env = {"KCONFIG_CONFIG": str(saved), "FLY_NEW_INITIAL_PATH": str(default_path)}
        previous = {key: os.environ.get(key) for key in menu_env}
        os.environ.update(menu_env)
        try:
            while True:
                menuconfig.menuconfig(kconfiglib.Kconfig(str(config), suppress_traceback=True))
                if not saved.is_file():
                    return None
                selected = kconfiglib.Kconfig(str(config), suppress_traceback=True)
                selected.load_config(str(saved))
                packs = [pack_id for pack_id, symbol in symbols.items() if selected.syms[symbol].tri_value == 2]
                defaults = {action: provider for symbol, (action, provider) in available_defaults.items()
                            if selected.syms[symbol].tri_value == 2}
                try:
                    destination = project_destination(selected.syms["NEW_PATH"].str_value)
                    project_inputs(scaffold, from_registry(registry, packs, defaults))
                except (OSError, ValueError) as error:
                    print(f"Invalid project configuration: {error}", file=sys.stderr)
                    if input("Return to the project menu to revise the selection? [Y/n] ").strip().lower() not in (
                            "", "y", "yes"):
                        return None
                else:
                    return destination, packs, defaults
        finally:
            for key, value in previous.items():
                if value is None:
                    os.environ.pop(key, None)
                else:
                    os.environ[key] = value


def shareable_origin(origin):
    if not origin or any(character.isspace() for character in origin):
        return None
    ssh_origin = SSH_ORIGIN.fullmatch(origin)
    if ssh_origin:
        host, path = ssh_origin.groups()
        if host.lower() == "github.com":
            repository = path.lstrip("/")
            if re.fullmatch(r"[\w.-]+/[\w.-]+", repository):
                return f"https://github.com/{repository}"
            return None
        return origin
    try:
        url = urlsplit(origin)
        if not url.hostname or not url.path or url.query or url.fragment or url.password:
            return None
        if url.scheme in ("http", "https") and url.username is None:
            return origin
        if url.scheme in ("ssh", "git"):
            return origin
    except ValueError:
        pass
    return None


def git_output(sdk_dir, *args):
    result = subprocess.run(
        ["git", "-C", str(sdk_dir), *args], capture_output=True, text=True,
    )
    return result.stdout.strip() if result.returncode == 0 else ""


def sdk_lock(sdk_dir):
    # Read the configured URL before Git applies local URL rewrites.
    origin = shareable_origin(git_output(sdk_dir, "config", "--local", "--get", "remote.origin.url"))
    commit = git_output(sdk_dir, "rev-parse", "--verify", "HEAD")
    entry = git_output(sdk_dir, "cat-file", "-t", "HEAD:cmake/fly-configure.cmake")
    if not GIT_COMMIT.fullmatch(commit) or entry != "blob":
        commit = None
    return {"url": origin, "commit": commit}


def pack_files(scaffold, composition):
    files = list(scaffold.rglob("*"))
    destination_names = {item.relative_to(scaffold).as_posix().casefold() for item in files if item.is_file()}
    reserved = {
        "defconfig", "fly", "fly.lock", "fly.project.json", ".fly-root", ".git",
        ".vscode/settings.json", ".vscode/tasks.json",
        *(project_pack_path(pack.id) for pack in composition.packs),
    }
    destination_names.update(reserved)
    for item in files:
        local_path(scaffold, item.relative_to(scaffold).as_posix(), scaffold)
    for pack in composition.packs:
        for item in pack.data["files"]:
            relative = item["destination"]
            destination = Path(relative.casefold())
            require(relative.casefold() not in destination_names and
                    not any(ancestor.as_posix() in destination_names for ancestor in destination.parents) and
                    not any(relative_name.startswith(destination.as_posix() + "/") for relative_name in destination_names),
                    pack.path, f"Destination would overwrite a scaffold file or another pack: {relative}")
            source = local_path(pack.path.parent, item["source"], pack.path)
            require(source.is_file(), pack.path, f"Pack source file missing: {item['source']}")
            destination_names.add(destination.as_posix())
            yield relative, source


def render_defconfig(composition, files):
    template = composition.app.data["app"]
    lines = []
    if "board/defconfig" in files:
        lines.extend(files["board/defconfig"].read_text(encoding="utf-8").splitlines())

    support_defaults = {}
    for pack in composition.packs:
        for line in pack.data.get("config_defaults", []):
            symbol = assignment(line)
            require(symbol not in support_defaults or line == support_defaults[symbol], pack.path,
                    f"Conflicting support defaults for {symbol}")
            support_defaults[symbol] = line
    lines.extend(support_defaults.values())
    template_config = "\n".join(template["config_defaults"])
    values = board_values(composition.board, composition.app)
    lines.extend(DEFCONFIG_TOKEN.sub(lambda match: values[match[1]], template_config).splitlines())
    resolved = {}
    for line in lines:
        symbol = assignment(line)
        if symbol:
            resolved[symbol] = line
        elif line.strip() and not line.startswith("#"):
            raise ValueError(f"Invalid board defconfig entry: {line}")
    return "\n".join(resolved.values()) + "\n"


def project_inputs(scaffold, composition):
    copies = dict(pack_files(scaffold, composition))
    return copies, render_defconfig(composition, copies)


def write_json(path, content):
    path.write_text(json.dumps(content, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def generate(sdk_dir, destination, name, composition, selection, defaults, copies, config):
    lock = sdk_lock(sdk_dir)
    destination.parent.mkdir(parents=True, exist_ok=True)
    # Reserve the name before writing so another generator's output stays intact.
    destination.mkdir()
    try:
        shutil.copytree(sdk_dir / "templates/scaffold", destination, dirs_exist_ok=True)
        for relative, source in copies.items():
            path = destination / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, path)
        for pack in composition.packs:
            path = destination / project_pack_path(pack.id)
            path.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(pack.path, path)
        write_json(destination / "fly.project.json", {"schema_version": 1, "packs": selection, "defaults": defaults})
        write_vscode_scaffold(destination)
        (destination / "defconfig").write_text(config, encoding="utf-8")
        cmake_file = destination / "CMakeLists.txt"
        cmake_file.write_text(cmake_file.read_text(encoding="utf-8").replace("@PROJECT_NAME@", name), encoding="utf-8")

        readme = destination / "README.md"
        content = readme.read_text(encoding="utf-8")
        board = composition.board.data["board"]
        outputs = composition.app.data["app"]["expected_log"]
        examples = ", ".join(f"`{line}`" for line in outputs)
        startup_log_note = f"When logging is enabled, expected output includes {examples}. " if examples else ""
        selected_tools = [pack.label for pack in composition.packs if pack.kind == "tool"]
        fields = {
            "@PROJECT_NAME@": name, "@BOARD_LABEL@": composition.board.label, "@BOARD_CORE@": board["core"]["id"],
            "@TOOLCHAIN@": board["toolchain"], "@PROBE@": ", ".join(selected_tools) or "none",
            "@TEMPLATE_ID@": composition.app.id.split("/")[1], "@STARTUP_LOG_NOTE@": startup_log_note,
        }
        for token, value in fields.items():
            content = content.replace(token, value)
        readme.write_text(content, encoding="utf-8")

        write_json(destination / "fly.lock", lock)
        if os.name == "nt":
            (destination / ".fly-root").write_text(f"{sdk_dir}\n", encoding="utf-8")
        else:
            (destination / "build.sh").chmod(0o755)
            (destination / "fly").symlink_to(os.path.relpath(sdk_dir, destination), target_is_directory=True)
        subprocess.run(["git", "init", "--quiet"], cwd=destination, check=True)
        # The Git index keeps the launcher executable on filesystems without mode bits.
        subprocess.run(["git", "add", "--chmod=+x", "--", "build.sh"], cwd=destination, check=True)
    except BaseException:
        shutil.rmtree(destination)
        raise
    return lock


def run(sdk_dir, registry, args):
    scaffold = sdk_dir / "templates/scaffold"
    empty_selections = not any((args.name, args.board, args.template, args.pack, args.no_tools, args.default))
    interactive = empty_selections and sys.stdin.isatty() and sys.stdout.isatty()
    try:
        if interactive:
            if find_spec("kconfiglib") is None:
                interpreter = prepare_python(sdk_dir / ".cache/new")
                if interpreter is not None:
                    return subprocess.call([str(interpreter), "-B", __file__, *sys.argv[1:]])
            picked = select_interactively(registry, scaffold, args.output)
            if picked is None:
                print("Project generation cancelled.")
                return 0
            destination, selection, defaults = picked
            name = project_name_from_path(destination)
        else:
            required = {"<name>": args.name, "--board": args.board, "--template": args.template}
            missing = [key for key, value in required.items() if value is None]
            if missing:
                raise ValueError(f"Missing {', '.join(missing)}. Run 'fly.sh new' in a terminal to use menuconfig.")
            name = args.name
            if not PROJECT_NAME.fullmatch(name) or WINDOWS_RESERVED.fullmatch(name):
                raise ValueError("Use a portable project name with letters, digits, '-' or '_', starting with a letter or digit.")
            output = args.output if args.output is not None else sdk_dir.parent / "fly-project" / name
            destination = project_destination(output)
            board, app = f"board/{args.board}", f"app/{args.template}"
            selection = [board, app, *(args.pack or [])]
            defaults = parse_defaults(args.default)
        composition = from_registry(registry, selection, defaults)
        copies, config = project_inputs(scaffold, composition)
        if interactive:
            print(f"Project: {name}\nDestination: {destination}\nPacks: {', '.join(selection)}")
            print(f"Defaults: {', '.join(f'{k}={v}' for k, v in defaults.items()) or 'automatic'}")
            if input("Create this project? [y/N] ").strip().lower() not in ("y", "yes"):
                print("Project generation cancelled.")
                return 0
        lock = generate(sdk_dir, destination, name, composition, selection, defaults, copies, config)
    except FileExistsError as exc:
        print(f"Cannot create project because a path already exists: {exc.filename}", file=sys.stderr)
        return 1
    except (ValueError, OSError, EOFError, subprocess.CalledProcessError) as exc:
        print(f"Cannot create project: {exc}", file=sys.stderr)
        return 1
    except KeyboardInterrupt:
        print("\nProject generation cancelled.", file=sys.stderr)
        return 130

    print(f"Created {destination}")
    print(f"{composition.board.label} | {composition.app.label}")
    if lock["commit"] is None or git_output(sdk_dir, "status", "--porcelain"):
        print("Before sharing this project, publish a compatible FLY commit and update fly.lock.", file=sys.stderr)
    if lock["url"] is None:
        print("Set a shareable FLY repository URL in fly.lock before sharing this project.", file=sys.stderr)
    print("To build, run ./build.sh (or build.bat on Windows) from the new project.")
    return 0


def main():
    sdk_dir = Path(__file__).resolve().parents[2]
    try:
        registry = discover(sdk_dir / "templates")
        if not {"board", "app"} <= {pack.kind for pack in registry.values()}:
            raise ValueError("At least one board and one application pack are required.")
    except (ValueError, OSError) as error:
        print(f"Cannot load templates: {error}", file=sys.stderr)
        return 1
    parser = argparse.ArgumentParser(prog="fly", description=__doc__)
    add_parser(parser.add_subparsers(dest="command", required=True), registry)
    args = parser.parse_args()
    if args.command == "packs":
        return list_packs(registry, args, sdk_dir / "templates/scaffold")
    return run(sdk_dir, registry, args)


if __name__ == "__main__":
    sys.exit(main())
