---@meta porla_http_client

local http_client = {}

---@class HttpClientOptions
---@field method? string "GET" by default; uppercased, must be an HTTP token
---@field body? string
---@field headers? table<string, string>
---@field timeout? integer seconds, 30 by default; 0 = none
---@field connect_timeout? integer seconds, 10 by default; 0 = none
---@field follow_redirects? boolean true by default
---@field max_redirects? integer 10 by default; -1 = unlimited

---Only http and https URLs, also when redirecting. Raises `invalid_params` for a bad method, a negative
---timeout or a non-string header key or value. The callback never runs inside the call.
---@param url string
---@param options HttpClientOptions
---@param callback fun(err: PoError?, res: PoHttpClientResponse?) # `err` is an `http_error` (`category = "curl"`), also for a body over 8 MiB; HTTP status codes are not errors
---@overload fun(url: string, callback: fun(err: PoError?, res: PoHttpClientResponse?))
function http_client.request(url, options, callback) end

return http_client
