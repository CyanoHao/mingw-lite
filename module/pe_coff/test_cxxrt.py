import json
from pathlib import Path

import pytest

from pe_coff import PeMachine

from . import read_import_library

_TEST_RES = Path(__file__).parent / 'res'

_ARCH_TO_PE = {
  'i386': PeMachine.I386,
  'amd64': PeMachine.AMD64,
}

_ARCHES = ('i386', 'amd64')


@pytest.mark.parametrize('arch', _ARCHES)
def test_cxxrt_longform_snapshot(arch: str) -> None:
  archive = _TEST_RES / arch / 'libc++rt.a'
  golden = _TEST_RES / arch / 'libc++rt.json'

  expected = json.loads(golden.read_text(encoding='utf-8'))
  actual = read_import_library(archive, _ARCH_TO_PE[arch])

  assert actual == expected, (
    f'mismatch for {arch}/libc++rt\n'
    f'--- expected ({golden}) ---\n{json.dumps(expected, indent=2, sort_keys=True)}\n'
    f'--- actual ---\n{json.dumps(actual, indent=2, sort_keys=True)}\n'
  )
