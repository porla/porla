---@meta porla_jsonrpc

local jsonrpc = {}

---Without `can_invoke` only signed-in users may call the method; API keys and anonymous callers get
---`permission_denied`. With `can_invoke` it alone decides for every caller, anonymous included, and a raise
---or a non-boolean return denies. Raises `already_registered` if the name is taken, built-in methods included.
---The method is removed when the plugin unloads.
---@param method string
---@param callback fun(params: any, res: PoJsonRpcResponse) # `params` is decoded as by `codec.json.decode`, or an empty table when absent or null; the reply may be sent later, and a raise before replying sends `internal_error`
---@param can_invoke? fun(ctx: PoAuthContext): boolean
function jsonrpc.register(method, callback, can_invoke) end

return jsonrpc
