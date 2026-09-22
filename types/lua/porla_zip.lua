---@meta porla_zip

local zip = {}

---@param buffer string
---@return table<string, string>? contents # The contents of the zip file, or nil if error
---@return string? err # Error if decoding failed
function zip.read(buffer) end

return zip
