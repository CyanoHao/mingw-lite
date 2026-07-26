from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List


@dataclass
class ArchiveMember:
  name: str
  data: bytes


def read_archive_members(path: Path) -> List[ArchiveMember]:
  with open(path, 'rb') as f:
    magic = f.read(8)
    if magic != b'!<arch>\n':
      raise ValueError(f'{path} is not an ar archive (missing !<arch> magic)')
    members: List[ArchiveMember] = []
    long_names: Dict[int, str] = {}

    while True:
      hdr = f.read(60)
      if len(hdr) < 60:
        break

      name_raw = hdr[0:16]
      size_str = hdr[48:58].decode('ascii', errors='replace').strip()
      if not size_str.isdigit():
        break
      size = int(size_str)

      # GNU ar: name field padded with spaces; special members are '/' and '//'.
      # Regular member names end with '/' (the GNU ar terminator).
      # Long-name references are '/N' where N is a byte offset into the '//' table.
      name_field = name_raw.rstrip(b' ').decode('ascii', errors='replace')

      if name_field == '/' or name_field == '':
        # Symbol table — skip
        f.read(size)
        if size % 2 == 1:
          f.read(1)
        continue
      elif name_field == '//':
        # Long name string table
        long_data = f.read(size)
        if size % 2 == 1:
          f.read(1)
        _parse_long_name_table(long_data, long_names)
        continue
      elif name_field.startswith('/') and name_field[1:].isdigit():
        idx = int(name_field[1:])
        resolved = long_names.get(idx)
        name = resolved if resolved is not None else name_field
        data = f.read(size)
        if size % 2 == 1:
          f.read(1)
        members.append(ArchiveMember(name = name, data = data))
        continue
      else:
        # Regular member — strip the single trailing '/' added by GNU ar
        name = name_field
        if name.endswith('/'):
          name = name[:-1]
        data = f.read(size)
        if size % 2 == 1:
          f.read(1)
        members.append(ArchiveMember(name = name, data = data))

  return members


def _parse_long_name_table(data: bytes, out: Dict[int, str]) -> None:
  # Two string-table layouts exist:
  #   COFF standard (recent LLVM): NUL-separated names with a trailing
  #     '\0\n' terminator, e.g. b'foo.o\x00bar.o\x00\n'
  #   GNU style: '\n'-separated names each ending in '/', e.g. b'foo.o/\nbar.o/\n'
  # The separator is picked by presence of '\0'; any trailing terminator
  # fragments split off as harmless extra entries that no '/N' reference
  # ever points at.
  sep = b'\n' if b'\0' not in data else b'\0'
  offset = 0
  for entry in data.split(sep):
    if entry:
      clean = entry
      if clean.endswith(b'/'):
        clean = clean[:-1]
      out[offset] = clean.decode('ascii', errors='replace')
    offset += len(entry) + 1
