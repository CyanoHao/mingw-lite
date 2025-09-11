import argparse
from packaging.version import Version
from pathlib import Path
import tempfile
from typing import NamedTuple, Optional

from module.profile import BranchProfile

class SourcePaths(NamedTuple):
  binutils: Path
  expat: Path
  gcc: Path
  gdb: Path
  gmp: Path
  iconv: Path
  make: Path
  mcfgthread: Path
  mingw: Path
  mingw_host: Path
  mingw_target: Path
  mingw_qt: Path
  mpc: Path
  mpfr: Path
  pdcurses: Path
  pkgconf: Path
  python: Path
  xmake: Path
  z: Path

class InTreeSourcePaths(NamedTuple):
  intl: Path
  thunk: Path

class LayerPathsAAA(NamedTuple):
  prefix: Path

  gmp: Path
  mpc: Path
  mpfr: Path
  z: Path

  python: Path

class LayerPathsAAB(NamedTuple):
  prefix: Path

  binutils: Path
  crt: Path
  gcc: Path
  headers: Path

  expat: Path
  gmp: Path
  iconv: Path
  intl: Path
  mpc: Path
  mpfr: Path
  pdcurses: Path
  python: Path

class LayerPathsABB(NamedTuple):
  prefix: Path

  binutils: Path
  crt: Path
  gcc: Path
  gdb: Path
  headers: Path
  make: Path
  pkgconf: Path
  xmake: Path

  crt_qt: Path

  license: Path

