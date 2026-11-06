---@meta porla_log

local log = {}

---Logs the arguments at debug level, tab-separated and converted with `tostring`, tagged with the plugin id.
---@param ... any
function log.debug(...) end

---Logs the arguments at error level, tab-separated and converted with `tostring`, tagged with the plugin id.
---@param ... any
function log.error(...) end

---Logs the arguments at info level, tab-separated and converted with `tostring`, tagged with the plugin id. The same as `print`.
---@param ... any
function log.info(...) end

---Logs the arguments at warning level, tab-separated and converted with `tostring`, tagged with the plugin id.
---@param ... any
function log.warning(...) end

return log
