--[[
The bootstrap process is responsible for getting Porla up and running with a
Lua core. By default, this is fetched from the gh:porla/core repository if no
local release exists. You can customize this behaviour in various ways using
command line arguments.

 * --core-dir=<path to directory>
   If this is set, no fetching will occur but rather the bootstrap process will
   load the core Lua code from this directory, executing `init.lua`.

 * --core-zip=<path to zip archive>
   If this is set, no fetching will occur but rather the bootstrap process will
   load the core Lua code from this zip archive, executing `init.lua`.

 * --core-github-repository=porla/core
   Sets the GitHub repository to get releases from.

 * --core-github-release=latest
   Sets the release tag to fetch. Used in combination with the
   `--core-github-repository` argument to get a specific version.

## The actual boot sequence

 * Checks if either --core-dir or --core-zip is set. If so, load one of
   those and do nothing more.

]]

local function _dir_resolver(path)
    return {
        read = function(file)
            local full_path = tostring(fs.path(path) / file)
            local f = io.open(full_path, "rb")

            if not f then return nil end

            local c = f:read("*a")
            f:close()

            return c
        end
    }
end

local function _zipball_resolver(zipball_file)
    local zip = libzip.open(tostring(zipball_file))
    local entries = zip:list()
    local prefix = ""

    if #entries > 0 and entries[1]:sub(-1) == "/" then
        prefix = entries[1]
    end

    return {
        read = function(file)
            return zip:read(prefix .. file)
        end
    }
end

local function _load_core(resolver)
    table.insert(package.searchers, 2, function(modname)
        local try = {
            modname:gsub("%.", "/") .. ".lua",
            modname:gsub("%.", "/") .. "/init.lua"
        }

        for _, path in ipairs(try) do
            local content = resolver.read(path)
            if content then
                return function()
                    return assert(load(content, "@core/" .. path))()
                end
            end
        end
    end)

    local content = resolver.read("init.lua")

    if not content then
        log.error("Could not find init.lua in core")
        return
    end

    return assert(load(content, "@core/init.lua"))()
end

local M = {
    core = nil
}

function M.load()
    log.info("Loading Porla core bootstrapper")

    local core_resolver = nil

    if args["core-dir"] ~= nil then
        log.info("--core-dir set, loading core from " .. args["core-dir"])
        core_resolver = _dir_resolver(args["core-dir"])
    end

    if args["core-zip"] ~= nil then
        log.info("--core-zip set, loading core from %s", args["core-zip"])

        if core_resolver ~= nil then
            log.warning("Overriding --core-dir with --core-zip")
        end

        core_resolver = _zipball_resolver(fs.path(args["core-zip"]))
    end

    if core_resolver == nil then
        local state_dir = args["state-dir"]
        state_dir = state_dir or tostring(fs.current_path())

        log.info("State dir resolved to " .. state_dir)

        local core_zipball = fs.path(state_dir) / "porla_core.zip"

        log.info("Checking if we have a local core zipball at " .. tostring(core_zipball))

        if fs.exists(core_zipball) and fs.is_regular_file(core_zipball) then
            log.info("Loading Porla core from " .. tostring(core_zipball))
            core_resolver = _zipball_resolver(core_zipball)
        else
            log.info("Fetching latest Porla core zipball from porla/core")

            -- get latest gh release
            local response, err = libcurl.request({
                url = "https://api.github.com/repos/vktr/porla-core/releases/latest",
                headers = {
                    ["User-Agent"] = "porla/1.0"
                }
            })

            if response.status == 404 then
                log.error("Failed to find GitHub release. Double-check repository and release version")
                return
            end

            if err ~= nil then
                log.error("Error occured when trying to fetch latest release from GitHub: %s", tostring(err))
                return
            end

            if response.status > 200 then
                log.error("Unexpected HTTP status code %d when fetching latest release from GitHub", response.status_code)
                return
            end

            local release, err = json.parse(response.body)

            if err then
                log.error("Failed to deserialize response body: " .. err)
                return
            end

            local zipball, err = libcurl.request({
                url = release.zipball_url,
                headers = {
                    ["User-Agent"] = "porla/1.0"
                }
            })

            log.info("Saving core zipball to " .. tostring(core_zipball))

            local core_zip = io.open(tostring(core_zipball), "wb")
            core_zip:write(zipball.body)
            core_zip:close()
            core_zip = nil

            core_resolver = _zipball_resolver(core_zipball)
        end
    end

    M.core = _load_core(core_resolver)
    M.core.load()
end

function M.unload()
    log.info("Unloading bootstrapper")

    if M.core ~= nil then
        M.core.unload()
    end
end

return M
