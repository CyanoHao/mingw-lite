import json
from pathlib import Path

import pytest

from .apply import apply_thunk
from pe_coff import (
  PeMachine,
  read_import_library,
)

_TEST_RES = Path(__file__).parent / 'res'

_FO_PREFIX = '__force_override_mingw_emu__imp_'

_ARCH_TO_PE = {
  'amd64': PeMachine.AMD64,
  'i386': PeMachine.I386,
}

_ARCHES = ('i386', 'amd64')


@pytest.mark.parametrize('arch', _ARCHES)
def test_convert_cxxrt_longform_to_short(tmp_path: Path, arch: str) -> None:
  base_ar = _TEST_RES / arch / 'libc++rt.a'
  golden = _TEST_RES / arch / 'libc++rt.json'
  output_ar = tmp_path / 'output.a'

  apply_thunk(
    arch = _ARCH_TO_PE[arch],
    base_ar = base_ar,
    overlay_ar = None,
    alias_ar = None,
    output_ar = output_ar,
    short_import = True,
    assert_thunk_free = False,
    assert_thunk_revertible = False,
    thunk_revert_prefix = '__ms_',
    allow_override_non_import = False,
    force_override_prefix = _FO_PREFIX,
  )

  pe = _ARCH_TO_PE[arch]
  expected = json.loads(golden.read_text(encoding='utf-8'))
  actual = read_import_library(output_ar, pe)

  assert actual == expected, (
    f'mismatch for {arch}/libc++rt after short-import conversion\n'
    f'--- expected ({golden}) ---\n{json.dumps(expected, indent=2, sort_keys=True)}\n'
    f'--- actual ---\n{json.dumps(actual, indent=2, sort_keys=True)}\n'
  )
