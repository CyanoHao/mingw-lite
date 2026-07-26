import json
from pathlib import Path
from typing import Dict

import pytest

from pe_coff import PeMachine

from . import (
  ImportDll,
  ImportLibrary,
  ObjectMember,
  read_import_library,
)

_TEST_RES = Path(__file__).parent / 'res'
_ARCHES = ('i386', 'amd64', 'arm64')
_LIBS = (
  'libcdecl', 'libstdcall', 'libcrt',
  'libcdecl_thunk', 'libstdcall_thunk',
  'libcdecl_mixed', 'libstdcall_mixed',
)

_ARCH_TO_PE = {
  'i386': PeMachine.I386,
  'amd64': PeMachine.AMD64,
  'arm64': PeMachine.ARM64,
}

_CASES = [(arch, lib) for arch in _ARCHES for lib in _LIBS]
_IDS = [f'{arch}/{lib}' for arch, lib in _CASES]


@pytest.mark.parametrize('arch,lib', _CASES, ids=_IDS)
def test_snapshot(arch: str, lib: str) -> None:
  archive = _TEST_RES / arch / f'{lib}.a'
  golden = _TEST_RES / arch / f'{lib}.json'

  expected = json.loads(golden.read_text(encoding='utf-8'))
  actual = read_import_library(archive, _ARCH_TO_PE[arch])

  assert actual == expected, (
    f'mismatch for {arch}/{lib}\n'
    f'--- expected ({golden}) ---\n{json.dumps(expected, indent=2, sort_keys=True)}\n'
    f'--- actual ---\n{json.dumps(actual, indent=2, sort_keys=True)}\n'
  )
