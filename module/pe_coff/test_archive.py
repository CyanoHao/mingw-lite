from pathlib import Path

import pytest

from . import read_archive_members

_TEST_RES = Path(__file__).parent / 'res'
_NONE = _TEST_RES / 'none'


def test_non_archive_raises() -> None:
  with pytest.raises(ValueError):
    read_archive_members(_NONE / 'notlib.a')


def test_empty_archive_returns_empty() -> None:
  assert read_archive_members(_NONE / 'empty.a') == []


def test_long_name_table() -> None:
  members = read_archive_members(_NONE / 'name.a')
  assert [(m.name, m.data) for m in members] == [
    ('simple.o', b'simple\n'),
    ('object_with_very_long_name.o', b'object with very long name\n'),
  ]
