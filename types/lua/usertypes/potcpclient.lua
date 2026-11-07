---@meta

---Callbacks run after the call returns. Errors passed to them are `closed`
---when the peer closed the connection, and `io_error` otherwise (`message = "timed out"` when a timeout
---closed the client).
---@class PoTcpClient
local PoTcpClient = {}

---Closes the socket; pending operations complete with an `io_error`. Safe to call more than once.
function PoTcpClient:close() end

---Resolves `host` and connects to the first address that accepts.
---@param host string
---@param port integer
---@param callback fun(err: PoError?)
function PoTcpClient:connect(host, port, callback) end

---Reads whatever is available, at most 8192 bytes. Raises `invalid_state` while another read is pending.
---@param callback fun(err: PoError?, data: string?)
function PoTcpClient:read(callback) end

---Reads exactly `n` bytes. Raises `invalid_state` while another read is pending.
---@param n integer
---@param callback fun(err: PoError?, data: string?) # on error, `data` holds the bytes read so far, if any
function PoTcpClient:read_exactly(n, callback) end

---@class PoTcpClientTlsOptions
---@field verify? boolean verify the certificate chain and host name, true by default
---@field ca_file? string PEM file to verify against instead of the system store
---@field cert_file? string PEM client certificate chain
---@field key_file? string PEM private key; needs `cert_file`, defaults to it
---@field server_name? string SNI and verified host name, the `connect` host by default

---Upgrades a connected client to TLS (1.2 or later); reads and writes are encrypted afterwards.
---Raises `invalid_params` for an unknown option, a value of the wrong type or a missing callback. Setup
---failures go to the callback like any other error: `invalid_state` with neither a `connect` host nor `server_name`,
---`invalid_params` for `key_file` without `cert_file`, `io_error` for an unreadable file.
---@param options PoTcpClientTlsOptions
---@param callback fun(err: PoError?)
---@overload fun(self: PoTcpClient, callback: fun(err: PoError?))
function PoTcpClient:tls(options, callback) end

---Queues `data`; writes go out in call order.
---@param data string
---@param callback? fun(err: PoError?) # runs once `data` is written or failed
function PoTcpClient:write(data, callback) end
