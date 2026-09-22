---@meta

---@class PoHttpServerResponse
local PoHttpServerResponse = {}

---@param data? string
function PoHttpServerResponse:finish(data) end

---@param data string
function PoHttpServerResponse:write(data) end

---@param key string
---@param value string
function PoHttpServerResponse:writeHeader(key, value) end

--- Writes the given HTTP status to the response.
--- 
--- ```lua
--- res:writeStatus("200 OK")
--- ```
---@param status string # The HTTP status line
function PoHttpServerResponse:writeStatus(status) end
