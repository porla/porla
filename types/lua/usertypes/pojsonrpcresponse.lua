---@meta

---@class PoJsonRpcResponse
local PoJsonRpcResponse = {}

function PoJsonRpcResponse:error(code, message) end

---@param data? any # The data (if any) to return to the client
function PoJsonRpcResponse:ok(data) end
