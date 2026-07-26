import json
from pathlib import Path

import pytest

from .apply import apply_thunk
from pe_coff import PeMachine
from .reader import read_mixed_library
from .test_helpers import serialize_apply_thunk_result

_TEST_RES = Path(__file__).parent / 'res'

_FO_PREFIX = '__force_override_mingw_emu__imp_'

_ARCHES = ('i386', 'amd64', 'arm64')
_ARCH_TO_PE = {
  'amd64': PeMachine.AMD64,
  'i386': PeMachine.I386,
  'arm64': PeMachine.ARM64,
}


def _kernel32_base(arch: str) -> Path:
  return _TEST_RES / arch / 'libkernel32.a'


def _kernel32_thunk(arch: str) -> Path:
  return _TEST_RES / arch / 'thunk-kernel32.a'


def _kernel32_alias(arch: str) -> Path:
  return _TEST_RES / arch / 'alias-kernel32.a'


def _crt_base(arch: str) -> Path:
  return _TEST_RES / arch / 'libcrt.a'


def _crt_thunk(arch: str) -> Path:
  return _TEST_RES / arch / 'thunk-libcrt.a'


def _crt_alias(arch: str) -> Path:
  return _TEST_RES / arch / 'alias-libcrt.a'


@pytest.mark.parametrize('arch', _ARCHES)
def test_snapshot(tmp_path: Path, arch: str) -> None:
  output_ar = tmp_path / 'output.a'
  result = apply_thunk(
    arch = _ARCH_TO_PE[arch],
    base_ar = _kernel32_base(arch),
    overlay_ar = _kernel32_thunk(arch),
    alias_ar = _kernel32_alias(arch),
    output_ar = output_ar,
    short_import = True,
    assert_thunk_free = False,
    assert_thunk_revertible = False,
    thunk_revert_prefix = '__ms_',
    allow_override_non_import = False,
    force_override_prefix = _FO_PREFIX,
  )
  mixed = read_mixed_library(output_ar, _ARCH_TO_PE[arch])
  actual = serialize_apply_thunk_result(result, mixed)

  golden = _TEST_RES / arch / 'apply_thunk_overlay.json'
  expected = json.loads(golden.read_text(encoding='utf-8'))

  assert actual == expected, (
    f'mismatch for {arch}\n'
    f'--- expected ({golden}) ---\n{json.dumps(expected, indent=2, sort_keys=True)}\n'
    f'--- actual ---\n{json.dumps(actual, indent=2, sort_keys=True)}\n'
  )


@pytest.mark.parametrize('arch', _ARCHES)
def test_snapshot_libcrt(tmp_path: Path, arch: str) -> None:
  output_ar = tmp_path / 'output.a'
  result = apply_thunk(
    arch = _ARCH_TO_PE[arch],
    base_ar = _crt_base(arch),
    overlay_ar = _crt_thunk(arch),
    alias_ar = _crt_alias(arch),
    output_ar = output_ar,
    short_import = True,
    assert_thunk_free = False,
    assert_thunk_revertible = False,
    thunk_revert_prefix = '__ms_',
    allow_override_non_import = False,
    force_override_prefix = _FO_PREFIX,
  )
  mixed = read_mixed_library(output_ar, _ARCH_TO_PE[arch])
  actual = serialize_apply_thunk_result(result, mixed)

  golden = _TEST_RES / arch / 'apply_thunk_libcrt.json'
  expected = json.loads(golden.read_text(encoding='utf-8'))

  assert actual == expected, (
    f'mismatch for {arch}/libcrt\n'
    f'--- expected ({golden}) ---\n{json.dumps(expected, indent=2, sort_keys=True)}\n'
    f'--- actual ---\n{json.dumps(actual, indent=2, sort_keys=True)}\n'
  )
