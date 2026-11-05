---@meta porla_sockets

local sockets = {}

---@class PoTcpClientOptions
---@field connect_timeout? number seconds (> 0) for `connect` and `tls`; no timeout by default
---@field read_timeout? number seconds (> 0) for each `read`/`read_exactly`; no timeout by default
---@field nodelay? boolean sets TCP_NODELAY once connected, false by default
---@field keepalive? boolean sets SO_KEEPALIVE once connected, false by default

---Creates an unconnected client. Raises `invalid_params` for an unknown option or a value of the wrong type.
---The socket is closed when the plugin unloads.
---@param options? PoTcpClientOptions
---@return PoTcpClient
function sockets.tcp_client(options) end

return sockets
