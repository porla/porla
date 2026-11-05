---@meta

---Every method is a no-op once the response is finished or the client has disconnected.
---@class PoHttpServerResponse
local PoHttpServerResponse = {}

---Ends the response. Without a prior `write`, `data` is sent with a `Content-Length`.
---@param data? string
function PoHttpServerResponse:finish(data) end

---Sends a body chunk (chunked transfer encoding). Call `finish` when done.
---@param data string
function PoHttpServerResponse:write(data) end

---Sends `200 OK` first unless `write_status` was called.
---@param key string
---@param value string
function PoHttpServerResponse:write_header(key, value) end

--- Writes the given HTTP status to the response. Must come before `write_header` and `write`; only the first
--- call counts.
---
--- ```lua
--- res:write_status("200 OK")
--- ```
---@param status string # The HTTP status line
function PoHttpServerResponse:write_status(status) end
