--[[
The bootstrap process is responsible for getting Porla up and running with a
Lua core. By default, this is fetched from the gh:porla/core repository if no
local release exists. You can customize this behaviour in various ways using
command line arguments.

 * --core-path=<path to directory>
   If this is set, no fetching will occur but rather the bootstrap process will
   load the core Lua code from this directory, executing `init.lua`.

 * --core-archive=<path to zip archive>
   If this is set, no fetching will occur but rather the bootstrap process will
   load the core Lua code from this zip archive, executing `init.lua`.

 * --core-github-repository=porla/core
   Sets the GitHub repository to get releases from.

 * --core-version=latest
   Sets the release tag to fetch. Used in combination with the
   `--core-github-repository` argument to get a specific version.

## The actual boot sequence

 * Checks if either --core-path or --core-archive is set. If so, load one of
   those and do nothing more.

]]

function load()
    if args["core-path"] ~= nil then
        load_core(args["core-path"])
        return
    end

    if args["core-archive"] ~= nil then
        load_core(args["core-archive"])
        return
    end

    local core_state_path = ".porla/core/latest.zip"

    if fs.exists(core_state_path) then
        load_core(core_state_path)
        return
    end

    print("fetching latest github release")

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

    print("writing core.zip")

    local core_zip = io.open("core.zip", "wb")
    core_zip:write(zipball.body)
    core_zip:close()
    core_zip = nil

    print("loading core.zip")

    local archive, err = zip.zip_t.open("core.zip", zip.ZIP_RDONLY)

    for i = 0, archive:get_num_entries(zip.ZIP_FL_UNCHANGED) - 1 do
        local stat, err = archive:stat_index(i, zip.ZIP_FL_UNCHANGED)
        print(stat.name)
    end
end

function unload()
    log.info("Unloading bootstrapper")
end
