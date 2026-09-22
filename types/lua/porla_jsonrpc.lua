---@meta porla_jsonrpc

local jsonrpc = {}

---@param method string
---@param callback fun(params: any, res: PoJsonRpcResponse)
function jsonrpc.register(method, callback) end

return jsonrpc
