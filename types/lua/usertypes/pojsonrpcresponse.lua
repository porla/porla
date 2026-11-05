---@meta

---@class PoJsonRpcResponse
local PoJsonRpcResponse = {}

---@param code integer
---@param message string
---@param data? any # Sent as the error's `data`
---@overload fun(self: PoJsonRpcResponse, err: PoError) # Replies with the registry code for `err.code`; `data` carries `reason`, `category` and `value`
function PoJsonRpcResponse:error(code, message, data) end

---@param data? any # The data (if any) to return to the client
function PoJsonRpcResponse:ok(data) end
