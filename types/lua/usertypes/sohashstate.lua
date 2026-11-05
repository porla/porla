---@meta

---@class SoHashState
local SoHashState = {}

---Raises `invalid_state` when already finalized.
---@return string hash # 32 raw bytes
function SoHashState:final() end

---Raises `invalid_state` after `final`.
---@param data string
function SoHashState:update(data) end
