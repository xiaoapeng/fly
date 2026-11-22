"""Validate and compose the local FLY template-pack registry."""

from __future__ import annotations

from dataclasses import dataclass
import heapq
import json
from pathlib import Path
import re

from action_protocol import ACTIONS
from arch_registry import discover as discover_architectures, resolve_board


PHASE = {"board": 0, "support": 1, "app": 2, "tool": 3}
NAME = re.compile(r"[a-z0-9][a-z0-9-]*")
IDENTIFIER = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
SYMBOL = re.compile(r"CONFIG_[A-Z0-9_]+")
PACKAGE_ID = re.compile(rf"(?:{'|'.join(PHASE)})/{NAME.pattern}")
CAPABILITY = re.compile(r"[a-z0-9][a-z0-9._/-]*")
CONFIG_LINE = re.compile(r"(CONFIG_[A-Z0-9_]+)=.+|# (CONFIG_[A-Z0-9_]+) is not set")
DEFCONFIG_TOKEN = re.compile(r"@([A-Za-z_][A-Za-z0-9_]*)@")
WINDOWS_RESERVED = re.compile(r"(?:CON|PRN|AUX|NUL|COM[0-9]|LPT[0-9])", re.IGNORECASE)


@dataclass(frozen=True)
class Pack:
    path: Path
    data: dict

    @property
    def kind(self):
        return self.data["kind"]

    @property
    def id(self):
        return self.data["id"]

    @property
    def label(self):
        return self.data["label"]


@dataclass(frozen=True)
class Composition:
    board: Pack
    app: Pack
    packs: tuple[Pack, ...]
    actions: dict[tuple[str, str], dict]
    defaults: dict[str, str]


class MissingCapabilities(ValueError):
    def __init__(self, missing, diagnostics):
        self.capabilities = missing
        super().__init__("Missing capabilities: " + " | ".join(diagnostics))


def text(value):
    return isinstance(value, str) and bool(value.strip()) and not any(c in value for c in "\r\n\0")


def matches(pattern, value):
    return text(value) and pattern.fullmatch(value) is not None


def positive_integer(value):
    return type(value) is int and value > 0


def require(condition, path, message):
    if not condition:
        raise ValueError(f"{path}: {message}")


def array(value, path, field, predicate, *, nonempty=False, unique=False):
    require(isinstance(value, list) and (value or not nonempty), path, f"{field} must be an array")
    require(all(map(predicate, value)), path, f"{field} contains an invalid value")
    if unique:
        require(len(set(value)) == len(value), path, f"{field} contains duplicates")
    return value


def fields(value, path, context, required, optional=()):
    require(isinstance(value, dict), path, f"{context} must be an object")
    missing = set(required) - value.keys()
    unknown = value.keys() - set(required) - set(optional)
    require(not missing, path, f"{context} missing fields: {', '.join(sorted(missing))}")
    require(not unknown, path, f"{context} unknown fields: {', '.join(sorted(unknown))}")


def version(value, path):
    require(type(value) is int and value == 1, path, "Unsupported schema_version, expected integer 1")


def read_json(path):
    def unique(pairs):
        result = dict(pairs)
        require(len(result) == len(pairs), path, "Duplicate JSON key")
        return result

    def invalid_constant(value):
        raise ValueError(f"{path}: Invalid JSON constant {value}")

    try:
        return json.loads(path.read_text(encoding="utf-8"), object_pairs_hook=unique, parse_constant=invalid_constant)
    except (json.JSONDecodeError, UnicodeError) as error:
        raise ValueError(f"{path}: {error}") from error


def valid_relpath(value):
    if not text(value) or any(c in value for c in '\\:<>|?*"'):
        return False
    return all(
        segment not in {"", ".", ".."} and not segment.endswith((".", " "))
        and not WINDOWS_RESERVED.fullmatch(segment.split(".")[0])
        for segment in value.split("/")
    )


def local_path(root, value, context):
    require(valid_relpath(value), context, f"Invalid relative path {value!r}")
    candidate = (root / value).resolve()
    require(root.resolve() in candidate.parents, context, f"Path {value!r} escapes {root}")
    return candidate


def assignment(line):
    match = CONFIG_LINE.fullmatch(line)
    return (match[1] or match[2]) if match else None


def config_defaults(value, path):
    entries = array(value, path, "config_defaults", lambda line: text(line) and assignment(line))
    require(len({assignment(line) for line in entries}) == len(entries), path, "Duplicate configuration symbol")


