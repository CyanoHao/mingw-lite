import('core.project.config')

function parse_nm_output(lines)
  -- dict of {object_name: dict of {symbol_name: kind}}
  local result = {}

  for _, line in ipairs(lines) do
    local fields = line:split(' ')
    if #fields ~= 4 then
      raise(string.format('Malformed line from nm: %s.', line))
    end
    local file, symbol, kind, addr = table.unpack(fields)

    local object = file:match('%[(.*)%]:')
    if not object then
      raise(string.format('Malformed file name from nm: %s.', file))
    end

    if not result[object] then
      result[object] = {}
    end

    if symbol:find('^__imp_') or symbol:find('^__force_override_mingw_emu__imp_') then
      result[object][symbol] = kind
    end
  end

  return result
end

function resolve_overlay_symbols(archive, nm)
  -- dict of {symbol_name: true}
  local result = {}

  local output = os.iorunv(nm, {
    '--defined-only',
    '--format=posix',
    '--print-file-name',
    archive})
  local object_symbols = parse_nm_output(output:split('\n'))

  for object, symbols in pairs(object_symbols) do
    for symbol, kind in pairs(symbols) do
      result[symbol] = true
    end
  end
  return result
end

function resolve_non_overridden_objects(archive, nm, overlay_symbols)
  -- list of object_name
  local result = {}

  local output = os.iorunv(nm, {
    '--defined-only',
    '--format=posix',
    '--print-file-name',
    archive})
  local object_symbols = parse_nm_output(output:split('\n'))

  for object, symbols in pairs(object_symbols) do
    local all_yes = true
    local all_no = true
    for symbol, kind in pairs(symbols) do
      all_yes = all_yes and overlay_symbols[symbol]
      all_no = all_no and not overlay_symbols[symbol]

      if kind ~= 'I' and overlay_symbols[symbol] then
        -- trying to override non-import symbol
        local force_override_symbol = '__force_override_mingw_emu' .. symbol
        if not overlay_symbols[force_override_symbol] then
          raise(string.format('Trying to override non-import symbol %s in object %s.', symbol, object))
        end
      end
    end
    if not (all_yes or all_no) then
      raise(string.format('Object %s is partially overridden.', object))
    end
    if all_no then
      table.insert(result, object)
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
