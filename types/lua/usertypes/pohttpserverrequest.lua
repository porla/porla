---@meta

---@class PoHttpServerRequest
---@field method string
---@field headers table<string, string>
---@field path string the request path without the query string
---@field query string the raw (percent-encoded) query string without the "?"; empty when there is none
---@field url string the full request target as sent, path and query ("/x/foo?a=1")
