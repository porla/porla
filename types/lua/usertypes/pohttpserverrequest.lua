---@meta

---@class PoHttpServerRequest
---@field method string uppercase, e.g. "GET"
---@field body string the raw request body, forms included; empty when the request has none
---@field form? PoHttpServerRequestForm nil unless the content type is `multipart/form-data` or `application/x-www-form-urlencoded`
---@field headers table<string, string> names in lowercase; of a repeated header only the last value is kept
---@field path string the request path without the query string, not percent-decoded
---@field query string the raw (percent-encoded) query string without the "?"; empty when there is none
---@field url string the full request target as sent, path and query ("/x/foo?a=1")

---@class PoHttpServerRequestForm
---@field kind "multipart"|"urlencoded"
---@field ok boolean false if a multipart body is empty, has no usable boundary or more than 256 parts (`fields` and `files` may then be partial); parsing stops silently at a malformed part
---@field fields table<string, string> decoded values by field name; a repeated name keeps the last value
---@field files PoHttpFormFile[] multipart parts sent with a filename; always empty for urlencoded forms
