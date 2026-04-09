local sqlite3             = require "sqlite3"

local _0001_initial_setup = require "db.migrations.0001_initial_setup"
local _0002_add_session_settings = require "db.migrations.0002_add_session_settings"
local _0003_users = require "db.migrations.0003_users"
local _0004_remove_session_params = require "db.migrations.0004_remove_session_params"
local _0005_add_torrents_metadata = require "db.migrations.0005_add_torrents_metadata"
local _0006_add_client_data = require "db.migrations.0006_add_client_data"
local _0007_remove_session_settings = require "db.migrations.0007_remove_session_settings"
local _0008_plugins = require "db.migrations.0008_plugins"
local _0009_multi_sessions = require "db.migrations.0009_multi_sessions"
local _0010_sessions = require "db.migrations.0010_sessions"

local M = {}

local function _get_user_version(db)
    local v = 0
    local stmt = db:prepare("PRAGMA user_version")

    if (stmt:step() == sqlite3.ROW) then
        v = stmt:get_value(0)
    end

    stmt:finalize()

    return v
end

local function _set_user_version(db, version)
    db:exec("PRAGMA user_version = " .. version .. ";")
end

local migrations = {
    _0001_initial_setup,
    _0002_add_session_settings,
    _0003_users,
    _0004_remove_session_params,
    _0005_add_torrents_metadata,
    _0006_add_client_data,
    _0007_remove_session_settings,
    _0008_plugins,
    _0009_multi_sessions,
    _0010_sessions
}

function M.migrate(db)
    local user_version = _get_user_version(db)

    db:exec("BEGIN TRANSACTION;")

    if #migrations > user_version then
        for i = user_version + 1, #migrations do
            migrations[i].migrate(db)
        end
    end

    _set_user_version(db, #migrations)

    db:exec("COMMIT;")
end

return M
