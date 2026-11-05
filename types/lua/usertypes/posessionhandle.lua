---@meta

---`==` is true for two handles to the same session (also once it's gone);
---`tostring` gives `PoSessionHandle(<id>, "<name>")` or `PoSessionHandle(gone)`.
---Once the session is gone, every field and method except `==` and `tostring` raises a
---`PoError` with `code = "session_not_loaded"`.
---@class PoSessionHandle
---@field id integer
---@field name string
local PoSessionHandle = {}

---@param host string
---@param port integer
function PoSessionHandle:add_dht_node(host, port) end

---Applies and persists the settings. Raises `invalid_params` if a value is out of range, and nothing is applied then.
---`alert_mask`, `peer_fingerprint` and `user_agent` are always overwritten with Porla's own values, also in `settings` itself.
---@param settings LtSettingsPack
function PoSessionHandle:apply_settings(settings) end

---@return LtSettingsPack # A copy; change it and pass it to `apply_settings`
function PoSessionHandle:get_settings() end

---True when the session listens on at least one interface.
---@return boolean
function PoSessionHandle:is_listening() end

---@return boolean
function PoSessionHandle:is_paused() end

function PoSessionHandle:pause() end

function PoSessionHandle:resume() end

---Asks for a `session.dht_stats` event; none comes while the DHT is off.
function PoSessionHandle:post_dht_stats() end

---Asks for a `session.stats` event.
function PoSessionHandle:post_session_stats() end

---@return PoTorrentsHandle
function PoSessionHandle:torrents() end
