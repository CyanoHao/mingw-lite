from contextlib import contextmanager
import logging
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
from tempfile import TemporaryDirectory
from typing import Dict, Iterable, List, Optional, Sequence, Union

from .path import ProjectPaths
from .platform import is_genuine_linux, is_wsl1
from .profile import OptLv

OPT_LV_2_CMAKE_TYPE_MAP: Dict[OptLv, str] = {
  OptLv.O0: 'Debug',
  OptLv.Og: 'Debug',
  OptLv.O1: 'MinSizeRel',
  OptLv.Oz: 'MinSizeRel',
  OptLv.Os: 'MinSizeRel',
  OptLv.O2: 'Release',
  OptLv.O3: 'Release',
}

OPT_LV_2_MESON_TYPE_MAP: Dict[OptLv, str] = {
  OptLv.O0: 'debug',
  OptLv.Og: 'debug',
  OptLv.O1: 'minsize',
  OptLv.Oz: 'minsize',
  OptLv.Os: 'minsize',
  OptLv.O2: 'release',
  OptLv.O3: 'release',
}

OPT_LV_2_XMAKE_MODE_MAP = {
  OptLv.O0: 'debug',
  OptLv.Og: 'debug',
  OptLv.O1: 'minsizerel',
  OptLv.Oz: 'minsizerel',
  OptLv.Os: 'minsizerel',
  OptLv.O2: 'release',
  OptLv.O3: 'release',
}

XMAKE_ARCH_MAP = {
  '32': 'i386',
  '64': 'x86_64',
  'arm64': 'aarch64',
}

def add_objects_to_static_lib(ar: str, lib: Path, objects: Iterable[Path]):
  subprocess.run(
    [ar, 'r', lib, *objects],
    check = True,
  )

def cflags_A(
  suffix: str = '',
  cpp_extra: List[str] = [],
  common_extra: List[str] = [],
  ld_extra: List[str] = [],
  c_extra: List[str] = [],
  cxx_extra: List[str] = [],
) -> List[str]:
  cpp = ['-DNDEBUG']
  common = ['-O2', '-pipe']
  ld = ['-s']
  return [
    f'CPPFLAGS{suffix}=' + ' '.join(cpp + cpp_extra),
    f'CFLAGS{suffix}=' + ' '.join(common + common_extra + c_extra),
    f'CXXFLAGS{suffix}=' + ' '.join(common + common_extra + cxx_extra),
    f'LDFLAGS{suffix}=' + ' '.join(ld + ld_extra),
  ]

def cflags_B(
  suffix: str = '',
  cpp_extra: List[str] = [],
  common_extra: List[str] = [],
  ld_extra: List[str] = [],
  c_extra: List[str] = [],
  cxx_extra: List[str] = [],
  opt_lv: OptLv = OptLv.O2,
  lto: bool = False,
) -> List[str]:
  debug = opt_lv == OptLv.O0 or opt_lv == OptLv.Og

  cpp: List[str] = [] if debug else ['-DNDEBUG']
  common = [opt_lv.value, '-pipe']
  ld: List[str] = [] if debug else ['-s']

  if lto:
    common.append('-flto')
    ld.extend([opt_lv.value, '-flto'])

  return [
    f'CPPFLAGS{suffix}=' + ' '.join(cpp + cpp_extra),
    f'CFLAGS{suffix}=' + ' '.join(common + common_extra + c_extra),
    f'CXXFLAGS{suffix}=' + ' '.join(common + common_extra + cxx_extra),
    f'LDFLAGS{suffix}=' + ' '.join(ld + ld_extra),
  ]

def cmake_build(
  cwd: Path,
  jobs: int,
  targets: List[str] = [],
  build_dir: str = 'build',
):
  subprocess.run(
    ['cmake', '--build', build_dir, '-j', str(jobs), *targets],
    cwd = cwd,
    check = True,
  )

def cmake_config(
  cwd: Path,
  extra_args: List[str],
  build_dir: str = 'build',
):
  subprocess.run(
    ['cmake', '-G', 'Ninja', '-S', '.', '-B', build_dir, *extra_args],
    cwd = cwd,
    check = True,
  )

def cmake_flags_A() -> List[str]:
  return [
    '-DCMAKE_BUILD_TYPE=Release',
  ]

def cmake_flags_B(
  opt_lv: OptLv = OptLv.Os,
  lto: bool = False,
) -> List[str]:
  lto_flag = []
  if lto:
    lto_flag = ['-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON']

  return [
    f'-DCMAKE_BUILD_TYPE={OPT_LV_2_CMAKE_TYPE_MAP[opt_lv]}',
    *lto_flag,
  ]

