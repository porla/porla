local codec       = require("porla_codec")
local fs          = require("porla_filesystem")
local http_client = require("porla_http_client")
local http_server = require("porla_http_server")
local runtime     = require("porla_runtime")
local kv          = require("porla_kv")
local zip         = require("porla_zip")

local base_path   = runtime.args["http-base-path"] or "/"

if base_path == "" then base_path = "/" end
if base_path:sub(1, 1) ~= "/" then base_path = "/" .. base_path end
if base_path:sub(-1) == "/" then base_path = base_path:sub(1, -2) end

---@type table<string, string>
local entries          = {}

---@type table<string, string>
local mime_types       = {
    [".css"] = "text/css",
    [".html"] = "text/html",
    [".js"] = "text/javascript",
    [".json"] = "application/json",
    [".svg"] = "image/svg+xml",
}

local http_client_opts = {
    headers = {
        ["User-Agent"] = string.format("porla/%s", runtime.version)
    }
}

---@param files table<string, string>
local function set_entries(files)
    local index = files["index.html"]

    if index then
        files["index.html"] = index:gsub("%%BASE_PATH%%", (base_path:gsub("%%", "%%%%")))
    end

    entries = files
end

---@param path string
local function resolve_request_path(path)
    if base_path ~= "" then
        if path:sub(1, #base_path) ~= base_path then
            return nil
        end

        path = path:sub(#base_path + 1)
    end

    local name = path:match("^/*(.*)$")

    if name == "" then
        return "index.html"
    end

    return name
end

---@param res PoHttpClientResponse
local function handle_asset_request(res)
    if res.status ~= 200 then
        print("Failed to fetch release asset - got status {}", res.status)
        return
    end

    local files, err = zip.read(res.body)

    if err then
        print("Failed to read release asset: {}", err)
        return
    end

    if not files then
        print("No error but files was missing")
        return
    end

    -- todo: store webui zip file

    set_entries(files)
end

---@param res PoHttpClientResponse
local function handle_release_request(res)
    if res.status ~= 200 then
        print("Failed to fetch release data - got status {}", res.status)
        return
    end

    local body, err = codec.json.decode(res.body)

    if err then
        print("Failed to JSON decode release: {}", err)
        return
    end

    if type(body.assets) ~= "table" or #body.assets == 0 then
        print("Invalid assets in release: {}", res.body)
        return
    end

    local asset_url = body.assets[1].browser_download_url

    if not asset_url then
        print("Failed to find asset URL: {}", res.body)
        return
    end

    print("Found version {} of web UI - fetching from {}", body.tag_name, asset_url)

    http_client.request(asset_url, handle_asset_request, http_client_opts)
end

---@param version string
local function install(version)
    local owner = kv.get("porla.webui.owner")
    local repository = kv.get("porla.webui.repository")

    owner = owner or "porla"
    repository = repository or "web"

    local url = string.format("https://api.github.com/repos/%s/%s/releases/%s", owner, repository, version)

    print("Installing Web UI from {}", url)

    http_client.request(url, handle_release_request, http_client_opts)
end

return function()
    local current_webui = kv.get("porla.webui.current")

    if type(current_webui) ~= "string" or not fs.exists(current_webui) then
        install("latest")
    else
        local file = io.open(current_webui, "rb")

        if not file then
            print("Failed to read web UI zip file")
        else
            local buffer = file:read("a")
            local files, err = zip.read(buffer)

            if err then
                print("Failed to web UI file: {}", err)
                return
            end

            if not files then
                print("No error but files was missing")
                return
            end

            set_entries(files)
        end
    end

    http_server.get(base_path .. "/*", function(req, res)
        local requested_path = resolve_request_path(req.path)

        if not requested_path then
            res:writeStatus("404 Not Found")
            res:finish("Not found")
            return
        end

        if entries[requested_path] == nil then
            requested_path = "index.html"
        end

        local content = entries[requested_path]

        if not content then
            res:writeStatus("404 Not Found")
            res:finish("Not found")
            return
        end

        local extension = requested_path:match("%.[^./]+$")
        local mime_type = mime_types[extension] or "application/octet-stream"

        res:writeStatus("200 OK")
        res:writeHeader("Content-Type", mime_type)
        res:finish(content)
    end)
end
