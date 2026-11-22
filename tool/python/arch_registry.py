"""Discover SDK architectures and resolve the one selected by a board."""

import re
from dataclasses import dataclass
from pathlib import Path
from typing import Optional

from action_protocol import ID_PATTERN, exact, read_json, string, version


_CONFIG = re.compile(r"CONFIG_ARCH_[A-Z0-9_]+")
_PATH_COMPONENT = re.compile(r"[A-Za-z0-9][A-Za-z0-9_.-]*")
_PROFILE_OPTIONS = ("fpu", "float_abi", "march", "abi")
_PROFILE_SYMBOLS = {f"CONFIG_ARCH_{name}" for name in (
    "PROFILE", "CORE", "ISA", "TOOLCHAIN", "FPU", "FLOAT_ABI", "MARCH", "ABI",
    "CPU_FAMILY_CORTEX_M", "HAS_HARD_FPU", "NVIC_PRIO_BITS",
)}


@dataclass(frozen=True)
class Architecture:
    id: str
    config: str
    relative_path: str
    directory: Path
    host_system: Optional[str]
    isa: str
    toolchain: str
    profiles: dict

    @property
    def symbol(self):
        return self.config[len("CONFIG_"):]


@dataclass(frozen=True)
class ArchitectureSelection(Architecture):
    profile: str
    cpu: str
    fpu: str
    float_abi: str
    march: str
    abi: str

    def kconfig_environment(self):
        return {
            "FLY_ARCH_SYMBOL": self.symbol,
            "FLY_ARCH_ID": self.id,
            "FLY_ARCH_PROFILE": self.profile,
            "FLY_ARCH_CORE": self.cpu,
            "FLY_ARCH_ISA": self.isa,
            "FLY_ARCH_TOOLCHAIN": self.toolchain,
            "FLY_ARCH_FPU": self.fpu,
            "FLY_ARCH_FLOAT_ABI": self.float_abi,
            "FLY_ARCH_MARCH": self.march,
            "FLY_ARCH_ABI": self.abi,
            "FLY_ARCH_IS_CORTEX_M": "y" if self.isa == "arm" and self.cpu.startswith("cortex-m") else "n",
            "FLY_ARCH_HAS_HARD_FPU": "y" if self.fpu else "n",
            "FLY_ARCH_KCONFIG": (self.directory / "Kconfig").as_posix(),
        }


def _profiles(manifest, isa):
    if "profiles" in manifest:
        if any(key in manifest for key in ("cpu", *_PROFILE_OPTIONS)):
            raise ValueError("Architecture profiles and top-level CPU settings are mutually exclusive")
        profiles = manifest["profiles"]
        if not isinstance(profiles, dict) or not profiles:
            raise ValueError("Architecture profiles must be a nonempty object")
    else:
        profiles = {"default": {key: manifest[key] for key in ("cpu", *_PROFILE_OPTIONS) if key in manifest}}

    normalized = {}
    for profile_id, data in profiles.items():
        string(profile_id, "architecture profile ID", pattern=ID_PATTERN)
        profile = exact(data, "cpu", optional=_PROFILE_OPTIONS)
        cpu = string(profile["cpu"], "CPU", pattern=_PATH_COMPONENT)
        options = {key: string(profile[key], key, pattern=_PATH_COMPONENT) if key in profile else ""
                   for key in _PROFILE_OPTIONS}
        if isa == "arm":
            options["float_abi"] = options["float_abi"] or "soft"
            if options["float_abi"] not in {"soft", "softfp", "hard"}:
                raise ValueError(f"Invalid Arm float ABI {options['float_abi']!r}")
            if options["float_abi"] == "hard" and not options["fpu"]:
                raise ValueError("A hard float ABI requires an FPU")
        if isa != "arm" and (options["fpu"] or options["float_abi"]):
            raise ValueError("fpu and float_abi apply to Arm profiles")
        if isa == "riscv":
            if not options["march"] or not options["abi"]:
                raise ValueError("A RISC-V profile requires both march and abi")
        elif options["march"] or options["abi"]:
            raise ValueError("march and abi apply to RISC-V profiles")
        normalized[profile_id] = {"cpu": cpu, **options}
    return normalized


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
            manifest = exact(read_json(path), "schema_version", "id", "config", "isa", "toolchain",
                             optional=("host_system", "cpu", "profiles", *_PROFILE_OPTIONS))
            version(manifest)
            arch_id = string(manifest["id"], "architecture ID", pattern=ID_PATTERN)
            config = string(manifest["config"], "Kconfig symbol", pattern=_CONFIG)
            if config in _PROFILE_SYMBOLS:
                raise ValueError(f"Kconfig symbol {config} is reserved for architecture profile data")
            isa = manifest["isa"]
            if isa not in ("arm", "riscv", "host"):
                raise ValueError(f"Unknown architecture ISA {isa!r}")
            toolchain = string(manifest["toolchain"], "toolchain", pattern=ID_PATTERN)
            host_system = manifest.get("host_system")
            if host_system is not None:
                string(host_system, "host system", pattern=_PATH_COMPONENT)
            if (isa == "host") != (host_system is not None):
                raise ValueError("host_system is required for host architectures only")
            profiles = _profiles(manifest, isa)
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
        architectures[arch_id] = Architecture(arch_id, config, relative_path.as_posix(), directory,
                                              host_system, isa, toolchain, profiles)
        symbols.add(config)
    return architectures