def cmake_install(
  cwd: Path,
  destdir: Path,
  targets: List[str] = [],
  build_dir: str = 'build',
):
  subprocess.run(
    ['cmake', '--install', build_dir, *targets],
    cwd = cwd,
    check = True,
    env = {
      **os.environ,
      'DESTDIR': destdir,
    },
  )

def common_cross_layers(paths: ProjectPaths):
  return [
    paths.layer_AAB.binutils / 'usr/local',
    paths.layer_AAB.gcc / 'usr/local',
    paths.layer_AAB.gcc_lib / 'usr/local',
    paths.layer_AAB.headers / 'usr/local',
    paths.layer_AAB.mcfgthread / 'usr/local',
    paths.layer_AAB.winpthreads / 'usr/local',
  ]

def configure(cwd: Path, args: List[str]):
  subprocess.run(
    ['../configure', *args],
    cwd = cwd,
    check = True,
  )

def ensure(path: Path):
  path.mkdir(parents = True, exist_ok = True)

def extract_shared_libs(
  base_prefix: Path,
  shared_prefix: Optional[Path],
  include: Sequence[Union[str, Path]] = [],
  exclude: Sequence[Union[str, Path]] = [],
):
  bin_dir = base_prefix / 'bin'
  lib_dir = base_prefix / 'lib'

  # part 1: normal (dll + dll.a + extra .a)
  all_files = [
    *bin_dir.glob('*.dll'),
    *lib_dir.glob('*.dll.a'),
    *map(lambda p: base_prefix / p, include),
  ]
  exclude_files = [*map(lambda p: base_prefix / p, exclude)]
  files = [f for f in all_files if f not in exclude_files]

  for file in files:
    if shared_prefix:
      rel_dir = file.parent.relative_to(base_prefix)
      ensure(shared_prefix / rel_dir)
      shutil.move(file, shared_prefix / rel_dir / file.name)
    else:
      file.unlink()

  dll_in_lib = [*lib_dir.glob('*.dll')]
  for dll in dll_in_lib:
    if shared_prefix:
      ensure(shared_prefix / 'bin')
      shutil.move(dll, shared_prefix / 'bin' / dll.name)
    else:
      dll.unlink()

def make_custom(cwd: Path, extra_args: List[str], jobs: int):
  subprocess.run(
    ['make', *extra_args, f'-j{jobs}'],
    cwd = cwd,
    check = True,
  )

def make_default(cwd: Path, jobs: int):
  make_custom(cwd, [], jobs)

def make_destdir_install(cwd: Path, destdir: Path):
  make_custom(cwd, [f'DESTDIR={destdir}', 'install'], jobs = 1)

def make_install(cwd: Path):
  make_custom(cwd, ['install'], jobs = 1)

def meson_build(
  cwd: Path,
  jobs: int,
  targets: List[str] = [],
  build_dir: str = 'build',
):
  subprocess.run(
    ['meson', 'compile', '-C', build_dir, f'-j{jobs}', *targets],
    cwd = cwd,
    check = True,
  )

def meson_config(
  cwd: Path,
  extra_args: Sequence[Union[str, Path]],
  build_dir: str = 'build',
):
  subprocess.run(
    ['meson', 'setup', *extra_args, build_dir],
    cwd = cwd,
    check = True
  )

def meson_flags_B(
  cpp_extra: List[str] = [],
  common_extra: List[str] = [],
  ld_extra: List[str] = [],
  c_extra: List[str] = [],
  cxx_extra: List[str] = [],
  opt_lv: OptLv = OptLv.Os,
) -> List[str]:
  cpp = ['-DNDEBUG']
  common = ['-pipe']
  return [
    '-Dc_args=' + ' '.join(cpp + cpp_extra + common + common_extra + c_extra),
    '-Dc_link_args=' + ' '.join(ld_extra),
    '-Dcpp_args=' + ' '.join(cpp + cpp_extra + common + common_extra + cxx_extra),
    '-Dcpp_link_args=' + ' '.join(ld_extra),
    f'--buildtype={OPT_LV_2_MESON_TYPE_MAP[opt_lv]}',
    '--strip',
  ]

def meson_install(
  cwd: Path,
  destdir: Path,
  build_dir: str = 'build',
):
  subprocess.run(
    ['meson', 'install', '-C', build_dir, '--destdir', destdir],
    cwd = cwd,
    check = True,
  )

