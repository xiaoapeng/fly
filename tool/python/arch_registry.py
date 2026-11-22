"""Discover SDK architectures and resolve the one selected by a board."""

import re
from dataclasses import dataclass
from pathlib import Path
from typing import Optional

from action_protocol import ID_PATTERN, exact, read_json, string, version


_CONFIG = re.compile(r"CONFIG_ARCH_[A-Z0-9_]+")
_PATH_COMPONENT = re.compile(r"[A-Za-z0-9][A-Za-z0-9_.-]*")


@dataclass(frozen=True)
class Architecture:
    id: str
    config: str
    relative_path: str
    directory: Path
    host_system: Optional[str] = None

    @property
    def symbol(self):
        return self.config[len("CONFIG_"):]

    def kconfig_environment(self):
        return {
            "FLY_ARCH_SYMBOL": self.symbol,
            "FLY_ARCH_PATH": self.relative_path,
            "FLY_ARCH_KCONFIG": (self.directory / "Kconfig").as_posix(),
        }


def discover(arch_root):
    """Index local architecture manifests by their unique IDs."""
    arch_root = Path(arch_root).resolve()
    architectures = {}
    symbols = set()
    for path in sorted(arch_root.rglob("arch.json")):
        try:
            path.resolve(strict=True).relative_to(arch_root)
            directory = path.parent.resolve(strict=True)
            relative_path = directory.relative_to(arch_root)
            if not relative_path.parts or not all(_PATH_COMPONENT.fullmatch(part) for part in relative_path.parts):
                raise ValueError("Architecture directory has an invalid path")
            manifest = exact(read_json(path), "schema_version", "id", "config", optional=("host_system",))
            version(manifest)
            arch_id = string(manifest["id"], "architecture ID", pattern=ID_PATTERN)
            config = string(manifest["config"], "Kconfig symbol", pattern=_CONFIG)
            host_system = manifest.get("host_system")
            if host_system is not None:
                string(host_system, "host system", pattern=_PATH_COMPONENT)
            for entry in ("gcc-config.cmake", "CMakeLists.txt"):
                entry_path = directory / entry
                if not entry_path.is_file():
                    raise ValueError(f"Missing architecture entry {entry}")
                entry_path.resolve(strict=True).relative_to(arch_root)
            kconfig = directory / "Kconfig"
            if kconfig.exists() or kconfig.is_symlink():
                if not kconfig.is_file():
                    raise ValueError("Invalid architecture Kconfig")
                kconfig.resolve(strict=True).relative_to(arch_root)
            if arch_id in architectures:
                raise ValueError(f"Duplicate architecture ID {arch_id!r}")
            if config in symbols:
                raise ValueError(f"Duplicate architecture symbol {config}")
        except (OSError, ValueError) as error:
            raise ValueError(f"{path}: {error}") from error
        architectures[arch_id] = Architecture(arch_id, config, relative_path.as_posix(), directory, host_system)
        symbols.add(config)
    return architectures


def resolve_board(architectures, board_id, board):
    arch_id = board.get("arch")
    if not isinstance(arch_id, str) or arch_id not in architectures:
        raise ValueError(f"{board_id} references unknown architecture {arch_id!r}")
    architecture = architectures[arch_id]
    if (board["target_kind"] == "host") != (architecture.host_system is not None):
        raise ValueError(f"{board_id} target kind does not match architecture {arch_id!r}")
    return architecture


def for_board(arch_root, board_id, board):
    return resolve_board(discover(arch_root), board_id, board)


def validate_choice(kconf, architecture):
    """Reject loaded config assignments that disagree with the selected board."""
    for name, value in kconf.missing_syms:
        if name.startswith("ARCH_") and value != "n":
            raise ValueError(f"Unexpected CONFIG_{name} for architecture {architecture.id}")
    arch_symbol = kconf.syms[architecture.symbol]
    arch_path = kconf.syms["ARCH"]
    if arch_symbol.user_value not in (None, 2) or arch_symbol.str_value != "y":
        raise ValueError(f"{architecture.config} must be enabled for architecture {architecture.id}")
    if arch_path.user_value not in (None, architecture.relative_path) or arch_path.str_value != architecture.relative_path:
        raise ValueError(f"CONFIG_ARCH must be {architecture.relative_path!r} for architecture {architecture.id}")