def resolve_board(architectures, board_id, board):
    arch_id = board.get("arch")
    if not isinstance(arch_id, str) or arch_id not in architectures:
        raise ValueError(f"{board_id} references unknown architecture {arch_id!r}")
    architecture = architectures[arch_id]
    if (board["target_kind"] == "host") != (architecture.host_system is not None):
        raise ValueError(f"{board_id} target kind does not match architecture {arch_id!r}")
    profile_id = board.get("arch_profile")
    if profile_id is None and len(architecture.profiles) == 1:
        profile_id = next(iter(architecture.profiles))
    if not isinstance(profile_id, str) or profile_id not in architecture.profiles:
        choices = ", ".join(sorted(architecture.profiles))
        raise ValueError(f"{board_id} has invalid arch_profile {profile_id!r} for {arch_id!r}; choose {choices}")
    profile = architecture.profiles[profile_id]
    if architecture.isa == "arm" and board["core"]["architecture"] != f"arm-{profile['cpu']}":
        raise ValueError(f"{board_id} core architecture must be arm-{profile['cpu']} for {arch_id}/{profile_id}")
    return ArchitectureSelection(**vars(architecture), profile=profile_id, **profile)


def for_board(arch_root, board_id, board):
    return resolve_board(discover(arch_root), board_id, board)


def validate_choice(kconf, architecture):
    """Reject loaded config assignments that disagree with the selected board."""
    for name, value in kconf.missing_syms:
        if name.startswith("ARCH_") and value != "n":
            raise ValueError(f"Unexpected CONFIG_{name} for architecture {architecture.id}")
    arch_symbol = kconf.syms[architecture.symbol]
    arch_id = kconf.syms["ARCH"]
    profile = kconf.syms["ARCH_PROFILE"]
    if arch_symbol.user_value not in (None, 2) or arch_symbol.str_value != "y":
        raise ValueError(f"{architecture.config} must be enabled for architecture {architecture.id}")
    # Existing build trees may still contain the old relative directory value.
    if arch_id.user_value not in (None, architecture.id, architecture.relative_path) or arch_id.str_value != architecture.id:
        raise ValueError(f"CONFIG_ARCH must be {architecture.id!r}")
    if profile.user_value not in (None, architecture.profile) or profile.str_value != architecture.profile:
        raise ValueError(f"CONFIG_ARCH_PROFILE must be {architecture.profile!r} for architecture {architecture.id}")
