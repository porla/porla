---@meta

---@class PoHttpClientResponse
---@field body string
---@field headers table<string, string> the final response's headers, names in lowercase; a repeated header's values joined with ", "
---@field redirects integer redirects followed
---@field status integer
---@field time number total time in seconds
---@field url string the effective url, after redirects
