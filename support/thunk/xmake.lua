add_rules(
  'mode.debug',
  'mode.minsizerel',
  'mode.release',
  'mode.releasedbg')
set_languages('c11', 'cxx17')
set_plat('mingw')

function enable_if_x86()
  -- set_enabled(is_arch('i386', 'i686', 'x86_64')) doesn't work with Xmake 2.8.7 (Ubuntu 24.04)
  if is_arch('i386', 'i686', 'x86_64') then
    set_enabled(true)
  else
    set_enabled(false)
  end
end

function enable_if_x86_32()
  if is_arch('i386', 'i686') then
    set_enabled(true)
  else
    set_enabled(false)
  end
end

function enable_squash_options()
  on_build('module.squash_overlay')
  set_enabled(get_config('mingw-prefix') ~= '')
  set_kind('static')
end

function enable_test_options()
  add_defines(
    'NOMINMAX',
    'NOSTL_NOCRT',
    'NS_NOSTL=mingw_thunk::stl',
    'NS_NOCRT=mingw_thunk::libc',
    'WIN32_LEAN_AND_MEAN',
    '_WIN32_WINNT=0x0A00',
    '__USE_MINGW_ANSI_STDIO=0')
  add_deps('catch2')
  add_includedirs('include')
  add_ldflags('-static')
  set_default(false)
end

function enable_thunk_options()
  add_cxxflags(
    -- set_exceptions('none') doesn't work with Xmake 2.8.7 (Ubuntu 24.04)
    '-fno-exceptions',
    '-fno-threadsafe-statics',
    '-nostdinc++')
  add_defines(
    'NOMINMAX',
    'NOSTL_NOCRT',
    'NS_NOSTL=mingw_thunk::stl',
    'NS_NOCRT=mingw_thunk::libc',
    'THUNK_LEVEL=' .. ntddi_version(),
    'WIN32_LEAN_AND_MEAN',
    '_WIN32_WINNT=0x0A00',
    '__USE_MINGW_ANSI_STDIO=0')
  add_files('placeholder/placeholder.c')
  add_includedirs('include')
  set_exceptions('none')
  set_kind('static')
end

function profile_core()
  return get_config('profile') == 'core'
end

function profile_toolchain()
  return get_config('profile') == 'toolchain'
end

function profile_toolchain_or_utf8()
  return profile_toolchain() or profile_toolchain_utf8()
end

function profile_toolchain_utf8()
  return get_config('profile') == 'toolchain-utf8'
end

function skip_install()
  on_install(function (target) end)
end

option('mingw-prefix')
  set_default('')
  set_showmenu(true)

option('mingw-version')
  set_default(11)
  set_showmenu(true)
  set_values(11, 12, 13, 14)

option('profile')
  set_default('toolchain-utf8')
  set_showmenu(true)
  set_values(
    'core',
    'qt',
    'toolchain',
    'toolchain-utf8')

option('thunk-level')
  set_showmenu(true)
  set_values(
    '10.0.19041',  -- Windows 10 2004
    '10.0.10240',  -- Windows 10 1507
    '6.3',         -- Windows 8.1
    '6.2',         -- Windows 8
    '6.1',         -- Windows 7
    '6.0',         -- Windows Vista
    '5.2',         -- Windows Server 2003
    '5.1',         -- Windows XP
    '5.0',         -- Windows 2000
    '4.0',         -- Windows NT 4.0
    '3.9999+4.90', -- Windows Me
    '3.9999+4.10', -- Windows 98
    '3.9999+4.00') -- Windows 95

  if is_arch('i386', 'i686') then
    set_default('3.9999+4.00')
  elseif is_arch('x86_64') then
    set_default('5.2')
  elseif is_arch('aarch64') then
    set_default('10.0.19041')
  end

rule('dlltool')
  set_extensions('.def')
  on_build_files(function (target, sourcebatch, opt)
    import("core.project.depend")

    local ar = target:tool('ar')
    local dlltool = ar:replace('-ar$', '-dlltool')

    local tmpdir = path.join(target:autogendir(), 'dlltool')
    os.mkdir(tmpdir)

    local objdir = path.join(tmpdir, 'objects')
    os.rm(path.join(objdir, '*.o'))
    os.mkdir(objdir)

    for _, sourcefile in ipairs(sourcebatch.sourcefiles) do
      -- generate import lib
      local temp_a = path.join(tmpdir, path.filename(sourcefile) .. '.a')
      os.runv(dlltool, {'--output-lib', temp_a, '--input-def', sourcefile})

      -- extract objects
      os.runv(ar, {'x', path.absolute(temp_a)}, {curdir = objdir})
    end

    -- add objects to target
    local objects = os.files(path.join(objdir, '*.o'))
    for _, object in ipairs(objects) do
      table.insert(target:objectfiles(), object)
    end
  end)

includes('dep/catch2')

includes('ntddi_version.lua')

includes('target_bcrypt.lua')
includes('target_kernel32.lua')
includes('target_msvcrt.lua')
includes('target_pathcch.lua')
includes('target_shell32.lua')
includes('target_ws2_32.lua')