def validate_board(board, path):
    fields(board, path, "board", ("target_kind", "chip", "core", "arch", "toolchain", "clock_hz", "memories"))
    host = board["target_kind"] == "host"
    require(host or board["target_kind"] == "mcu", path, "Invalid board.target_kind")
    require((host and board["chip"] is None) or text(board["chip"]), path, "Invalid board.chip")
    core = board["core"]
    fields(core, path, "board.core", ("id", "architecture"))
    for key in ("id", "architecture"):
        require(matches(NAME, core[key]), path, f"Invalid core.{key}")
    require(not host or core["id"] == "native", path, "Host core.id must be native")
    require(matches(NAME, board["arch"]), path, "Invalid board.arch")
    require(matches(NAME, board["toolchain"]), path, "Invalid board.toolchain")
    require(board["clock_hz"] is None or positive_integer(board["clock_hz"]), path, "Invalid board.clock_hz")
    memories = array(board["memories"], path, "board.memories", lambda value: isinstance(value, dict), nonempty=not host)
    regions = {}
    for region in memories:
        fields(region, path, "memory", ("id", "kind", "address", "size"))
        key = region["id"]
        address = region["address"]
        size = region["size"]
        require(matches(NAME, key) and key not in regions, path, "Invalid or duplicate memory ID")
        require(region["kind"] in ("flash", "ram"), path, f"Invalid memory kind for {key}")
        require(type(address) is int and address >= 0 and positive_integer(size), path, f"Invalid memory range for {key}")
        require(not any(address < end and beginning < address + size for beginning, end in regions.values()),
                path, f"Overlapping memory range for {key}")
        regions[key] = address, address + size


def validate_app(app, path):
    fields(app, path, "app", ("firmware_target", "config_defaults", "expected_log"), ("board_bindings",))
    require(matches(IDENTIFIER, app["firmware_target"]), path, "Invalid app.firmware_target")
    config_defaults(app["config_defaults"], path)
    array(app["expected_log"], path, "app.expected_log", text)
    bindings = app.get("board_bindings", {})
    require(isinstance(bindings, dict), path, "app.board_bindings must be an object")
    tokens = set(DEFCONFIG_TOKEN.findall("\n".join(app["config_defaults"])))
    require(tokens == bindings.keys(), path, "Board bindings must match defconfig placeholders")
    for token, binding in bindings.items():
        fields(binding, path, f"binding {token}", ("field", "type"))
        require(matches(IDENTIFIER, token) and matches(IDENTIFIER, binding["field"]), path, f"Invalid binding {token}")
        require(binding["type"] in ("integer", "positive_integer", "string"), path, f"Invalid binding type for {token}")


def board_values(board, app):
    checks = {"integer": lambda value: type(value) is int, "positive_integer": positive_integer, "string": text}
    values = {}
    for token, binding in app.data["app"].get("board_bindings", {}).items():
        value = board.data["board"].get(binding["field"])
        require(checks[binding["type"]](value), app.path,
                f"{board.id} needs {binding['type']} field {binding['field']!r} for @{token}@")
        values[token] = str(value)
    return values


def category_ids(value, path, field, kind):
    return array(value, path, field, lambda name: matches(PACKAGE_ID, name) and name.startswith(kind + "/"),
                 nonempty=True, unique=True)


def validate_profiles(profiles, inputs, path):
    seen = set()
    for profile in array(profiles, path, "debug_profiles", lambda value: isinstance(value, dict), nonempty=True):
        fields(profile, path, "debug profile", ("id", "input", "mode", "server", "after_connect"),
               ("vscode_fragment", "pre_action"))
        name = profile["id"]
        require(matches(NAME, name) and name not in seen, path, "Invalid or duplicate debug profile ID")
        seen.add(name)
        require(profile["input"] in ("build", "artifact") and profile["input"] in inputs,
                path, f"Profile {name} needs a declared build or artifact input")
        require(profile["mode"] in ("start", "attach") and profile["server"] in ("owned", "external"),
                path, f"Invalid mode or server for profile {name}")
        array(profile["after_connect"], path, "after_connect", text)
        if "vscode_fragment" in profile:
            require(valid_relpath(profile["vscode_fragment"]) and profile["vscode_fragment"].endswith(".json"), path,
                    f"Invalid vscode_fragment for profile {name}")
        if "pre_action" in profile:
            pre = profile["pre_action"]
            fields(pre, path, "pre_action", ("action", "provider"))
            require(profile["input"] == "artifact" and pre["action"] == "flash-run" and matches(NAME, pre["provider"]),
                    path, f"Invalid pre_action for profile {name}")


