import('core.project.config')

function split_nm_archive_output(lines)
  local result = {} -- dict of object name: list of line
  local current_name = ''
  local current_object = {}
  for _, line in ipairs(lines) do
    -- line that starts a new object: 'libucrt.a[printf.o]:'
    local match = line:match('^.*%.a%[(.*%.o)%]:$')
    if match then
      if #current_object > 0 then
        result[current_name] = current_object
        current_object = {}
      end
      current_name = match
    else
      table.insert(current_object, line)
    end
  end
  if #current_object > 0 then
    result[current_name] = current_object
  end
  return result
end

function resolve_imp_symbols_from_nm_object_output(lines)
  local is_imp_object = false
  local result = {} -- list of symbol
  for _, line in ipairs(lines) do
    local symbol = line:split(' ')[1]
    if symbol:find('^__imp_') or symbol:find('^__force_override_mingw_emu__imp_') then
      table.insert(result, symbol)
    end
    -- '.idata$n'
    if symbol:find('^%.idata%$') then
      is_imp_object = true
    end
  end
  return is_imp_object, result
end

function resolve_overlay_symbols(archive, nm)
  local output = os.iorunv(nm, {'--format=posix', '--defined-only', archive})
  local objects = split_nm_archive_output(output:split('\n'))

  local result = {} -- dict of symbol: bool
  for object, content in pairs(objects) do
    local _, symbols = resolve_imp_symbols_from_nm_object_output(output:split('\n'))
    for _, symbol in ipairs(symbols) do
      result[symbol] = true
    end
  end
  return result
end

function resolve_non_overridden_objects(archive, nm, overlay_symbols)
  local output = os.iorunv(nm, {'--format=posix', '--defined-only', archive})
  local objects = split_nm_archive_output(output:split('\n'))

  local result = {} -- list of object name
  for object, content in pairs(objects) do
    local is_imp_object, symbols = resolve_imp_symbols_from_nm_object_output(content)
    if is_imp_object then
      if #symbols > 1 then
        raise(
          string.format('Import object %s contains multiple symbols: %s.',
          object,
          table.concat(symbols, ', ')))
      end
      if #symbols == 0 or not overlay_symbols[symbols[1]] then
        table.insert(result, object)
      end
    else
      -- not override: okay, add object
      -- marked force override (u8crt): okay, skip object
      -- unexpected override: raise error
      local force_override = false
      for _, symbol in ipairs(symbols) do
        if overlay_symbols[symbol] then
          local force_override_symbol = '__force_override_mingw_emu' .. symbol
          if overlay_symbols[force_override_symbol] then
            force_override = true
          else
            raise(
              string.format('Trying to override non import object %s with symbols: %s.',
              object,
              table.concat(symbols, ', ')))
          end
        end
      end
      if not force_override then
        table.insert(result, object)
      end
    end
  end
  return result
end

-- Squash overlay library to original import library.
-- 1. Check symbols from the two libraries.
-- 2. Create a new archive with overlay objects and non-overridden original objects.
function main(target)
  -- tools
  local ar = target:tool('ar')
  local nm = ar:replace('-ar$', '-nm')

  -- targets
  local name = target:name()
  local lib = target:targetfile()

  local overlay_name = 'overlay-' .. name
  local overlay_target = target:dep(overlay_name)
  local overlay_lib = overlay_target:targetfile()
  overlay_lib = path.absolute(overlay_lib)

  local mingw_prefix = config.get('mingw-prefix')
  local mingw_lib = path.join(mingw_prefix, 'lib', path.filename(lib))
  if not os.isfile(mingw_lib) then
    raise(string.format('Original import library not found at %s. Is `--mingw-prefix` set correctly?', mingw_lib))
  end

  -- check non-overridden objects
  local overlay_symbols = resolve_overlay_symbols(overlay_lib, nm)
  local non_overridden_objects = resolve_non_overridden_objects(mingw_lib, nm, overlay_symbols)

  -- create archive
  local mri_file = path.join(target:autogendir(), 'mri')
  local mri_content = {
    'create ' .. lib,
    'addlib ' .. overlay_lib,
    'addlib ' .. mingw_lib .. '(' .. table.concat(non_overridden_objects, ',') .. ')',
    'save',
    'end',
  }
  io.writefile(mri_file, table.concat(mri_content, '\n'))
  os.iorunv(ar, {'-M'}, {stdin = mri_file})
end
