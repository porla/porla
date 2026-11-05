---@meta porla_runtime

local runtime = {}

---@class RuntimeArgs
---@field ["http-base-path"] string? nil when no base path is configured
---@field ["state-dir"] string as configured, so it can be relative to the working directory

---@type RuntimeArgs
runtime.args = {}

--- The Lua API version: the major changes when existing plugins can break, the minor when APIs are added.
---@type integer
runtime.api_version_major = 1

---@type integer
runtime.api_version_minor = 0

--- The running Porla version, e.g. `"0.43.0"`.
---@type string
runtime.version = ""

return runtime