@contextmanager
def overlayfs_ro(merged: Union[Path, str], lower: Sequence[Union[Path, str]]):
  if type(merged) is not Path:
    merged = Path(merged)
  ensure(merged)
  if is_genuine_linux():
    # `mount -t overlay`, or `mount --bind` (if only 1 lower).
    if len(lower) == 1:
      subprocess.run([
        'mount',
        '--bind',
        lower[0],
        merged,
        '-o', 'ro',
      ], check = True)
    else:
      lowerdir = ':'.join(map(str, lower))
      subprocess.run([
        'mount',
        '-t', 'overlay',
        'none',
        merged,
        '-o', f'lowerdir={lowerdir}',
      ], check = True)
    try:
      yield
    finally:
      subprocess.run(['umount', merged], check = False)
  elif is_wsl1():
    # `mount -t tmpfs`, then copy.
    subprocess.run(['mount', '-t', 'tmpfs', 'none', merged], check = True)
    try:
      # https://www.kernel.org/doc/html/v6.18/filesystems/overlayfs.html
      #   The specified lower directories will be stacked
      #   beginning from the rightmost one and going left.
      # so we copy from right to left.
      for layer in reversed(lower):
        shutil.copytree(layer, merged, dirs_exist_ok = True)
      yield
    finally:
      subprocess.run(['umount', merged], check = False)
  else:
    raise RuntimeError("unsupported platform")

def remove_info_main_menu(prefix: Path):
  info_main_menu = prefix / 'share/info/dir'
  if info_main_menu.exists():
    info_main_menu.unlink()

def _pe_rva_to_offset(data: bytes, rva: int) -> int:
  pe_off = struct.unpack_from('<I', data, 0x3C)[0]
  num_sections = struct.unpack_from('<H', data, pe_off + 6)[0]
  opt_size = struct.unpack_from('<H', data, pe_off + 20)[0]
  sect_off = pe_off + 24 + opt_size
  for i in range(num_sections):
    s = sect_off + i * 40
    virt_size = struct.unpack_from('<I', data, s + 8)[0]
    virt_addr = struct.unpack_from('<I', data, s + 12)[0]
    raw_size = struct.unpack_from('<I', data, s + 16)[0]
    raw_off = struct.unpack_from('<I', data, s + 20)[0]
    if virt_addr <= rva < virt_addr + max(virt_size, raw_size):
      return raw_off + (rva - virt_addr)
  raise ValueError(f'RVA {rva:#x} not in any section')

def strip_pe_tls_directory(path: Path) -> bool:
  data = path.read_bytes()

  pe_off = struct.unpack_from('<I', data, 0x3C)[0]
  magic = struct.unpack_from('<H', data, pe_off + 24)[0]
  if magic == 0x10b:
    dd_off = pe_off + 24 + 96
    image_base = struct.unpack_from('<I', data, pe_off + 24 + 28)[0]
  elif magic == 0x20b:
    dd_off = pe_off + 24 + 112
    image_base = struct.unpack_from('<Q', data, pe_off + 24 + 24)[0]
  else:
    return False

  tls_dd_off = dd_off + 9 * 8
  tls_rva, _ = struct.unpack_from('<II', data, tls_dd_off)
  if tls_rva == 0:
    return False

  tls_off = _pe_rva_to_offset(data, tls_rva)
  if magic == 0x10b:
    start_va, end_va, _, _, zero_fill, _ = struct.unpack_from('<IIIIII', data, tls_off)
  else:
    start_va, end_va, _, _, zero_fill, _ = struct.unpack_from('<QQQQII', data, tls_off)

  raw_size = end_va - start_va
  placeholder_size = 4 if magic == 0x10b else 8
  assert raw_size <= placeholder_size, f'{path.name}: TLS raw data is {raw_size} bytes (expected <= {placeholder_size})'
  if raw_size > 0:
    raw_off = _pe_rva_to_offset(data, start_va - image_base)
    raw_data = data[raw_off:raw_off + raw_size]
    assert raw_data == b'\x00' * raw_size, f'{path.name}: TLS raw data is non-zero ({raw_size} bytes)'
  assert zero_fill == 0, f'{path.name}: TLS SizeOfZeroFill = {zero_fill} (expected 0)'

  with open(path, 'r+b') as f:
    f.seek(tls_dd_off)
    f.write(struct.pack('<II', 0, 0))
  return True

def touch(path: Path):
  ensure(path.parent)
  path.touch(exist_ok = True)

def xmake_build(cwd: Path, jobs: int, targets: List[str] = []):
  subprocess.run(
    ['xmake', 'build', '-j', str(jobs), *targets],
    cwd = cwd,
    check = True,
  )

def xmake_config(cwd: Path, extra_args: List[str]):
  subprocess.run(
    ['xmake', 'config', *extra_args],
    cwd = cwd,
    check = True,
  )

def xmake_install(cwd: Path, destdir: Path, targets: List[str] = []):
  subprocess.run(
    ['xmake', 'install', '-o', destdir, *targets],
    cwd = cwd,
    check = True,
  )
