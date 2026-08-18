#!/usr/bin/python3

import argparse
import json
from typing import TypedDict

class SatGroup(TypedDict):
  name: str
  items: list[dict[str, str]]
  pattern: str
  dict: str

parser = argparse.ArgumentParser()
parser.add_argument('--ref-type')
parser.add_argument('--ref-name')
args = parser.parse_args()

all_branch = [
  'next', 'current',
  '16', '16+emutls', '15', '14', '13',
]

common_profile = [
  '64-mcf', '64-win32', '64-ucrt', '64-msvcrt',
  '32-mcf', '32-win32', '32-ucrt', '32-msvcrt',

  # profile variants for micro architectures
  '64_v2-mcf', '64_v2-win32', '64_v2-ucrt', '64_v2-msvcrt',
]
all_old_profile = [
  '64-ucrt_ws2003', '64-msvcrt_ws2003',
  '32-ucrt_winxp',  '32-msvcrt_win2000',
  '32_686-msvcrt_win98',
  '32_486-msvcrt_win98',
  '32_386-msvcrt_win95',
]
release_old_profile = [
  '32-msvcrt_win2000',
  '32_686-msvcrt_win98',
  '32_486-msvcrt_win98',
]
beyond_profile = [
  # u8crt
  '64-u8crt', '32-u8crt',

  # optimization level
  '64-ucrt_og', '64-ucrt_o1', '64-ucrt_oz', '64-ucrt_os', '64-ucrt_o3',
]

exclude_profile_branch = [
  *( # native TLS removal
    {'profile': '32_386-msvcrt_win95', 'branch': b}
    for b in ['next', 'current', '16']
  ),
  *( # native TLS
    {'profile': p, 'branch': b}
    # 32-msvcrt_win2000 was initially added because gcc was mislead by Microsoft docs
    # and its UTF-8 manifest was not compatible with Windows XP.
    # Windows 2000 is the first NT version that guarantees SSE support,
    # and is so close to Windows XP, so we support it without difficulty.

    # 32-msvcrt_win2000 was deprecated since we fixed the manifest conformance.
    # Windows XP was the oldest NT version supported by upstream at that time,
    # so we set it as the new baseline (more specifically, thunk-free).
    # 32-msvcrt_win2000 was planned to be removed in branch 16.

    # as GCC 16 introduces native TLS, and some newer APIs are called by standard library,
    # the baseline is raised to Windows Vista. Windows XP is still supported for its usage.
    # for the same reason we initially supported Windows 2000, 32-msvcrt_win2000 is de-deprecated.

    # it's a coincidence that 32-msvcrt_win2000 by any branch is not excluded.
    for p in [
      '64-ucrt_ws2003', '64-msvcrt_ws2003',
      '32-ucrt_winxp',
    ]
    for b in ['15', '14', '13']
  ),
  *( # emutls
    {'profile': f'{bit}-{abi}', 'branch': '16+emutls'}
    for bit in ['64', '32', '64_v2']
    for abi in ['mcf', 'win32', 'ucrt', 'msvcrt']
  ),
  *( # u8crt
    {'profile': p, 'branch': b}
    for p in ['64-u8crt', '32-u8crt']
    for b in ['16', '16+emutls', '15', '14', '13']
  ),
  *( # optimization level
    {'profile': p, 'branch': b}
    for p in ['64-ucrt_og', '64-ucrt_o1', '64-ucrt_oz', '64-ucrt_os', '64-ucrt_o3']
    for b in ['16', '16+emutls', '15', '14', '13']
  ),
]

alt_branch = ['current']
alt_profile = ['64-mcf']
alt_osrel = [
  'archlinux',
  'debian13', 'debian12', 'debian11',
  'ubuntu2604',
  'alpine324',
]

# branch subsets for SAT group construction
_b_std = ['next', 'current', '16', '15', '14', '13']
_b_std_17_16 = ['next', 'current', '16']  # baseline vista; default to native TLS
_b_std_15_14_13 = ['15', '14', '13']      # baseline xp; default to emulated TLS
_b_dev = ['next', 'current']
_b_emutls = ['16+emutls']
_b_non_conforming_manifest = ['15', '14', '13']  # see above (32-msvcrt_win2000)

