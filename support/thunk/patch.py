#!/usr/bin/python3

import argparse
import os
from packaging.version import Version
from pathlib import Path
import re
import shutil
import subprocess

from module.thunk_list_core import THUNK_LIST_CORE_VCRT, THUNK_LIST_CORE_VCRTW
from module.thunk_list_core import THUNK_LIST_CORE_WIN32, THUNK_LIST_CORE_WIN32W
from module.thunk_list_qt import THUNK_LIST_QT_WIN32
from module.thunk_list_toolchain import THUNK_LIST_TOOLCHAIN_UCRTU
from module.thunk_list_toolchain import THUNK_LIST_TOOLCHAIN_VCRT, THUNK_LIST_TOOLCHAIN_VCRTU, THUNK_LIST_TOOLCHAIN_VCRTW
from module.thunk_list_toolchain import THUNK_LIST_TOOLCHAIN_WIN32, THUNK_LIST_TOOLCHAIN_WIN32U, THUNK_LIST_TOOLCHAIN_WIN32W

ARCH_DEFAULT_VERSION_MAP = {
  '32': '4.0',
  '64': '5.2',
  'arm64': '10.0.19041',
}

ARCH_LIB_DIR_MAP = {
  '32': 'lib32',
  '64': 'lib64',
  'arm64': 'libarm64',
}

ARCH_DLL_DEF_CPP_DEFINES_MAP = {
  '32': {
    'DEF_I386': '1',
    '__i386__': '1',
    '__SIZEOF_DOUBLE__': '8',
    '__SIZEOF_LONG_DOUBLE__': '12',
  },
  '64': {
    'DEF_X64': '1',
    '__x86_64__': '1',
    '__SIZEOF_DOUBLE__': '8',
    '__SIZEOF_LONG_DOUBLE__': '16',
  },
  'arm64': {
    'DEF_ARM64': '1',
    '__aarch64__': '1',
    '__SIZEOF_DOUBLE__': '8',
    '__SIZEOF_LONG_DOUBLE__': '8',
  },
}

ARCH_CPPFLAGS_MAP = {
  '32': 'CPPFLAGS32',
  '64': 'CPPFLAGS64',
  'arm64': 'CPPFLAGSARM64',
}

ARCH_DT_DEF_COMMAND_MAP = {
  '32': 'DTDEF32',
  '64': 'DTDEF64',
  'arm64': 'DTDEFARM64',
}

def parse_args():
  parser = argparse.ArgumentParser(
    description = 'Patch mingw-w64 source code'
  )
  parser.add_argument(
    'source',
    type = Path,
    help = 'Path to mingw-w64 source code'
  )
  parser.add_argument(
    '-a', '--arch',
    type = str,
    choices = ARCH_DEFAULT_VERSION_MAP.keys(),
    required = True,
    help = 'Target architecture'
  )
  parser.add_argument(
    '--level',
    type = str,
    choices = ['core', 'toolchain', 'qt'],
    required = True,
    help = 'Thunk level'
  )
  parser.add_argument(
    '--nt-ver',
    type = Version,
    help = 'Target Windows NT version'
  )
  parser.add_argument(
    '--msvcrt',
    action = 'store_true',
    help = 'Add MSVCRT thunks'
  )
  parser.add_argument(
    '--assert-thunk-free',
    action = 'store_true',
    help = 'Assert no thunk is applied (to guarantee ABI stability)'
  )
  args = parser.parse_args()
  if not args.nt_ver:
    args.nt_ver = Version(ARCH_DEFAULT_VERSION_MAP[args.arch])
  return args

def copy_headers(args):
  crt_dir = args.source / 'mingw-w64-crt'
  shutil.copytree('include', crt_dir, dirs_exist_ok = True)

def resolve_mingw_version(args):
  if not hasattr(resolve_mingw_version, 'saved_version'):
    mingw_mac_path = args.source / 'mingw-w64-headers' / 'crt' / '_mingw_mac.h'
    with open(mingw_mac_path, 'r') as f:
      v_mingw_major_pattern = re.compile(r'#define\s+__MINGW64_VERSION_MAJOR\s+(\d+)')
      v_mingw_major_str = v_mingw_major_pattern.search(f.read()).group(1)
      resolve_mingw_version.saved_version = int(v_mingw_major_str)

  return resolve_mingw_version.saved_version

def resolve_single_module(args, module, thunk_list, directory):
  v_mingw = resolve_mingw_version(args)
  crt_dir = args.source / 'mingw-w64-crt'
  for func_name in thunk_list:
    if type(func_name) == tuple:
      func_name, cond = func_name
      if not cond(v_mingw):
        continue
    shutil.copy(f'{directory}/{func_name}.cc', crt_dir / f'thunk/{func_name}.cc')
    module.append(func_name)

def resolve_multi_modules(args, modules, thunk_list, directory):
  for (mod_name, func_list) in thunk_list.items():
    if mod_name not in modules:
      modules[mod_name] = []
    resolve_single_module(args, modules[mod_name], func_list, f'{directory}/{mod_name}')