def validate_actions(data, path):
    seen = set()
    kind = data["kind"]
    for action in array(data.get("actions", []), path, "actions", lambda item: isinstance(item, dict)):
        fields(action, path, "action", ("name", "provider", "handler", "inputs", "help"), ("tool_check", "debug_profiles"))
        name, provider = action["name"], action["provider"]
        require(kind == "tool" and name in ACTIONS and matches(NAME, provider),
                path, f"Invalid action {name!r} for {kind} pack")
        require((name, provider) not in seen, path, f"Duplicate action {name}/{provider}")
        seen.add((name, provider))
        permitted_inputs = {
            "flash-run": ("artifact",), "export": ("artifact",),
            "log": ("artifact", "none"), "debug": ("artifact", "build"),
        }
        inputs = array(action["inputs"], path, "action inputs", lambda value: value in permitted_inputs[name],
                       nonempty=True, unique=True)
        require("none" not in inputs or len(inputs) == 1, path, "A none input must be declared alone")
        require(text(action["help"]), path, "Action help must be nonempty")
        for key in ("handler", "tool_check"):
            if key in action:
                require(valid_relpath(action[key]) and action[key].endswith(".py"), path, f"Invalid action {key}")
        require((name == "debug") == ("debug_profiles" in action), path, "Only debug actions must declare debug_profiles")
        if name == "debug":
            validate_profiles(action["debug_profiles"], inputs, path)


def read_pack(path):
    data = read_json(path)
    category = {
        "board": (("board",), ()),
        "app": (("app",), ()),
        "support": ((), ("config_defaults", "log_sink", "settings")),
        "tool": ((), ("models", "settings")),
    }
    kind = data.get("kind") if isinstance(data, dict) else None
    require(isinstance(kind, str) and kind in category, path, "Unknown pack kind")
    required, optional = category[kind]
    fields(data, path, "pack", ("schema_version", "id", "kind", "label", "files", *required),
           ("cmake", "requires", "provides", "dependencies", "conflicts", "compatibility", "actions", *optional))
    version(data["schema_version"], path)
    require(matches(PACKAGE_ID, data["id"]) and data["id"].startswith(f"{kind}/"),
            path, "Pack ID namespace must match its kind")
    require(valid_relpath(data["id"]), path, "Pack ID must use a portable path")
    require(text(data["label"]), path, "Pack label must be nonempty")
    for key, pattern in (("requires", CAPABILITY), ("provides", CAPABILITY), ("dependencies", PACKAGE_ID),
                         ("conflicts", PACKAGE_ID)):
        array(data.get(key, []), path, key, lambda value: matches(pattern, value), unique=True)
    for capability in data.get("provides", []):
        namespace = capability.split("/", 1)[0]
        owner = {"board": "board", "cpu": "board", "app": "app"}.get(namespace, kind)
        require(owner == kind, path, f"{kind} pack cannot provide {capability!r}")
    for item in array(data["files"], path, "files", lambda value: isinstance(value, dict)):
        fields(item, path, "file", ("source", "destination"))
        for key in item:
            require(valid_relpath(item[key]), path, f"Invalid file {key}: {item[key]!r}")
    if "cmake" in data:
        require(valid_relpath(data["cmake"]) and data["cmake"].endswith(".cmake"), path, "Invalid CMake entry")
    if "compatibility" in data:
        compat = data["compatibility"]
        fields(compat, path, "compatibility", (), ("boards", "apps", "board_data"))
        for key, category_name in (("boards", "board"), ("apps", "app")):
            if key in compat:
                category_ids(compat[key], path, key, category_name)
        if "board_data" in compat:
            records = compat["board_data"]
            require(isinstance(records, dict), path, "compatibility.board_data must be an object")
            require("boards" not in compat, path, "Declare compatibility.boards or compatibility.board_data")
            category_ids(list(records), path, "compatibility.board_data keys", "board")
            require(all(isinstance(item, dict) for item in records.values()), path,
                    "compatibility.board_data values must be objects")
    if "settings" in data:
        require(isinstance(data["settings"], dict), path, "settings must be an object")
    if kind == "board":
        validate_board(data["board"], path)
    if kind == "app":
        validate_app(data["app"], path)
    if "config_defaults" in data:
        config_defaults(data["config_defaults"], path)
    offers_sink = "firmware/log-sink" in data.get("provides", [])
    require(offers_sink == ("log_sink" in data), path, "firmware/log-sink and log_sink must be declared together")
    if offers_sink:
        sink = data["log_sink"]
        fields(sink, path, "log_sink", ("target", "enabled_by"))
        require("cmake" in data and matches(IDENTIFIER, sink["target"]) and matches(SYMBOL, sink["enabled_by"]), path,
                "Invalid log_sink target, symbol or missing CMake entry")
    for model in array(data.get("models", []), path, "models", lambda value: isinstance(value, dict)):
        fields(model, path, "model", ("board", "apps", "path", "level", "limitations"))
        category_ids([model["board"]], path, "model.board", "board")
        category_ids(model["apps"], path, "model.apps", "app")
        require(valid_relpath(model["path"]) and model["level"] in ("startup-smoke", "peripheral-verified"),
                path, "Invalid model path or verification level")
        array(model["limitations"], path, "model.limitations", text, nonempty=True)
    validate_actions(data, path)
    return Pack(path, data)


