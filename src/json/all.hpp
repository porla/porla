#pragma once

#include <libtorrent/torrent_flags.hpp>
#include <nlohmann/json.hpp>

namespace boost::system
{
    class error_code;

    inline void from_json(const nlohmann::json& json, error_code& ec) {}
    void to_json(nlohmann::json& json, const error_code& ec);
}

namespace libtorrent
{
    struct announce_endpoint;
    struct announce_entry;
    struct announce_infohash;
    struct info_hash_t;
    struct peer_info;
    struct settings_pack;
    struct torrent_status;

    inline void from_json(const nlohmann::json& json, announce_endpoint& endpoint) {}
    inline void from_json(const nlohmann::json& json, announce_entry& entry) {}
    inline void from_json(const nlohmann::json& json, announce_infohash& infohash) {}
    void from_json(const nlohmann::json& json, info_hash_t& infohash);
    inline void from_json(const nlohmann::json& json, peer_info& pi) {}
    void from_json(const nlohmann::json& json, settings_pack& settings);
    void from_json(const nlohmann::json& json, torrent_flags_t& flags);
    inline void from_json(const nlohmann::json& json, torrent_status& ts) {}

    void to_json(nlohmann::json& json, const announce_endpoint& endpoint);
    void to_json(nlohmann::json& json, const announce_entry& entry);
    void to_json(nlohmann::json& json, const announce_infohash& infohash);
    void to_json(nlohmann::json& json, const torrent_flags_t& flags);
    void to_json(nlohmann::json& json, const info_hash_t& infohash);
    void to_json(nlohmann::json& json, const peer_info& pi);
    void to_json(nlohmann::json& json, const settings_pack& settings);
    void to_json(nlohmann::json& json, const torrent_status& ts);
}

namespace porla
{
    class TorrentClientData;
    void to_json(nlohmann::json& json, const TorrentClientData& tcd);
}

namespace porla::Methods
{
    struct FsSpaceReq;
    struct FsSpaceRes;
    struct KeyValueGetReq;
    struct KeyValueGetRes;
    struct KeyValueSetReq;
    struct KeyValueSetRes;
    struct PluginsAddReq;
    struct PluginsAddRes;
    struct PluginsGetReq;
    struct PluginsGetRes;
    struct PluginsListReq;
    struct PluginsListRes;
    struct PluginsReloadReq;
    struct PluginsReloadRes;
    struct PluginsRemoveReq;
    struct PluginsRemoveRes;
    struct PluginsUpdateReq;
    struct PluginsUpdateRes;

    struct SessionsListReq;
    struct SessionsListRes;
    struct SessionsPauseReq;
    struct SessionsPauseRes;
    struct SessionsResumeReq;
    struct SessionsResumeRes;

    struct TorrentsAddReq;
    struct TorrentsAddRes;
    struct TorrentsFilesListReq;
    struct TorrentsFilesListRes;
    struct TorrentsFilesProgressReq;
    struct TorrentsFilesProgressRes;
    struct TorrentsGetReq;
    struct TorrentsGetRes;
    struct TorrentsListReq;
    struct TorrentsListRes;
    struct TorrentsMoveReq;
    struct TorrentsMoveRes;
    struct TorrentsPauseReq;
    struct TorrentsPauseRes;
    struct TorrentsPeersAddReq;
    struct TorrentsPeersAddRes;
    struct TorrentsPeersListReq;
    struct TorrentsPeersListRes;
    struct TorrentsPiecesGetReq;
    struct TorrentsPiecesGetRes;
    struct TorrentsPropertiesGetReq;
    struct TorrentsPropertiesGetRes;
    struct TorrentsPropertiesSetReq;
    struct TorrentsRecheckReq;
    struct TorrentsRecheckRes;
    struct TorrentsRemoveReq;
    struct TorrentsRemoveRes;
    struct TorrentsResumeReq;
    struct TorrentsResumeRes;
    struct TorrentsTrackersListReq;
    struct TorrentsTrackersListRes;

    namespace Auth
    {
        struct AuthInitReq;
        struct AuthInitRes;
        struct AuthLoginReq;
        struct AuthLoginRes;

