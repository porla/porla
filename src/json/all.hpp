#pragma once

#include <nlohmann/json.hpp>

#include "lttypes.hpp"

namespace boost::system
{
    class error_code;

    inline void from_json(const nlohmann::json& json, error_code& ec) {}
    void to_json(nlohmann::json& json, const error_code& ec);
}

namespace porla
{
    class TorrentClientData;
    void to_json(nlohmann::json& json, const TorrentClientData& tcd);
}

namespace porla::Rpc::Methods
{
    namespace Torrents
    {
        struct TorrentsAddReq;
        struct TorrentsAddRes;
        struct TorrentsCountReq;
        struct TorrentsCountRes;
        struct TorrentsFilesListReq;
        struct TorrentsFilesListRes;
        struct TorrentsFilesPrioritiesReq;
        struct TorrentsFilesPrioritiesRes;
        struct TorrentsFilesPrioritizeReq;
        struct TorrentsFilesPrioritizeRes;
        struct TorrentsFilesProgressReq;
        struct TorrentsFilesProgressRes;
        struct TorrentsFilesRenameReq;
        struct TorrentsFilesRenameRes;
        struct TorrentsGetReq;
        struct TorrentsGetRes;
        struct TorrentsListReq;
        struct TorrentsListRes;
        struct TorrentsMigrateReq;
        struct TorrentsMigrateRes;
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
        struct TorrentsQueueAnyReq;
        struct TorrentsQueueAnyRes;
        struct TorrentsQueueSetReq;
        struct TorrentsQueueSetRes;
        struct TorrentsRecheckReq;
        struct TorrentsRecheckRes;
        struct TorrentsRemoveReq;
        struct TorrentsRemoveRes;
        struct TorrentsResumeReq;
        struct TorrentsResumeRes;
        struct TorrentsTrackersAddReq;
        struct TorrentsTrackersAddRes;
        struct TorrentsTrackersListReq;
        struct TorrentsTrackersListRes;

        void from_json(const nlohmann::json& json, TorrentsAddReq& req);
        void from_json(const nlohmann::json& json, TorrentsCountReq& req);
        void from_json(const nlohmann::json& json, TorrentsFilesListReq& req);
        void from_json(const nlohmann::json& json, TorrentsFilesPrioritiesReq& req);
        void from_json(const nlohmann::json& json, TorrentsFilesPrioritizeReq& req);
        void from_json(const nlohmann::json& json, TorrentsFilesProgressReq& req);
        void from_json(const nlohmann::json& json, TorrentsFilesRenameReq& req);
        void from_json(const nlohmann::json& json, TorrentsGetReq& req);
        void from_json(const nlohmann::json& json, TorrentsListReq& req);
        void from_json(const nlohmann::json& json, TorrentsMigrateReq& req);
        void from_json(const nlohmann::json& json, TorrentsMoveReq& req);
        void from_json(const nlohmann::json& json, TorrentsPauseReq& req);
        void from_json(const nlohmann::json& json, TorrentsPeersAddReq& req);
        void from_json(const nlohmann::json& json, TorrentsPeersListReq& req);
        void from_json(const nlohmann::json& json, TorrentsPiecesGetReq& req);
        void from_json(const nlohmann::json& json, TorrentsPropertiesGetReq& req);
        void from_json(const nlohmann::json& json, TorrentsPropertiesSetReq& req);
        void from_json(const nlohmann::json& json, TorrentsQueueAnyReq& req);
        void from_json(const nlohmann::json& json, TorrentsQueueSetReq& req);
        void from_json(const nlohmann::json& json, TorrentsRecheckReq& req);
        void from_json(const nlohmann::json& json, TorrentsResumeReq& req);
        void from_json(const nlohmann::json& json, TorrentsRemoveReq& req);
        void from_json(const nlohmann::json& json, TorrentsTrackersAddReq& req);
        void from_json(const nlohmann::json& json, TorrentsTrackersListReq& req);

        void to_json(nlohmann::json& json, const TorrentsAddRes& res);
        void to_json(nlohmann::json& json, const TorrentsCountRes& res);
        void to_json(nlohmann::json& json, const TorrentsFilesListRes& res);
        void to_json(nlohmann::json& json, const TorrentsFilesPrioritiesRes& res);
        void to_json(nlohmann::json& json, const TorrentsFilesPrioritizeRes& res);
        void to_json(nlohmann::json& json, const TorrentsFilesProgressRes& res);
        void to_json(nlohmann::json& json, const TorrentsFilesRenameRes& res);
        void to_json(nlohmann::json& json, const TorrentsGetRes& res);
        void to_json(nlohmann::json& json, const TorrentsListRes& res);
        void to_json(nlohmann::json& json, const TorrentsMigrateRes& res);
        void to_json(nlohmann::json& json, const TorrentsMoveRes& res);
        void to_json(nlohmann::json& json, const TorrentsPauseRes& res);
        void to_json(nlohmann::json& json, const TorrentsPeersAddRes& res);
        void to_json(nlohmann::json& json, const TorrentsPeersListRes& res);
        void to_json(nlohmann::json& json, const TorrentsPiecesGetRes& res);
        void to_json(nlohmann::json& json, const TorrentsPropertiesGetRes& res);
        void to_json(nlohmann::json& json, const TorrentsQueueAnyRes& res);
        void to_json(nlohmann::json& json, const TorrentsQueueSetRes& res);
        void to_json(nlohmann::json& json, const TorrentsRecheckRes& res);
        void to_json(nlohmann::json& json, const TorrentsRemoveRes& res);
        void to_json(nlohmann::json& json, const TorrentsResumeRes& res);
        void to_json(nlohmann::json& json, const TorrentsTrackersAddRes& res);
        void to_json(nlohmann::json& json, const TorrentsTrackersListRes& res);
    }
}
