---@meta porla_http_client

local http_client = {}

--- Makes a HTTP request to the provided URL.
---@param url string
---@param callback fun(err: string?, res: PoHttpClientResponse?)
---@param options? {}
function http_client.request(url, callback, options) end

return http_client