def discover(templates_root):
    architectures = discover_architectures(templates_root.parent / "arch")
    registry = {}
    for entry in sorted(templates_root.rglob("pack.json")):
        local_path(templates_root, entry.relative_to(templates_root).as_posix(), entry)
        pack = read_pack(entry)
        if pack.id in registry:
            raise ValueError(f"{entry}: Duplicate pack ID {pack.id!r}, also declared in {registry[pack.id].path}")
        if pack.kind == "board":
            resolve_board(architectures, pack.id, pack.data["board"])
        registry[pack.id] = pack
    return registry


def project_pack_path(pack_id):
    return f"tool/packs/{pack_id}/pack.json"


def dependency_order(get_pack, selection):
    packs = {}
    dependents = {}
    pending = {}

    def collect(pack_id):
        if pack_id in packs:
            return
        pack = packs[pack_id] = get_pack(pack_id)
        dependencies = pack.data.get("dependencies", [])
        pending[pack_id] = len(dependencies)
        for dependency in dependencies:
            dependents.setdefault(dependency, []).append(pack_id)
            collect(dependency)
            dependency_pack = packs[dependency]
            if "cmake" in pack.data and "cmake" in dependency_pack.data:
                require(PHASE[dependency_pack.kind] <= PHASE[pack.kind], pack.path,
                        f"CMake entry of {pack_id} depends on later stage {dependency}")

    for pack_id in selection:
        collect(pack_id)
    ready = [pack_id for pack_id, count in pending.items() if count == 0]
    heapq.heapify(ready)
    ordered = []
    while ready:
        pack_id = heapq.heappop(ready)
        ordered.append(packs[pack_id])
        for dependent in dependents.get(pack_id, []):
            pending[dependent] -= 1
            if pending[dependent] == 0:
                heapq.heappush(ready, dependent)
    require(len(ordered) == len(packs), "pack dependencies", "Dependency cycle: " +
            ", ".join(sorted(name for name, count in pending.items() if count)))
    return tuple(ordered)


def check_file_references(pack):
    paths = [pack.data["cmake"]] if "cmake" in pack.data else []
    paths.extend(model["path"] for model in pack.data.get("models", []))
    for action in pack.data.get("actions", []):
        paths.extend(action[field] for field in ("handler", "tool_check") if field in action)
        paths.extend(profile["vscode_fragment"] for profile in action.get("debug_profiles", []) if "vscode_fragment" in profile)
    return paths


