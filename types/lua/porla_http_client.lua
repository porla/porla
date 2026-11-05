---@meta porla_http_client

local http_client = {}

---@class HttpClientOptions
---@field method? string "GET" by default
---@field body? string
---@field headers? table<string, string>
---@field timeout? integer seconds, 30 by default; 0 = none
---@field connect_timeout? integer seconds, 10 by default; 0 = none
---@field follow_redirects? boolean true by default
---@field max_redirects? integer 10 by default

--- Makes a HTTP request to the provided URL. The callback comes last.
---@param url string
---@param options HttpClientOptions
---@param callback fun(err: PoError?, res: PoHttpClientResponse?) # `err` is an `http_error` (`category = "curl"`); HTTP status codes are not errors
---@overload fun(url: string, callback: fun(err: PoError?, res: PoHttpClientResponse?))
function http_client.request(url, options, callback) end

return http_client