        void from_json(const nlohmann::json& json, AuthInitReq& req);
        void from_json(const nlohmann::json& json, AuthLoginReq& req);

        void to_json(nlohmann::json& json, const AuthInitRes& res);
        void to_json(nlohmann::json& json, const AuthLoginRes& res);
    }

    namespace Mmdb
    {
        struct MmdbLookupReq;
        struct MmdbLookupRes;

        void from_json(const nlohmann::json& json, MmdbLookupReq& req);
        void to_json(nlohmann::json& json, const MmdbLookupRes& res);
    }

    namespace Plugins
    {
        struct PluginsInstallReq;
        struct PluginsInstallRes;

        void from_json(const nlohmann::json& json, PluginsInstallReq& req);
        void to_json(nlohmann::json& json, const PluginsInstallRes& res);
    }

    namespace Presets
    {
        struct PresetsAddReq;
        struct PresetsAddRes;
        struct PresetsGetReq;
        struct PresetsGetRes;
        struct PresetsListReq;
        struct PresetsListRes;
        struct PresetsRemoveReq;
        struct PresetsRemoveRes;
        struct PresetsUpdateReq;
        struct PresetsUpdateRes;

        void from_json(const nlohmann::json& json, PresetsAddReq& req);
        void from_json(const nlohmann::json& json, PresetsGetReq& req);
        void from_json(const nlohmann::json& json, PresetsListReq& req);
        void from_json(const nlohmann::json& json, PresetsRemoveReq& req);
        void from_json(const nlohmann::json& json, PresetsUpdateReq& req);

        void to_json(nlohmann::json& json, const PresetsAddRes& res);
        void to_json(nlohmann::json& json, const PresetsGetRes& res);
        void to_json(nlohmann::json& json, const PresetsListRes& res);
        void to_json(nlohmann::json& json, const PresetsRemoveRes& res);
        void to_json(nlohmann::json& json, const PresetsUpdateRes& res);
    }

    namespace Sessions
    {
        struct SessionsAddReq;
        struct SessionsAddRes;
        struct SessionsGetReq;
        struct SessionsGetRes;
        struct SessionsRemoveReq;
        struct SessionsRemoveRes;
        struct SessionsSettingsGetReq;
        struct SessionsSettingsGetRes;
        struct SessionsSettingsSetReq;
        struct SessionsSettingsSetRes;
        struct SessionsUpdateReq;
        struct SessionsUpdateRes;

        void from_json(const nlohmann::json& json, SessionsAddReq& req);
        void from_json(const nlohmann::json& json, SessionsGetReq& req);
        void from_json(const nlohmann::json& json, SessionsRemoveReq& req);
        void from_json(const nlohmann::json& json, SessionsSettingsGetReq& req);
        void from_json(const nlohmann::json& json, SessionsSettingsSetReq& req);
        void from_json(const nlohmann::json& json, SessionsUpdateReq& req);

        void to_json(nlohmann::json& json, const SessionsAddRes& res);
        void to_json(nlohmann::json& json, const SessionsGetRes& res);
        void to_json(nlohmann::json& json, const SessionsRemoveRes& res);
        void to_json(nlohmann::json& json, const SessionsSettingsGetRes& res);
        void to_json(nlohmann::json& json, const SessionsSettingsSetRes& res);
        void to_json(nlohmann::json& json, const SessionsUpdateRes& res);
    }

    namespace Torrents
    {
        struct TorrentsCountReq;
        struct TorrentsCountRes;
        struct TorrentsOverviewReq;
        struct TorrentsOverviewRes;

        void from_json(const nlohmann::json& json, TorrentsCountReq& req);
        void from_json(const nlohmann::json& json, TorrentsOverviewReq& req);
        void to_json(nlohmann::json& json, const TorrentsCountRes& res);
        void to_json(nlohmann::json& json, const TorrentsOverviewRes& res);
    }

    namespace WebUI
    {
        struct WebUIInstallReq;
        struct WebUIInstallRes;

        void from_json(const nlohmann::json& json, WebUIInstallReq& req);
        void to_json(nlohmann::json& json, const WebUIInstallRes& res);
    }

