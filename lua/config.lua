local M = {}

M._instance = nil

local function load()
    local cfg = {
        config_file = nil,
        db_file     = nil,
        mmdb_file   = nil,

        http_auth_enabled     = true,
        http_base_path        = nil,
        http_host             = "127.0.0.1",
        http_metrics_enabled  = true,
        http_port             = 1337,
        http_webui_enabled    = true,
        http_webui_file       = "webui.zip",
        http_webui_repository = "porla/web",

        sessions = {
            default = lt.default_settings()
        },
        state_dir = nil
    }

    local config_file_search_paths = {
        fs.current_path() / "porla.toml",
        os.getenv("XDG_CONFIG_HOME") and (fs.path(os.getenv("XDG_CONFIG_HOME")) / "porla" / "porla.toml") or fs.path(),
        os.getenv("HOME")            and (fs.path(os.getenv("HOME")) / ".config" / "porla" / "porla.toml") or fs.path(),
        os.getenv("HOME")            and (fs.path(os.getenv("HOME")) / ".config" / "porla.toml") or fs.path(),
        fs.path("/etc/porla/porla.toml"),
        fs.path("/etc/porla.toml")
    }

    local config_file_path = nil

    for _, path in ipairs(config_file_search_paths) do
        if fs.exists(path) and fs.is_regular_file(path) then
            cfg.config_file = path
            break
        end
    end

    -- set values from env
    cfg.config_file = os.getenv("PORLA_CONFIG_FILE") or cfg.config_file
    cfg.db_file     = os.getenv("PORLA_DB")          or cfg.db_file
    cfg.mmdb_file   = os.getenv("PORLA_MMDB_FILE")   or cfg.mmdb_file

    -- set values from cmd args
    -- TODO

    if cfg.config_file then
        print("Loading config from", cfg.config_file)

        local f = io.open(tostring(cfg.config_file), "rb")

        if f then
            local content = f:read("*a")
            f:close()

            local toml_config = toml.parse(content)

            local default_settings_base = toml_config.session_settings and toml_config.session_settings.base

            if base == "default" then
                cfg.sessions["default"] = lt.default_settings()
            elseif base == "high_performance_seed" then
                cfg.sessions["default"] = lt.high_performance_seed()
            elseif base == "min_memory_usage" then
                cfg.sessions["default"] = lt.min_memory_usage()
            end
        end
    end

    return {
        mmdb_file = nil,
        sessions  = {
            default = lt.default_settings()
        },
        state_dir = "/workspaces/porla/build"
    }
end

function M.get()
    if (M._instance == nil) then
        local ok, result = pcall(load)
        if not ok then
            print("load error: " .. tostring(result))
            return nil
        end
        M._instance = result
    end

    return M._instance
end

return M