def resolve_vcrt_module(args, module, thunk_list):
  for (ver_str, func_list) in thunk_list.items():
    v = Version(ver_str)
    if v <= args.nt_ver:
      continue
    resolve_single_module(args, module, func_list, f'vcrt/{ver_str}')

def resolve_win32_modules(args, modules, thunk_list):
  for (ver_str, mod_list) in thunk_list.items():
    v = Version(ver_str)
    if v <= args.nt_ver:
      continue
    resolve_multi_modules(args, modules, mod_list, f'win32/{ver_str}')

def generate_crt_def(args, basename):
  crt_dir = args.source / 'mingw-w64-crt'
  crt_def_in = crt_dir / f'lib-common/{basename}.def.in'
  crt_def = crt_dir / ARCH_LIB_DIR_MAP[args.arch] / f'{basename}.def'
  def_include = crt_dir / 'def-include'
  subprocess.run([
    'cpp',
    '-undef',
    *map(
      lambda entry: f'-D{entry[0]}={entry[1]}',
      ARCH_DLL_DEF_CPP_DEFINES_MAP[args.arch].items(),
    ),
    '-I', def_include,
    crt_def_in,
    '-P',
    '-o', crt_def,
  ], check = True)
  return crt_def

def locate_mod_def_file(possible_paths):
  for path in possible_paths:
    if path.exists():
      return path
  raise FileNotFoundError('No suitable .def file found')

def patch_mod_def_file(args, def_path, func_list, winapi = False, crt_alias = False):
  with open(def_path, 'r') as f:
    def_content = f.readlines()
  for func_name in func_list:
    if crt_alias:
      pattern = re.compile(rf'^{func_name}(\s*==\s*\w+)?(\s*;.*)?$')
    elif winapi and args.arch == '32':
      pattern = re.compile(rf'^{func_name}(@\d*)?(\s*;.*)?$')
    else:
      pattern = re.compile(rf'^{func_name}(\s*;.*)?$')
    export_line_found = False
    for i, line in enumerate(def_content):
      if pattern.match(line):
        def_content[i] = '; ' + line
        export_line_found = True
        break
    if not export_line_found:
      raise RuntimeError(f'Failed to replace {func_name} in {def_path}')
  with open(def_path, 'w') as f:
    f.writelines(def_content)

