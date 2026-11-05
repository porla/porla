---@meta

---A parsed PQL filter. Has no constructor; create one with `PoQuery.parse`.
---@class PoQuery
PoQuery = {}

---@param pql string
---@return PoQuery?
---@return PoError? err # `invalid_params` when the query doesn't parse
function PoQuery.parse(pql) end

---@param torrent LtTorrentStatus
---@return boolean
function PoQuery:includes(torrent) end
