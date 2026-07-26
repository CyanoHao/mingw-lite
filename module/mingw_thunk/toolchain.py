from typing import Dict

from pe_coff import PeMachine


_DLLTOOL_ARCH: Dict[PeMachine, str] = {
  PeMachine.I386: 'i386',
  PeMachine.AMD64: 'i386:x86-64',
  PeMachine.ARM64: 'arm64',
}

_TRIPLET: Dict[PeMachine, str] = {
  PeMachine.I386: 'i686-w64-mingw32',
  PeMachine.AMD64: 'x86_64-w64-mingw32',
  PeMachine.ARM64: 'aarch64-w64-mingw32',
}


def dlltool_arch(machine: PeMachine) -> str:
  return _DLLTOOL_ARCH[machine]


def triplet(machine: PeMachine) -> str:
  return _TRIPLET[machine]
