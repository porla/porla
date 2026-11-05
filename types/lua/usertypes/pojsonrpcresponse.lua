---@meta

---Only the first `ok` or `error` is sent; later calls, and calls after the client disconnected, are ignored.
---@class PoJsonRpcResponse
local PoJsonRpcResponse = {}

---Raises `invalid_params` if `data` can't be encoded as by `codec.json.encode`.
---@param code integer # Sent as-is
---@param message string
---@param data? any # Sent as the error's `data`, `null` when omitted
---@overload fun(self: PoJsonRpcResponse, err: PoError) # Replies with the registry code for `err.code` (`internal_error` if unknown) and the registry message if `err.message` is empty; `data` carries `reason`, `category` and `value`
function PoJsonRpcResponse:error(code, message, data) end

---Raises `invalid_params` if `data` can't be encoded as by `codec.json.encode`.
---@param data? any # The data (if any) to return to the client, `null` when omitted
function PoJsonRpcResponse:ok(data) end
