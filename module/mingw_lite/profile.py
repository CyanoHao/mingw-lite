import argparse
import dataclasses
from dataclasses import dataclass
from enum import Enum
from packaging.version import Version
from typing import Callable, Dict, Optional

class OptLv(Enum):
  O0 = '-O0'
  Og = '-Og'
  O1 = '-O1'
  Oz = '-Oz'
  Os = '-Os'
  O2 = '-O2'
  O3 = '-O3'

@dataclass
class BranchVersions:
  gcc: str
  rev: str

  abi_frozen: bool
  branch_opt_lv: OptLv
  iconv_win32: bool
  lto_zstd: bool
  native_tls: bool
  short_import: bool
  thunk_free_os: Version
  utf8_thunk: bool

  mcfgthread: str
  mingw: str
  nowide: str

  binutils: str
  expat: str
  gdb: str
  gmp: str
  iconv: str
  isl: str
  make: str
  mpc: str
  mpfr: str
  pdcurses: str
  pkgconf: str
  python: str
  zlib_net: str
  zstd: str

  display_version: Optional[str] = None

  meson: str = '1.11.2'
  setuptools: str = '84.0.0'
  xmake: str = '3.1.0'

@dataclass
class ProfileInfo:
  arch: str
  fpmath: Optional[str]
  march: str
  target: str

  default_crt: str
  exception: str
  thread: str

  win32_winnt: int
  min_os: Version

  gdb_python: bool
  lang_lto: bool
  nls: bool
  profile_opt_lto: bool
  profile_opt_lv: Optional[OptLv]
  utf8_user_crt: bool

@dataclass
class BranchProfile(BranchVersions, ProfileInfo):
  @property
  def min_winnt(self) -> int:
    if self.min_os.major < 4:
      return 0x0400
    else:
      return self.min_os.major * 0x100 + self.min_os.minor

  @property
  def opt_lv(self) -> OptLv:
    if self.profile_opt_lv is not None:
      return self.profile_opt_lv
    return self.branch_opt_lv

  @property
  def thunk_free(self) -> bool:
    if self.min_os < self.thunk_free_os:
      return False
    if self.utf8_user_crt:
      return False
    return True

  @property
  def lto_bigobj(self) -> bool:
    # liblto_plugin (libiberty simple-object-coff.c) gained PE BigObj COFF
    # support in GCC 16; older toolchains cannot link -flto -Wa,-mbig-obj.
    return self.lang_lto and Version(self.gcc).major >= 16