class ProjectPaths:
  root_dir: Path
  abi_name: str

  assets_dir: Path
  dist_dir: Path
  patch_dir: Path

  meson_cross_file: Path

  mingw_pkg: Path
  mingw_qt_pkg: Path
  xmake_pkg: Path
  cross_pkg: Path

  # build phase

  utf8_src_dir: Path
  build_dir: Path
  layer_dir: Path
  pkg_dir: Path

  src_dir: SourcePaths
  src_arx: SourcePaths

  in_tree_src_dir: InTreeSourcePaths
  in_tree_src_tree: InTreeSourcePaths

  layer_AAA: LayerPathsAAA
  layer_AAB: LayerPathsAAB
  layer_ABB: LayerPathsABB

  # test phase

  test_dir: Path
  test_src_dir: Path

  test_mingw_dir: Path

  # target test archive phase

  sat_dir: Path
  sat_mingw_dir: Path

  def __init__(
    self,
    config: argparse.Namespace,
    ver: BranchProfile,
  ):
    self.root_dir = Path.cwd()
    abi_name = f'mingw{config.profile}-{config.branch}'
    self.abi_name = abi_name

    self.assets_dir = self.root_dir / 'assets'
    self.dist_dir = self.root_dir / 'dist'
    self.patch_dir = self.root_dir / 'patch'

    self.meson_cross_file = self.root_dir / 'support/meson' / f'{ver.target}.txt'

    self.mingw_pkg = self.dist_dir / f'mingw{config.profile}-{ver.gcc}-r{ver.rev}.tar.zst'
    self.mingw_qt_pkg = self.dist_dir / f'q-mingw{config.profile}-{ver.gcc}-r{ver.rev}.tar.zst'
    self.xmake_pkg = self.dist_dir / f'xmake-mingw{config.profile}-{ver.gcc}-r{ver.rev}.tar.zst'
    self.cross_pkg = self.dist_dir / f'x-mingw{config.profile}-{ver.gcc}-r{ver.rev}.tar.zst'

    # build phase

    self.utf8_src_dir = self.root_dir / 'support' / 'utf8'
    self.build_dir = Path(f'/tmp/build/{abi_name}')
    self.layer_dir = Path(f'{tempfile.gettempdir()}/layer/{abi_name}')
    self.pkg_dir = Path(f'/tmp/pkg/{abi_name}')

    src_name = SourcePaths(
      binutils = f'binutils-{ver.binutils}',
      expat = f'expat-{ver.expat}',
      gcc = f'gcc-{ver.gcc}',
      gdb = f'gdb-{ver.gdb}',
      gmp = f'gmp-{ver.gmp}',
      iconv = f'libiconv-{ver.iconv}',
      make = f'make-{ver.make}',
      mcfgthread = f'mcfgthread-{ver.mcfgthread}'
        if ver.thread == 'mcf'
        else None,
      mingw = f'mingw-w64-v{ver.mingw}',
      mingw_host = f'mingw-w64-v{ver.mingw}-host',
      mingw_target = f'mingw-w64-v{ver.mingw}-target',
      mingw_qt = f'mingw-w64-v{ver.mingw}-qt',
      mpc = f'mpc-{ver.mpc}',
      mpfr = f'mpfr-{ver.mpfr}',
      pdcurses = f'PDCurses-{ver.pdcurses}',
      pkgconf = f'pkgconf-pkgconf-{ver.pkgconf}',
      python = f'Python-{ver.python}',
      xmake = f'xmake-{ver.xmake}',
      z = f'zlib-{ver.z}',
    )

    self.src_dir = SourcePaths(
      binutils = self.build_dir / src_name.binutils,
      expat = self.build_dir / src_name.expat,
      gcc = self.build_dir / src_name.gcc,
      gdb = self.build_dir / src_name.gdb,
      gmp = self.build_dir / src_name.gmp,
      iconv = self.build_dir / src_name.iconv,
      make = self.build_dir / src_name.make,
      mcfgthread = self.build_dir / src_name.mcfgthread
        if ver.thread == 'mcf'
        else None,
      mingw = self.build_dir / src_name.mingw,
      mingw_host = self.build_dir / src_name.mingw_host,
      mingw_target = self.build_dir / src_name.mingw_target,
      mingw_qt = self.build_dir / src_name.mingw_qt,
      mpc = self.build_dir / src_name.mpc,
      mpfr = self.build_dir / src_name.mpfr,
      pdcurses = self.build_dir / src_name.pdcurses,
      pkgconf = self.build_dir / src_name.pkgconf,
      python = self.build_dir / src_name.python,
      xmake = self.build_dir / src_name.xmake,
      z = self.build_dir / src_name.z,
    )

    self.src_arx = SourcePaths(
      binutils = self.assets_dir / f'{src_name.binutils}.tar.zst'
        if Version(ver.binutils) >= Version('2.43')
        else self.assets_dir / f'{src_name.binutils}.tar.xz',
      expat = self.assets_dir / f'{src_name.expat}.tar.xz',
      gcc = self.assets_dir / f'{src_name.gcc}.tar.xz',
      gdb = self.assets_dir / f'{src_name.gdb}.tar.xz',
      gmp = self.assets_dir / f'{src_name.gmp}.tar.zst',
      iconv = self.assets_dir / f'{src_name.iconv}.tar.gz',
      make = self.assets_dir / f'{src_name.make}.tar.lz',
      mcfgthread = self.assets_dir / f'{src_name.mcfgthread}.tar.gz'
        if ver.thread == 'mcf'
        else None,
      mingw = self.assets_dir / f'{src_name.mingw}.tar.bz2',
      mingw_host = None,
      mingw_target = None,
      mingw_qt = None,
      mpc = self.assets_dir / f'{src_name.mpc}.tar.gz',
      mpfr = self.assets_dir / f'{src_name.mpfr}.tar.xz',
      pdcurses = self.assets_dir / f'{src_name.pdcurses}.tar.gz',
      python = self.assets_dir / f'{src_name.python}.tar.xz',
      pkgconf = self.assets_dir / f'{src_name.pkgconf}.tar.gz',
      xmake = self.assets_dir / f'{src_name.xmake}.tar.gz',
      z = self.assets_dir / f'{src_name.z}.tar.gz',
    )

    self.in_tree_src_dir = InTreeSourcePaths(
      intl = self.build_dir / 'intl',
      thunk = None,
    )

    self.in_tree_src_tree = InTreeSourcePaths(
      intl = self.root_dir / 'support/intl',
      thunk = self.root_dir / 'support/thunk',
    )

    layer_AAA_prefix = self.layer_dir / 'AAA'
    self.layer_AAA = LayerPathsAAA(
      prefix = layer_AAA_prefix,

      gmp = layer_AAA_prefix / 'gmp',
      mpc = layer_AAA_prefix / 'mpc',
      mpfr = layer_AAA_prefix / 'mpfr',
      z = layer_AAA_prefix / 'z',

      python = layer_AAA_prefix / 'python',
    )

    layer_AAB_prefix = self.layer_dir / 'AAB'
    self.layer_AAB = LayerPathsAAB(
      prefix = layer_AAB_prefix,

      binutils = layer_AAB_prefix / 'binutils',
      crt = layer_AAB_prefix / 'crt',
      gcc = layer_AAB_prefix / 'gcc',
      headers = layer_AAB_prefix / 'headers',

      expat = layer_AAB_prefix / 'expat',
      gmp = layer_AAB_prefix / 'gmp',
      iconv = layer_AAB_prefix / 'iconv',
      intl = layer_AAB_prefix / 'intl',
      mpc = layer_AAB_prefix / 'mpc',
      mpfr = layer_AAB_prefix / 'mpfr',
      pdcurses = layer_AAB_prefix / 'pdcurses',
      python = layer_AAB_prefix / 'python',
    )

    layer_ABB_prefix = self.layer_dir / 'ABB'
    self.layer_ABB = LayerPathsABB(
      prefix = layer_ABB_prefix,

      binutils = layer_ABB_prefix / 'binutils',
      crt = layer_ABB_prefix / 'crt',
      gcc = layer_ABB_prefix / 'gcc',
      gdb = layer_ABB_prefix / 'gdb',
      headers = layer_ABB_prefix / 'headers',
      make = layer_ABB_prefix / 'make',
      pkgconf = layer_ABB_prefix / 'pkgconf',
      xmake = layer_ABB_prefix / 'xmake',

      crt_qt = layer_ABB_prefix / 'crt_qt',

      license = layer_ABB_prefix / 'license',
    )

    # test phase

    self.test_dir = Path(f'{tempfile.gettempdir()}/{abi_name}')
    self.test_src_dir = self.root_dir / 'support' / 'test'

    self.test_mingw_dir = self.test_dir / abi_name

    # target semi-automated testing archive phase

    self.sat_dir = self.root_dir / 'pkg' / f'sat{config.profile}-{config.branch}'
    self.sat_mingw_dir = self.sat_dir / abi_name
