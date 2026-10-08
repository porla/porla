#pragma once

#include <functional>
#include <optional>
#include <string>

#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <sqlite3.h>

namespace porla
{
    class Events;
    class Presets;
    class Sessions;
    struct TorrentClientData;

    struct TorrentsOptions
    {
        sqlite3*  db;
        Events&   events;
        Presets&  presets;
        Sessions& sessions;
    };

    struct TorrentsAddOptions
    {
        std::optional<int>         session_id;
        std::optional<int>         preset_id;
        std::optional<std::string> preset;
    };

    struct TorrentsAddResult
    {
        enum class Error
        {
            None,
            SessionNotFound,
            SessionNotLoaded,
            AlreadyInSession,
            MissingInfoHash,
            MissingSavePath,
            InvalidData,
            Failed
        };

        Error           error      = Error::None;
        std::string     what;
        int             session_id = -1;
        lt::info_hash_t info_hash;

        explicit operator bool() const { return error == Error::None; }
    };

    class Torrents
    {
    public:
        explicit Torrents(const TorrentsOptions& options);

        Torrents(const Torrents&)            = delete;
        Torrents& operator=(const Torrents&) = delete;

        TorrentsAddResult Prepare(lt::add_torrent_params& params, const TorrentsAddOptions& options);

        TorrentsAddResult Add(lt::add_torrent_params params);

        bool UpdateClientData(const lt::torrent_handle& th, const std::function<void(TorrentClientData&)>& change);

    private:
        TorrentsOptions m_options;
    };
}
