"""Read the load image and debug link of a standalone ELF without a toolchain."""

from dataclasses import dataclass
import struct
import zlib


@dataclass(frozen=True)
class LoadSection:
    address: int
    data: bytes


class ElfImage:
    def __init__(self, data):
        if data[:4] != b"\x7fELF" or len(data) < 16 or data[6] != 1:
            raise ValueError("not an ELF v1 file")
        if data[4] not in (1, 2) or data[5] not in (1, 2):
            raise ValueError("unknown ELF word size or byte order")
        wide = data[4] == 2
        self.endian = "<" if data[5] == 1 else ">"
        header_fmt = "HHIQQQIHHHHHH" if wide else "HHIIIIIHHHHHH"
        ph_fmt = "IIQQQQQQ" if wide else "IIIIIIII"
        sh_fmt = "IIQQQQIIQQ" if wide else "IIIIIIIIII"
        (_, _, _, _, phoff, shoff, _, _, phsize, phcount, shsize,
         shcount, names_idx) = self._unpack(header_fmt, data, 16)
        if not shcount or names_idx >= shcount or phcount == 0xffff:
            raise ValueError("missing or extended ELF sections")
        if shsize < struct.calcsize(self.endian + sh_fmt) or (
                phcount and phsize < struct.calcsize(self.endian + ph_fmt)):
            raise ValueError("invalid ELF table entry sizes")
        if shoff + shsize * shcount > len(data) or phoff + phsize * phcount > len(data):
            raise ValueError("ELF table exceeds file size")

        segments = []
        for index in range(phcount):
            fields = self._unpack(ph_fmt, data, phoff + index * phsize)
            if wide:
                kind, _, offset, vma, lma, length, memsize, _ = fields
            else:
                kind, offset, vma, lma, length, memsize, _, _ = fields
            if kind == 1 and length:
                self._slice(data, offset, length)
                segments.append((offset, vma, lma, length, memsize))

        sections = [self._unpack(sh_fmt, data, shoff + index * shsize) for index in range(shcount)]
        names = self._slice(data, sections[names_idx][4], sections[names_idx][5])
        self.sections = {}
        loadable = []
        for name_at, kind, flags, address, offset, length, *_ in sections[1:]:
            name_end = names.find(b"\0", name_at)
            if name_end < 0:
                raise ValueError("invalid ELF section name")
            name = names[name_at:name_end]
            # SHT_NOBITS includes BSS. It has no bytes in the binary export.
            if not length or kind == 8:
                continue
            self.sections[name] = self._slice(data, offset, length)
            if not flags & 0x2:
                continue
            matches = {
                lma + address - vma
                for start, vma, lma, filesz, memsz in segments
                if start <= offset and offset + length <= start + filesz
                and vma <= address and address + length <= vma + memsz
            }
            if len(matches) > 1:
                raise ValueError("ELF section has ambiguous load addresses")
            if matches:
                loadable.append(LoadSection(matches.pop(), self.sections[name]))
        self.loadable = sorted(loadable, key=lambda section: section.address)

    def _unpack(self, fmt, data, offset):
        try:
            return struct.unpack_from(self.endian + fmt, data, offset)
        except (struct.error, OverflowError) as error:
            raise ValueError("truncated ELF structure") from error

    @staticmethod
    def _slice(data, offset, size):
        if offset + size > len(data):
            raise ValueError("ELF section exceeds file size")
        return data[offset:offset + size]

    def verify_binary(self, binary, address, capacity):
        if not self.loadable:
            raise ValueError("ELF has no file-backed load sections")
        base = self.loadable[0].address
        if base != address:
            raise ValueError(f"ELF load address {base:#x} differs from partition address {address:#x}")
        position = 0
        for section in self.loadable:
            offset = section.address - base
            end = offset + len(section.data)
            if offset < position or end > capacity:
                raise ValueError("ELF load ranges overlap or exceed partition capacity")
            if any(binary[position:offset]) or binary[offset:end] != section.data:
                raise ValueError(f"raw BIN does not match ELF load bytes at {section.address:#x}")
            position = end
        if position != len(binary):
            raise ValueError("raw BIN size differs from ELF export")
        return base

    def verify_debug_file(self, name, content):
        link = self.sections.get(b".gnu_debuglink")
        if link is None:
            if name is not None:
                raise ValueError("ELF lacks .gnu_debuglink for the declared debug file")
            return
        if name is None:
            raise ValueError("ELF requires the paired .gnu_debuglink debug file")
        name_end = link.find(b"\0")
        crc_at = (name_end + 4) & ~3
        if name_end < 1 or crc_at + 4 != len(link) or link[:name_end] != name.encode("utf-8"):
            raise ValueError("paired debug filename does not match .gnu_debuglink")
        if struct.unpack_from(self.endian + "I", link, crc_at)[0] != zlib.crc32(content):
            raise ValueError("paired debug CRC does not match .gnu_debuglink")
