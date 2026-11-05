---@meta

---The announce state for one info hash version; fields are readonly.
---@class LtAnnounceInfohash
---@field complete_sent boolean
---@field fails integer # consecutive failures
---@field message string # the tracker's last message
---@field min_announce integer # unix time in seconds; -2147483648 when unset
---@field next_announce integer # unix time in seconds; -2147483648 when unset
---@field scrape_complete integer # -1 when unknown
---@field scrape_downloaded integer # -1 when unknown
---@field scrape_incomplete integer # -1 when unknown
---@field start_sent boolean
---@field updating boolean # an announce is in progress
local LtAnnounceInfohash = {}
