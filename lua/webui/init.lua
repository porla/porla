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

---@param html string
---@return string
local function rewrite_index(html)
    local token_value   = base_path ~= "" and base_path or "/"
    local escaped_token = token_value:gsub("%%", "%%%%")
    local escaped_base  = base_path:gsub("%%", "%%%%")

    html                = html:gsub("%%BASE_PATH%%", escaped_token)

    if base_path ~= "" then
        html = html:gsub('src="/([^/])', 'src="' .. escaped_base .. "/%1")
        html = html:gsub('href="/([^/])', 'href="' .. escaped_base .. "/%1")
    end

    return html
end

---@param files table<string, string>
local function set_entries(files)
    local index = files["index.html"]

    if index then
        files["index.html"] = rewrite_index(index)
    end

    entries = files

    print("Updated web UI files")
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

---@param callback function
local function handle_asset_request(callback, owner, repository, tag_name)
    ---@param err string?
    ---@param res PoHttpClientResponse?
    return function(err, res)
        if err then
            return callback(err)
        end

        if not res then
            return callback("no response returned")
        end

        if res.status ~= 200 then
            return callback("invalid http status")
        end

        local webui_dir = string.format("%s/webui", runtime.args["state-dir"])

        local _, create_err = fs.create_directories(webui_dir)

        if create_err then
            return callback(create_err)
        end

        local webui_file_path = string.format(
            "%s/%s_%s_%s.zip",
            webui_dir,
            owner,
            repository,
            tag_name);

        local output, output_err = io.open(webui_file_path, "wb")

        if not output then
            return callback(output_err)
        end

        output:write(res.body)
        output:close()
        output = nil

        callback(nil, webui_file_path)
    end
end

local function handle_release_request(callback, owner, repository)
    ---@param err string?
    ---@param res PoHttpClientResponse?
    return function(err, res)
        if err then
            return callback(err)
        end

        if not res then
            return callback("no response returned")
        end

        if res.status ~= 200 then
            return callback("invalid http status")
        end

        local body, err = codec.json.decode(res.body)

        if err then
            return callback(err)
        end

        if type(body.assets) ~= "table" or #body.assets == 0 then
            return callback("invalid release assets")
        end

        local asset_url = body.assets[1].browser_download_url

        if not asset_url then
            return callback("no asset url")
        end

        http_client.request(asset_url, handle_asset_request(callback, owner, repository, body.tag_name), http_client_opts)
    end
end

local webui = {}

---@param version string # The version to install
function webui.install(version, callback)
    local owner = kv.get("porla.webui.owner")
    local repository = kv.get("porla.webui.repository")

    owner = owner or "porla"
    repository = repository or "web"

    local url = string.format("https://api.github.com/repos/%s/%s/releases/%s", owner, repository, version)

    print("Installing Web UI from {}", url)

    http_client.request(url, handle_release_request(callback, owner, repository), http_client_opts)
end

---@param path string # The path to a zip archive
function webui.load_archive(path)
    local archive = io.open(path, "rb")

    if not archive then
        print("Failed to open archive file: {}", path)
        return false
    end

    local buffer = archive:read("a")
    archive:close()
    archive = nil

    local files, err = zip.read(buffer)

    if err then
        print("Failed to read archive: {}", err)
        return false
    end

    if not files then
        print("No error but files was missing")
        return false
    end

    set_entries(files)

    return true
end

function webui.load()
    local current_webui = kv.get("porla.webui.current")

    if type(current_webui) ~= "string" or not fs.exists(current_webui) then
        webui.install("latest", function(err, file)
            if err then
                return print("Failed to install web UI: {}", err)
            end

            if not webui.load_archive(file) then
                return print("Failed to load web UI archive")
            end

            webui.set_current(file)
        end)
    else
        webui.load_archive(current_webui)
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

function webui.set_current(path)
    kv.set("porla.webui.current", path)
end

return webui
