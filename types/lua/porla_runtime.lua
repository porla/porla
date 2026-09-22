---@meta porla_runtime

local runtime = {}

---@class RuntimeArgs
---@field ["http-base-path"] string
---@field ["state-dir"] string

---@type RuntimeArgs
runtime.args = {}

--- The running Porla version, e.g. `"0.43.0"`.
---@type string
runtime.version = ""

return runtime
