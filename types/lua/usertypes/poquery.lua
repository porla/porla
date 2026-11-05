---@meta

---@class PoQuery
PoQuery = {}

---@param pql string # The PQL query to parse
---@return PoQuery?
---@return PoError? err # `invalid_params` when the query doesn't parse
function PoQuery.parse(pql) end

---@param torrent LtTorrentStatus # The torrent status to check
---@return boolean
function PoQuery:includes(torrent) end