BRANCHES: Dict[str, BranchVersions] = {
  'next': BranchVersions(
    gcc = '17-20260809',
    rev = '0',
    display_version = 'next-17-20260809',

    abi_frozen = False,
    branch_opt_lv = OptLv.O2,
    iconv_win32 = True,
    lto_zstd = True,
    native_tls = True,
    short_import = True,
    thunk_free_os = Version('6.0'),
    utf8_thunk = True,

    mcfgthread = '2.4-ga.2',
    mingw = '14.0.0',
    nowide = '11.3.1',

    binutils = '2.47',
    expat = '2.8.3',
    gdb = '17.2',
    gmp = '6.3.0',
    iconv = '1.19',
    isl = '0.28',
    make = '4.4.1',
    mpc = '1.4.1',
    mpfr = '4.2.2',
    pdcurses = '3.9',
    pkgconf = '3.0.5',
    python = '3.14.7',
    zlib_net = '1.3.2',
    zstd = '1.5.7',
  ),
  'current': BranchVersions(
    gcc = '16-20260815',
    rev = '0',
    display_version = 'current-16-20260815',

    abi_frozen = False,
    branch_opt_lv = OptLv.O2,
    iconv_win32 = True,
    lto_zstd = True,
    native_tls = True,
    short_import = True,
    thunk_free_os = Version('6.0'),
    utf8_thunk = True,

    mcfgthread = '2.4-ga.2',
    mingw = '14.0.0',
    nowide = '11.3.1',

    binutils = '2.47',
    expat = '2.8.3',
    gdb = '17.2',
    gmp = '6.3.0',
    iconv = '1.19',
    isl = '0.28',
    make = '4.4.1',
    mpc = '1.4.1',
    mpfr = '4.2.2',
    pdcurses = '3.9',
    pkgconf = '3.0.5',
    python = '3.14.7',
    zlib_net = '1.3.2',
    zstd = '1.5.7',
  ),
  '16': BranchVersions(
    gcc = '16.2.0',
    rev = '1',

    abi_frozen = False,
    branch_opt_lv = OptLv.O2,
    iconv_win32 = True,
    lto_zstd = True,
    native_tls = True,
    short_import = True,
    thunk_free_os = Version('6.0'),
    utf8_thunk = True,

    # ABI critical: 2026-08-07
    mcfgthread = '2.4-ga.2',
    mingw = '14.0.0',
    nowide = '11.3.1',

    binutils = '2.47',
    expat = '2.8.3',
    gdb = '17.2',
    gmp = '6.3.0',
    iconv = '1.19',
    isl = '0.28',
    make = '4.4.1',
    mpc = '1.4.1',
    mpfr = '4.2.2',
    pdcurses = '3.9',
    pkgconf = '3.0.5',
    python = '3.14.7',
    zlib_net = '1.3.2',
    zstd = '1.5.7',
  ),
  '15': BranchVersions(
    gcc = '15.3.0',
    rev = '1',

    abi_frozen = True,
    branch_opt_lv = OptLv.Os,
    iconv_win32 = False,
    lto_zstd = False,
    native_tls = False,
    short_import = False,
    thunk_free_os = Version('5.1'),
    utf8_thunk = False,

    # ABI critical: 2025-08-08
    mcfgthread = '2.1-ga.1',
    mingw = '13.0.0',
    nowide = '11.3.0',  # 11.3.1 is not ABI compatible

    # freeze: 2025-12-21
    binutils = '2.45.1',
    expat = '2.7.5',
    gdb = '17.2',
    gmp = '6.3.0',
    iconv = '1.18',
    isl = '0.27',
    make = '4.4.1',
    mpc = '1.3.1',
    mpfr = '4.2.2',
    pdcurses = '3.9',
    pkgconf = '2.5.1',
    python = '3.14.7',
    zlib_net = '1.3.2',
    zstd = '1.5.7',
  ),
  '14': BranchVersions(
    gcc = '14.4.0',
    rev = '1',

    abi_frozen = True,
    branch_opt_lv = OptLv.Os,
    iconv_win32 = False,
    lto_zstd = False,
    native_tls = False,
    short_import = False,
    thunk_free_os = Version('5.1'),
    utf8_thunk = False,

    # ABI critical: 2024-08-01
    mcfgthread = '1.8-ga.4',
    mingw = '12.0.0',
    nowide = '11.3.0',  # 11.3.1 is not ABI compatible

    # freeze: 2024-12-21
    binutils = '2.43.1',
    expat = '2.6.4',
    gdb = '15.2',
    gmp = '6.3.0',
    iconv = '1.18',
    isl = '0.27',
    make = '4.4.1',
    mpc = '1.3.1',
    mpfr = '4.2.2',
    pdcurses = '3.9',
    pkgconf = '2.3.0',
    python = '3.13.15',
    zlib_net = '1.3.2',
    zstd = '1.5.7',
  ),
  '13': BranchVersions(
    gcc = '13.4.0',
    rev = '8',

    abi_frozen = True,
    branch_opt_lv = OptLv.Os,
    iconv_win32 = False,
    lto_zstd = False,
    native_tls = False,
    short_import = False,
    thunk_free_os = Version('5.1'),
    utf8_thunk = False,

    # trace back: 2023-12-22
    binutils = '2.41',
    expat = '2.5.0',
    gdb = '14.2',
    gmp = '6.3.0',
    iconv = '1.17',
    isl = '0.26',
    make = '4.4.1',
    # postponed until 1.8-ga.3 (2024-08-01)
    # - bootstrapping from scratch made easy
    # - gcc runtime library exception added
    mcfgthread = '1.8-ga.4',
    mingw = '11.0.1',
    mpc = '1.3.1',
    mpfr = '4.2.2',
    nowide = '11.3.0',  # 11.3.1 is not ABI compatible
    pdcurses = '3.9',
    pkgconf = '2.1.1',
    python = '3.12.14',
    zlib_net = '1.3.2',
    zstd = '1.5.7',
  ),
}