sat_group: list[SatGroup] = [
  {
    'name': '64-nt61',
    'items': [{'profile': '64-mcf', 'branch': b} for b in _b_std],
    'pattern': 'mingw64-mcf-*',
    'dict': '1024m',
  },
  {
    'name': '64_v2-nt61',
    'items': [{'profile': '64_v2-mcf', 'branch': b} for b in _b_std],
    'pattern': 'mingw64_v2-mcf-*',
    'dict': '1024m',
  },
  {
    'name': '32-nt61',
    'items': [{'profile': '32-mcf', 'branch': b} for b in _b_std],
    'pattern': 'mingw32-mcf-*',
    # 32-bit limited address space
    'dict': '512m',
  },
  {
    'name': '64-nt60',
    'items': [
      *({'profile': '64-win32',   'branch': b} for b in _b_std),
      *({'profile': '64-ucrt',    'branch': b} for b in _b_std_17_16),
      *({'profile': '64-msvcrt',  'branch': b} for b in _b_std_17_16),
    ],
    'pattern': '{mingw64-win32-*,mingw64-ucrt-*,mingw64-msvcrt-*}',
    'dict': '1024m',
  },
  {
    'name': '64_v2-nt60',
    'items': [
      *({'profile': '64_v2-win32',  'branch': b} for b in _b_std),
      *({'profile': '64_v2-ucrt',   'branch': b} for b in _b_std_17_16),
      *({'profile': '64_v2-msvcrt', 'branch': b} for b in _b_std_17_16),
    ],
    'pattern': '{mingw64_v2-win32-*,mingw64_v2-ucrt-*,mingw64_v2-msvcrt-*}',
    'dict': '1024m',
  },
  {
    'name': '32-nt60',
    'items': [
      *({'profile': '32-win32',  'branch': b} for b in _b_std),
      *({'profile': '32-ucrt',   'branch': b} for b in _b_std_17_16),
      *({'profile': '32-msvcrt', 'branch': b} for b in _b_std_17_16),
    ],
    'pattern': '{mingw32-win32-*,mingw32-ucrt-*,mingw32-msvcrt-*}',
    # 32-bit limited address space
    'dict': '512m',
  },
  {
    'name': '64-nt52',
    'items': [
      *({'profile': '64-ucrt',          'branch': b} for b in _b_std_15_14_13),
      *({'profile': '64-msvcrt',        'branch': b} for b in _b_std_15_14_13),
      *({'profile': '64-ucrt_ws2003',   'branch': b} for b in _b_std_17_16 + _b_emutls),
      *({'profile': '64-msvcrt_ws2003', 'branch': b} for b in _b_std_17_16 + _b_emutls),
    ],
    'pattern': '{mingw64-ucrt-*,mingw64-msvcrt-*,mingw64-ucrt_ws2003-*,mingw64-msvcrt_ws2003-*}',
    'dict': '1024m',
  },
  {
    'name': '64_v2-nt52',
    'items': [
      *({'profile': '64_v2-ucrt',   'branch': b} for b in _b_std_15_14_13),
      *({'profile': '64_v2-msvcrt', 'branch': b} for b in _b_std_15_14_13),
    ],
    'pattern': '{mingw64_v2-ucrt-*,mingw64_v2-msvcrt-*}',
    'dict': '1024m',
  },
  {
    'name': '32-nt51',
    'items': [
      *({'profile': '32-ucrt',       'branch': b} for b in _b_std_15_14_13),
      *({'profile': '32-msvcrt',     'branch': b} for b in _b_std_15_14_13),
      *({'profile': '32-ucrt_winxp', 'branch': b} for b in _b_std_17_16 + _b_emutls),
    ],
    'pattern': '{mingw32-ucrt-*,mingw32-msvcrt-*,mingw32-ucrt_winxp-*}',
    # 32-bit limited address space
    'dict': '512m',
  },
  {
    'name': '32-nt50',
    'items': [{'profile': '32-msvcrt_win2000', 'branch': b} for b in _b_std_17_16 + _b_emutls + _b_non_conforming_manifest],
    'pattern': 'mingw32-msvcrt_win2000-*',
    # 32-bit limited address space
    'dict': '512m',
  },
  {
    'name': '32_686-410',
    'items': [{'profile': '32_686-msvcrt_win98', 'branch': b} for b in _b_std + _b_emutls],
    'pattern': 'mingw32_686-msvcrt_win98-*',
    # 9x limited memory
    'dict': '128m',
  },
  {
    'name': '32_486-410',
    'items': [{'profile': '32_486-msvcrt_win98', 'branch': b} for b in _b_std + _b_emutls],
    'pattern': 'mingw32_486-msvcrt_win98-*',
    # 9x limited memory
    'dict': '128m',
  },
  {
    'name': '32_386-400',
    'items': [{'profile': '32_386-msvcrt_win95', 'branch': b} for b in _b_emutls + _b_std_15_14_13],
    'pattern': 'mingw32_386-msvcrt_win95-*',
    # 9x limited memory
    'dict': '128m',
  },
  {
    'name': '-u8crt',
    'items': [
      *({'profile': '64-u8crt', 'branch': b} for b in _b_dev),
      *({'profile': '32-u8crt', 'branch': b} for b in _b_dev),
    ],
    'pattern': 'mingw*-u8crt-*',
    # 32-bit limited address space
    'dict': '512m',
  },
  {
    'name': '-optimize',
    'items': [
      *({'profile': '64-ucrt_og', 'branch': b} for b in _b_dev),
      *({'profile': '64-ucrt_o1', 'branch': b} for b in _b_dev),
      *({'profile': '64-ucrt_oz', 'branch': b} for b in _b_dev),
      *({'profile': '64-ucrt_os', 'branch': b} for b in _b_dev),
      *({'profile': '64-ucrt_o3', 'branch': b} for b in _b_dev),
    ],
    'pattern': 'mingw64-ucrt_o*-*',
    'dict': '1024m',
  },
]

