---@meta porla_kv

local kv = {}

---The store is global: Porla, every plugin and the JSON-RPC API share the same keys.
---@param key string
---@return any # The stored value decoded from JSON, or nil if the key is missing or holds `null`. Nested `null`s are `porla.null`.
function kv.get(key) end

---Stores `value` as JSON; nil stores `null`, which reads back as nil. Read-only keys are silently left unchanged.
---Raises `invalid_params` for values JSON can't hold (functions, userdata, sparse arrays, NaN/inf, non-string keys).
---@param key string
---@param value any
function kv.set(key, value) end

return kv