BRANCHES['16+emutls'] = dataclasses.replace(
  BRANCHES['16'],
  display_version = BRANCHES['16'].gcc + '+emutls',
  native_tls = False,
)

_MINGW_ARCH_2_TRIPLET_MAP: Dict[str, str] = {
  '64': 'x86_64-w64-mingw32',
  'arm64': 'aarch64-w64-mingw32',
  # required by xmake, here we use i686 for -march=i486 and -march=i386
  '32': 'i686-w64-mingw32',
}

_ARCH_VARIANT_2_MARCH_MAP: Dict[str, str] = {
  '64_v2': 'x86-64-v2',
  '64': 'x86-64',
  'arm64': 'armv8-a',
  '32': 'pentium4',
  '32_686': 'i686',
  '32_486': 'i486',
  '32_386': 'i386',
}

_ARCH_VARIANT_2_FPMATH_MAP: Dict[str, Optional[str]] = {
  '64_v2': None,
  '64': None,
  'arm64': None,
  '32': 'sse',
  '32_686': None,
  '32_486': None,
  '32_386': None,
}

def _create_profile(
  arch: str,
  crt: str,
  thread: str,
  min_os: str,

  gdb_python: bool = True,
  lang_lto: bool = True,
  nls: bool = True,
  opt_lto: bool = False,
  opt_lv: Optional[OptLv] = None,
  u8crt: bool = False,
) -> ProfileInfo:
  mingw_arch = arch.split('_')[0]
  exception = 'dwarf' if mingw_arch == '32' else 'seh'
  triplet = _MINGW_ARCH_2_TRIPLET_MAP[mingw_arch]
  march = _ARCH_VARIANT_2_MARCH_MAP[arch]
  fpmath = _ARCH_VARIANT_2_FPMATH_MAP[arch]

  return ProfileInfo(
    arch = mingw_arch,
    fpmath = fpmath,
    march = march,
    target = triplet,

    default_crt = crt,
    exception = exception,
    thread = thread,

    win32_winnt = 0x0A00,
    min_os = Version(min_os),

    gdb_python = gdb_python,
    lang_lto = lang_lto,
    nls = nls,
    profile_opt_lto = opt_lto,
    profile_opt_lv = opt_lv,
    utf8_user_crt = u8crt,
  )

