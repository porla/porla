---@meta porla_timers

local timers = {}

---Runs `callback` on a cron schedule in the server's local time. Six fields, seconds first:
---`sec min hour dom month dow` (croncpp syntax), e.g. `"*/5 * * * * ?"`. Raises `invalid_params` for an invalid expression
---or when `callback` isn't a function.
---The schedule keeps running until cancelled or the plugin unloads, even if the returned handle is dropped.
---@param expression string
---@param callback fun()
---@return PoCancellable
function timers.cron(expression, callback) end

---@return integer # The current Unix time in milliseconds
function timers.epoch() end

---Runs `callback` every `ms` milliseconds, the first time after `ms`. Raises `invalid_params` unless `ms` is a whole
---number from 10 to 2147483647 (a float like `1.5 * 60 * 1000` is fine), or when `callback` isn't a function.
---@param ms integer
---@param callback fun()
---@return PoCancellable
function timers.interval(ms, callback) end

---Runs `callback` once after `ms` milliseconds. Raises `invalid_params` unless `ms` is a whole number from 10 to
---2147483647, or when `callback` isn't a function.
---@param ms integer
---@param callback fun()
---@return PoCancellable
function timers.timeout(ms, callback) end

return timers
