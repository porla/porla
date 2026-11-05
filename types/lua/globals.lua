---@meta

porla = {}

---JSON `null`, the same value as `codec.json.null`. Encodes as `null` where a table can't hold nil
---(e.g. to clear a key), and is what a `null` inside a decoded array or object becomes.
---@type PoJsonNull
porla.null = nil

---Logs the arguments at info level, tab-separated and converted with `tostring`, prefixed with the plugin id.
---@param ... any
function print(...) end

---The table a plugin's entry point returns; returning anything else fails the load.
---@class PoPlugin
---@field init? fun(config: table?) Called once after the entry point ran, with the table the plugin's config chunk returns (nil when there is none). Raising fails the load.
---@field destroy? fun() Called when the plugin unloads (reload, update, removal or shutdown). Errors are logged and ignored.