PROFILES: Dict[str, Optional[ProfileInfo]] = {
  '64-mcf':    _create_profile('64', 'ucrt',   'mcf',   '6.1'),
  '64-win32':  _create_profile('64', 'ucrt',   'win32', '6.0'),
  '64-ucrt':   None, # branch-dependent
  '64-msvcrt': None, # branch-dependent

  'arm64-mcf':   _create_profile('arm64', 'ucrt', 'mcf',   '10.0.16299'),
  'arm64-win32': _create_profile('arm64', 'ucrt', 'win32', '10.0.16299'),
  'arm64-ucrt':  _create_profile('arm64', 'ucrt', 'posix', '10.0.16299'),

  '32-mcf':    _create_profile('32', 'ucrt',   'mcf',   '6.1'),
  '32-win32':  _create_profile('32', 'ucrt',   'win32', '6.0'),
  '32-ucrt':   None, # branch-dependent
  '32-msvcrt': None, # branch-dependent

  ############################################
  # profile variants for micro architectures #
  ############################################

  '64_v2-mcf':    _create_profile('64_v2', 'ucrt',   'mcf',   '6.1', opt_lto = True, opt_lv = OptLv.O2),
  '64_v2-win32':  _create_profile('64_v2', 'ucrt',   'win32', '6.0', opt_lto = True, opt_lv = OptLv.O2),
  '64_v2-ucrt':   _create_profile('64_v2', 'ucrt',   'posix', '5.2', opt_lto = True, opt_lv = OptLv.O2),
  '64_v2-msvcrt': _create_profile('64_v2', 'msvcrt', 'posix', '5.2', opt_lto = True, opt_lv = OptLv.O2),

  #################################################
  # profile variants for earlier Windows versions #
  #################################################

  '64-ucrt_ws2003':    _create_profile('64', 'ucrt',   'posix', '5.2'),
  '64-msvcrt_ws2003':  _create_profile('64', 'msvcrt', 'posix', '5.2'),
  '32-ucrt_winxp':     _create_profile('32', 'ucrt',   'posix', '5.1'),
  '32-msvcrt_win2000': _create_profile('32', 'msvcrt', 'posix', '5.0'),

  '32_686-msvcrt_win98': _create_profile('32_686', 'msvcrt', 'posix', '3.9999+4.10'),
  '32_486-msvcrt_win98': _create_profile('32_486', 'msvcrt', 'posix', '3.9999+4.10'),
  '32_386-msvcrt_win95': _create_profile('32_386', 'msvcrt', 'posix', '3.9999+4.00'),

  #####################
  # beyond MinGW Lite #
  #####################

  '64-u8crt': _create_profile('64', 'ucrt', 'posix', '5.2', u8crt = True),
  '32-u8crt': _create_profile('32', 'ucrt', 'posix', '5.1', u8crt = True),

  '64-ucrt_og': _create_profile('64', 'ucrt', 'posix', '6.0', opt_lv = OptLv.Og),
  '64-ucrt_o1': _create_profile('64', 'ucrt', 'posix', '6.0', opt_lv = OptLv.O1, lang_lto = False, nls = False),
  '64-ucrt_oz': _create_profile('64', 'ucrt', 'posix', '6.0', opt_lv = OptLv.Oz, lang_lto = False, nls = False, gdb_python = False),
  '64-ucrt_os': _create_profile('64', 'ucrt', 'posix', '6.0', opt_lv = OptLv.Os, lang_lto = False, nls = False),
  '64-ucrt_o3': _create_profile('64', 'ucrt', 'posix', '6.0', opt_lv = OptLv.O3),
}

def _gcc_16_requires_vista(
  arch: str,
  crt: str,
  thread: str,
  min_os: str,

  opt_lv: Optional[OptLv] = None,
  opt_lto: bool = False,
) -> Callable[[str, str], ProfileInfo]:
  def _profile(branch: str, v_gcc: str) -> ProfileInfo:
    v = Version(v_gcc)
    if v.major >= 16:
      return _create_profile(arch, crt, thread, '6.0', opt_lv = opt_lv, opt_lto = opt_lto)
    return _create_profile(arch, crt, thread, min_os, opt_lv = opt_lv, opt_lto = opt_lto)

  return _profile

BRANCH_DEPENDENT_PROFILES: Dict[str, Callable[[str, str], ProfileInfo]] = {
  '64-ucrt':   _gcc_16_requires_vista('64', 'ucrt',   'posix', '5.2'),
  '64-msvcrt': _gcc_16_requires_vista('64', 'msvcrt', 'posix', '5.2'),
  '32-ucrt':   _gcc_16_requires_vista('32', 'ucrt',   'posix', '5.1'),
  '32-msvcrt': _gcc_16_requires_vista('32', 'msvcrt', 'posix', '5.1'),
}

def resolve_profile(config: argparse.Namespace) -> BranchProfile:
  ver = BRANCHES[config.branch]
  if config.profile in BRANCH_DEPENDENT_PROFILES:
    info = BRANCH_DEPENDENT_PROFILES[config.profile](config.branch, ver.gcc)
  else:
    info = PROFILES[config.profile]
  return BranchProfile(
    **ver.__dict__,
    **info.__dict__,
  )
