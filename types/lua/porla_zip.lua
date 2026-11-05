---@meta porla_zip

local zip = {}

---Extracts every file entry into memory. Directory entries and entries without a known size are skipped.
---@param buffer string # The raw archive bytes
---@return table<string, string>? contents # File contents keyed by path inside the archive, or nil on error
---@return PoError? err # `decode_error` (`category = "libzip"`) if decoding failed
function zip.read(buffer) end

return zip
