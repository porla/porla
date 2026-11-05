---@meta

---Returned by `porla_timers` and `porla_events`. Dropping it doesn't cancel anything.
---@class PoCancellable
local PoCancellable = {}

---Stops the timer or event handler. Safe to call more than once, from inside its own callback, or after it already finished.
function PoCancellable:cancel() end