def compose(get_pack, selection, defaults):
    array(selection, "fly.project.json", "packs", lambda value: matches(PACKAGE_ID, value), nonempty=True, unique=True)
    require(isinstance(defaults, dict), "fly.project.json", "defaults must be an object")
    packs = dependency_order(get_pack, selection)
    selected = {pack.id: pack for pack in packs}
    by_kind = {kind: [pack for pack in packs if pack.kind == kind] for kind in ("board", "app")}
    require(all(len(items) == 1 and items[0].id in selection for items in by_kind.values()), "fly.project.json",
            "Select exactly one board and one app explicitly")
    board, app = by_kind["board"][0], by_kind["app"][0]
    board_values(board, app)
    require(all(selected[pack_id].kind != "support" for pack_id in selection), "fly.project.json",
            "Support packs are selected by dependency")
    available = {capability for pack in packs for capability in pack.data.get("provides", [])}
    actions = {}
    owners = {}
    missing = set()
    diagnostics = []
    for pack in packs:
        absent = set(pack.data.get("requires", [])) - available
        if absent:
            missing.update(absent)
            diagnostics.append(f"{pack.id} needs {', '.join(sorted(absent))}")
        conflicts = set(pack.data.get("conflicts", [])) & selected.keys()
        require(not conflicts, pack.path, f"Conflicting packs: {', '.join(sorted(conflicts))}")
        compat = pack.data.get("compatibility", {})
        for key, item in (("boards", board), ("apps", app)):
            require(key not in compat or item.id in compat[key], pack.path, f"Incompatible with {item.id}")
        if "board_data" in compat:
            require(board.id in compat["board_data"], pack.path, f"Incompatible with {board.id}")
        if "models" in pack.data:
            models = [model for model in pack.data["models"] if model["board"] == board.id and app.id in model["apps"]]
            require(len(models) == 1, pack.path, f"Expected exactly one verified model for {board.id} + {app.id}")
        for action in pack.data.get("actions", []):
            key = action["name"], action["provider"]
            require(key not in actions, pack.path, f"Duplicate action registration {key[0]}/{key[1]}")
            actions[key] = action
        for item in pack.data["files"]:
            destination = item["destination"]
            require(destination.casefold() not in owners, pack.path, f"File destination conflict: {destination}")
            owners[destination.casefold()] = pack.id
    for pack in packs:
        allowed = {pack.id, *pack.data.get("dependencies", [])}
        for path in check_file_references(pack):
            owner = owners.get(path.casefold())
            require(owner in allowed and any(item["destination"] == path for item in selected[owner].data["files"]),
                    pack.path, f"Path {path!r} must belong to this pack or an explicit dependency")
    for action, provider in defaults.items():
        require(action in ACTIONS and matches(NAME, provider) and (action, provider) in actions,
                "fly.project.json", f"Unregistered default action {action}/{provider}")
    if missing:
        raise MissingCapabilities(missing, diagnostics)
    return Composition(board, app, packs, actions, defaults)


def from_registry(registry, selection, defaults):
    def get_pack(pack_id):
        require(pack_id in registry, "template registry", f"Unknown pack {pack_id!r}")
        return registry[pack_id]

    return compose(get_pack, selection, defaults)


class AvailablePacks:
    """Determine whether a board, app and optional tools can form a complete project."""

    def __init__(self, registry, validate=None):
        self.registry = registry
        self.validate = validate
        self.tools = {}
        self.checked = {}
        for pack in registry.values():
            if pack.kind != "tool":
                continue
            try:
                closure = dependency_order(registry.__getitem__, [pack.id])
            except (ValueError, KeyError):
                continue
            self.tools[pack.id] = {
                capability for dependency in closure for capability in dependency.data.get("provides", ())
            }

    def is_available(self, board, app, *tools):
        selection = (board, app, *sorted(tools))
        if selection in self.checked:
            return self.checked[selection]
        try:
            composition = from_registry(self.registry, list(selection), {})
            if self.validate is not None:
                self.validate(composition)
            available = True
        except MissingCapabilities as error:
            available = any(
                self.is_available(board, app, *tools, tool)
                for tool, provides in self.tools.items()
                if tool not in tools and provides & error.capabilities
            )
        except (OSError, ValueError):
            available = False
        self.checked[selection] = available
        return available


def load_project(project_root):
    """Load installed source manifests as the authority for the selected actions."""
    project_root = Path(project_root).resolve()
    manifest_path = local_path(project_root, "fly.project.json", project_root)
    data = read_json(manifest_path)
    fields(data, manifest_path, "project", ("schema_version", "packs", "defaults"))
    version(data["schema_version"], manifest_path)

    def get_pack(pack_id):
        path = local_path(project_root, project_pack_path(pack_id), manifest_path)
        pack = read_pack(path)
        require(pack.id == pack_id, path, f"Installed pack ID must match {pack_id!r}")
        return pack

    composition = compose(get_pack, data["packs"], data["defaults"])
    for pack in composition.packs:
        for file in pack.data["files"]:
            path = local_path(project_root, file["destination"], pack.path)
            require(path.is_file(), pack.path, f"Installed file is missing: {file['destination']}")
    return composition