sat_group_repr = [
  {
    'name': g['name'],
    'items': json.dumps(g['items']),
    'pattern': g['pattern'],
    'dict': g['dict'],
  } for g in sat_group
]

def check_sat_coverage():
  sat_expected = {
    (p, b)
    for p in common_profile + all_old_profile + beyond_profile
    for b in all_branch
    if {'profile': p, 'branch': b} not in exclude_profile_branch
  }

  sat_covered: set[tuple[str, str]] = set()
  for g in sat_group:
    for item in g['items']:
        p = (item['profile'], item['branch'])
        assert p not in sat_covered, f'duplicate SAT entry: {p}'
        sat_covered.add(p)

  assert sat_covered == sat_expected, (
    f'SAT coverage mismatch:\n'
    f'  missing: {sat_expected - sat_covered}\n'
    f'  extra:   {sat_covered - sat_expected}'
  )
check_sat_coverage()

if args.ref_type == 'tag':
  base = args.ref_name.split('-')[0]
  suffix = ('+' + base.split('+', 1)[1]) if '+' in base else ''
  current_branch = base.split('+')[0].split('.')[0] + suffix
  branch = [current_branch]
  profile = [
    p for p in (common_profile + release_old_profile)
    if not any(
      p == x['profile'] and current_branch == x['branch']
      for x in exclude_profile_branch)]
  exclude_profile_branch = []
  release = True
  prerelease = not current_branch.isdigit() or int(current_branch) >= 17
else:
  branch = all_branch
  profile = common_profile + all_old_profile + beyond_profile
  release = False
  prerelease = True

print(f'branch={json.dumps(branch)}')
print(f'profile={json.dumps(profile)}')
print(f'exclude_profile_branch={json.dumps(exclude_profile_branch)}')
print(f'alt_branch={json.dumps(alt_branch)}')
print(f'alt_profile={json.dumps(alt_profile)}')
print(f'alt_osrel={json.dumps(alt_osrel)}')
print(f'release={json.dumps(release)}')
print(f'prerelease={json.dumps(prerelease)}')
print(f'sat_group={json.dumps(sat_group_repr)}')
