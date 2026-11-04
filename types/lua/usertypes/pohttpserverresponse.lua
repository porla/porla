---@meta

---@class PoHttpServerResponse
local PoHttpServerResponse = {}

---@param data? string
function PoHttpServerResponse:finish(data) end

---@param data string
function PoHttpServerResponse:write(data) end

---@param key string
---@param value string
function PoHttpServerResponse:write_header(key, value) end

--- Writes the given HTTP status to the response.
--- 
--- ```lua
--- res:write_status("200 OK")
--- ```
---@param status string # The HTTP status line
function PoHttpServerResponse:write_status(status) end