def patch_automake_template(args, normal_modules, vcrt_module):
  # load automake template
  AM_ADDITION_START = '# <<< mingw-thunk addition\n'
  AM_ADDITION_END = '# >>> mingw-thunk addition\n'
  am_file = args.source / 'mingw-w64-crt' / 'Makefile.am'
  with open(am_file, 'r') as f:
    am_content = f.readlines()

  # patch automake cflags
  cflags_line = -1
  cxxflags_line = -1
  cflags_pattern = re.compile(r'^AM_CFLAGS\s*=')
  cxxflags_pattern = re.compile(r'^AM_CXXFLAGS\s*=')
  for i, line in enumerate(am_content):
    if cflags_pattern.match(line):
      cflags_line = i
    if cxxflags_pattern.match(line):
      cxxflags_line = i
  if cflags_line < 0 or cxxflags_line < 0:
    raise RuntimeError('Cannot find cflags or cxxflags in Makefile.am')
  cflags = cflags_pattern.sub('', am_content[cflags_line]).strip().split()
  cxxflags = [
    '-nostdinc++', '-fno-exceptions', '-fno-threadsafe-statics',
    '-DNOSTL_NOCRT', '-DNS_NOSTL=mingw_thunk::stl', '-DNS_NOCRT=mingw_thunk::libc',
  ]
  for flag in cflags:
    if flag.startswith('-std='):
      cxxflags.append('-std=gnu++17')
    elif flag.startswith('-D__MSVCRT_VERSION__='):
      if not args.msvcrt:
        cxxflags.append('-D_UCRT')
      else:
        cxxflags.append(flag)
    elif '_C_ONLY_' in flag:
      cxxflags.append(flag.replace('_C_ONLY_', '_CXX_ONLY_'))
    else:
      cxxflags.append(flag)
  am_content[cxxflags_line] = 'AM_CXXFLAGS=' + ' '.join(cxxflags) + '\n'

  # process win32 modules
  am_addition = []
  lib_dir = ARCH_LIB_DIR_MAP[args.arch]
  arch_def_dir = args.source / 'mingw-w64-crt' / lib_dir
  common_def_dir = args.source / 'mingw-w64-crt' / 'lib-common'
  for mod_name, func_list in normal_modules.items():
    # sanitize 'api-ms-win-...'
    sanitized_mod_name = mod_name.replace('-', '_')

    # find module def file
    mod_def_file_paths = [
      arch_def_dir / (mod_name + '.def.in'),
      arch_def_dir / (mod_name + '.def'),
      common_def_dir / (mod_name + '.def.in'),
      common_def_dir / (mod_name + '.def'),
    ]
    mod_def_file = locate_mod_def_file(mod_def_file_paths)
    if mod_name.startswith('api-ms-win-crt-'):
      patch_mod_def_file(args, mod_def_file, func_list, crt_alias = True)
    else:
      patch_mod_def_file(args, mod_def_file, func_list, winapi = True)

    # add thunk files to makefile rules
    am_src = list(map(lambda func: f'thunk/{func}.cc', func_list))
    am_lib_name = f'{lib_dir}_lib{sanitized_mod_name}_a'

    # do we have a lib<mod_name>.a rule already?
    # then append to existing one or create new one
    am_lib_sources_pattern = re.compile(fr'^{am_lib_name}_SOURCES\s*=')
    am_lib_sources_line = -1
    for i, line in enumerate(am_content):
      if am_lib_sources_pattern.match(line):
        am_lib_sources_line = i
    if am_lib_sources_line >= 0:
      am_addition.append(f'{am_lib_name}_SOURCES += ' + ' '.join(am_src) + '\n')
    else:
      cppflags = ARCH_CPPFLAGS_MAP[args.arch]
      dt_def_command = ARCH_DT_DEF_COMMAND_MAP[args.arch]
      am_addition.append(f'{lib_dir}_LIBRARIES += {lib_dir}/lib{mod_name}.a\n')
      am_addition.append(f'{am_lib_name}_SOURCES = {" ".join(am_src)}\n')
      am_addition.append(f'{am_lib_name}_CPPFLAGS = $({cppflags}) $(sysincludes)\n')
      rel_def_path = mod_def_file.relative_to(args.source / 'mingw-w64-crt')
      am_addition.append(f'{am_lib_name}_AR = $({dt_def_command}) $(top_srcdir)/{rel_def_path} && $(AR) $(ARFLAGS)\n')

  # process msvcrt def
  # some definitions are shared among multiple versions by `crt-aliases.def.in`,
  # here we have to generate msvcrt.def and patch it instead.
  if args.msvcrt:
    crt_basename = 'msvcrt'
    crt_def_file = generate_crt_def(args, crt_basename)
    am_lib_name = f'{lib_dir}_libmsvcrt_extra_a'

    # remove def from `make clean`
    clean_crt_def_found = False
    pattern = f'{lib_dir}/{crt_basename}.def'
    for i, line in enumerate(am_content):
      if pattern in line:
        am_content[i] = line.replace(pattern, '')
        clean_crt_def_found = True
        break
    if not clean_crt_def_found:
      raise RuntimeError('Failed to locate clean rule for msvcrt.def')
    patch_mod_def_file(args, crt_def_file, vcrt_module, crt_alias = True)

    # protect from cygwin w32api
    am_addition.append(f'if !W32API\n')
    am_src = list(map(lambda func: f'thunk/{func}.cc', vcrt_module))
    am_addition.append(f'{am_lib_name}_SOURCES += ' + ' '.join(am_src) + '\n')
    am_addition.append(f'endif\n')

  with open(am_file, 'w') as f:
    f.writelines(am_content + [AM_ADDITION_START, f'if {lib_dir.upper()}\n'] + am_addition + ['endif\n', AM_ADDITION_END])

def main():
  args = parse_args()
  win9x = args.nt_ver < Version('4.0')
  copy_headers(args)

  vcrt_module = []
  normal_modules = {}

  if args.level == 'core':
    if args.msvcrt:
      resolve_vcrt_module(args, vcrt_module, THUNK_LIST_CORE_VCRT)
      if win9x:
        resolve_single_module(args, vcrt_module, THUNK_LIST_CORE_VCRTW, 'vcrtw')

    resolve_win32_modules(args, normal_modules, THUNK_LIST_CORE_WIN32)
    if win9x:
      resolve_multi_modules(args, normal_modules, THUNK_LIST_CORE_WIN32W, 'win32w')

  elif args.level == 'toolchain':
    if args.msvcrt:
      resolve_vcrt_module(args, vcrt_module, THUNK_LIST_TOOLCHAIN_VCRT)
      resolve_single_module(args, vcrt_module, THUNK_LIST_TOOLCHAIN_VCRTU, 'vcrtu')
      if win9x:
        resolve_single_module(args, vcrt_module, THUNK_LIST_TOOLCHAIN_VCRTW, 'vcrtw')
    else:
      # no special handling for UCRT
      resolve_multi_modules(args, normal_modules, THUNK_LIST_TOOLCHAIN_UCRTU, 'ucrtu')

    resolve_win32_modules(args, normal_modules, THUNK_LIST_TOOLCHAIN_WIN32)
    resolve_multi_modules(args, normal_modules, THUNK_LIST_TOOLCHAIN_WIN32U, 'win32u')
    if win9x:
      resolve_multi_modules(args, normal_modules, THUNK_LIST_TOOLCHAIN_WIN32W, 'win32w')

  elif args.level == 'qt':
    resolve_win32_modules(args, normal_modules, THUNK_LIST_QT_WIN32)

  else:
    raise RuntimeError('Invalid level')

  if args.assert_thunk_free:
    if any(v for _, v in normal_modules.items()) or vcrt_module:
      print('Oops there are thunks:')
      print(normal_modules)
      print(vcrt_module)
      raise RuntimeError('ABI stability broken')

  patch_automake_template(args, normal_modules, vcrt_module)

if __name__ == '__main__':
  main()