    void from_json(const nlohmann::json& json, FsSpaceReq& req);
    void from_json(const nlohmann::json& json, KeyValueGetReq& req);
    void from_json(const nlohmann::json& json, KeyValueSetReq& req);
    void from_json(const nlohmann::json& json, PluginsAddReq& req);
    void from_json(const nlohmann::json& json, PluginsGetReq& req);
    void from_json(const nlohmann::json& json, PluginsListReq& req);
    void from_json(const nlohmann::json& json, PluginsReloadReq& req);
    void from_json(const nlohmann::json& json, PluginsRemoveReq& req);
    void from_json(const nlohmann::json& json, PluginsUpdateReq& req);
    void from_json(const nlohmann::json& json, SessionsListReq& req);
    void from_json(const nlohmann::json& json, SessionsPauseReq& req);
    void from_json(const nlohmann::json& json, SessionsResumeReq& req);
    void from_json(const nlohmann::json& json, TorrentsAddReq& req);
    void from_json(const nlohmann::json& json, TorrentsFilesListReq& req);
    void from_json(const nlohmann::json& json, TorrentsFilesProgressReq& req);
    void from_json(const nlohmann::json& json, TorrentsGetReq& req);
    void from_json(const nlohmann::json& json, TorrentsListReq& req);
    void from_json(const nlohmann::json& json, TorrentsMoveReq& req);
    void from_json(const nlohmann::json& json, TorrentsPauseReq& req);
    void from_json(const nlohmann::json& json, TorrentsPeersAddReq& req);
    void from_json(const nlohmann::json& json, TorrentsPeersListReq& req);
    void from_json(const nlohmann::json& json, TorrentsPiecesGetReq& req);
    void from_json(const nlohmann::json& json, TorrentsPropertiesGetReq& req);
    void from_json(const nlohmann::json& json, TorrentsPropertiesSetReq& req);
    void from_json(const nlohmann::json& json, TorrentsRecheckReq& req);
    void from_json(const nlohmann::json& json, TorrentsResumeReq& req);
    void from_json(const nlohmann::json& json, TorrentsRemoveReq& req);
    void from_json(const nlohmann::json& json, TorrentsTrackersListReq& req);

    void to_json(nlohmann::json& json, const FsSpaceRes& res);
    void to_json(nlohmann::json& json, const KeyValueGetRes& res);
    void to_json(nlohmann::json& json, const KeyValueSetRes& res);
    void to_json(nlohmann::json& json, const PluginsAddRes& res);
    void to_json(nlohmann::json& json, const PluginsGetRes& res);
    void to_json(nlohmann::json& json, const PluginsListRes& res);
    void to_json(nlohmann::json& json, const PluginsReloadRes& res);
    void to_json(nlohmann::json& json, const PluginsRemoveRes& res);
    void to_json(nlohmann::json& json, const PluginsUpdateRes& res);
    void to_json(nlohmann::json& json, const SessionsListRes& res);
    void to_json(nlohmann::json& json, const SessionsPauseRes& res);
    void to_json(nlohmann::json& json, const SessionsResumeRes& res);
    void to_json(nlohmann::json& json, const TorrentsAddRes& res);
    void to_json(nlohmann::json& json, const TorrentsFilesListRes& res);
    void to_json(nlohmann::json& json, const TorrentsFilesProgressRes& res);
    void to_json(nlohmann::json& json, const TorrentsGetRes& res);
    void to_json(nlohmann::json& json, const TorrentsListRes& res);
    void to_json(nlohmann::json& json, const TorrentsMoveRes& res);
    void to_json(nlohmann::json& json, const TorrentsPauseRes& res);
    void to_json(nlohmann::json& json, const TorrentsPeersAddRes& res);
    void to_json(nlohmann::json& json, const TorrentsPeersListRes& res);
    void to_json(nlohmann::json& json, const TorrentsPiecesGetRes& res);
    void to_json(nlohmann::json& json, const TorrentsPropertiesGetRes& res);
    void to_json(nlohmann::json& json, const TorrentsRecheckRes& res);
    void to_json(nlohmann::json& json, const TorrentsRemoveRes& res);
    void to_json(nlohmann::json& json, const TorrentsResumeRes& res);
    void to_json(nlohmann::json& json, const TorrentsTrackersListRes& res);
}
