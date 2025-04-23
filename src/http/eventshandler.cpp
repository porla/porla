#include "eventshandler.hpp"

#include <boost/log/trivial.hpp>
#include <boost/signals2.hpp>
#include <nlohmann/json.hpp>

#include "../json/all.hpp"
#include "../sessions.hpp"

using json = nlohmann::json;
using porla::Http::EventsHandler;

template <bool SSL> template <bool SSLS> class EventsHandler<SSL>::State
{
public:
    explicit State(porla::Sessions& sessions)
    {
        m_state_update_connection = sessions.OnStateUpdate([this](auto && p1, auto && p2) { OnStateUpdate(p1, p2); });
        m_torrent_paused_connection = sessions.OnTorrentPaused([this](auto && p1, auto && p2) { OnTorrentPaused(p1, p2); });
        m_torrent_removed_connection = sessions.OnTorrentRemoved([this](auto && p1, auto && p2) { OnTorrentRemoved(p1, p2); });
        m_torrent_resumed_connection = sessions.OnTorrentResumed([this](auto && p1, auto && p2) { OnTorrentResumed(p1, p2); });
    }

    ~State()
    {
        m_state_update_connection.disconnect();
        m_torrent_paused_connection.disconnect();
        m_torrent_removed_connection.disconnect();
        m_torrent_resumed_connection.disconnect();
    }

    void Add(uWS::HttpResponse<SSLS>* res)
    {
        res->writeStatus("200 OK")
            ->writeHeader("Connection", "keep-alive")
            ->writeHeader("Content-Type", "text/event-stream")
            ->writeHeader("Cache-Control", "no-cache, no-transform");

        std::stringstream evt;
        evt << "event: hello\n";
        evt << "data: {}\n\n";

        res->write(evt.str());

        m_responses.insert(res);
    }

    void Remove(uWS::HttpResponse<SSLS>* res)
    {
        m_responses.erase(res);
    }

private:
    void Broadcast(const std::string& name, const std::string& data)
    {
        if (m_responses.empty())
        {
            return;
        }

        std::stringstream evt;
        evt << "event: "<< name << "\n";
        evt << "data: " << data << "\n\n";

        for (auto& res : m_responses)
        {
            res->write(evt.str());
        }
    }

    void OnStateUpdate(const std::string& session, const std::vector<libtorrent::torrent_status>& torrents)
    {
        json state = json::array();

        for (const auto& status : torrents)
        {
            state.push_back(status.info_hashes);
        }

        Broadcast("state_update", json({
            {"session", session},
            {"info_hashes", state}
        }).dump());
    }

    void OnTorrentPaused(const std::string& session, const libtorrent::torrent_handle& th)
    {
        Broadcast("torrent_paused", json({
            {"info_hash", th.info_hashes()},
            {"session", session}
        }).dump());
    }

    void OnTorrentRemoved(const std::string& session, const libtorrent::info_hash_t& hash)
    {
        Broadcast("torrent_removed", json({
            {"info_hash", hash},
            {"session", session}
        }).dump());
    }

    void OnTorrentResumed(const std::string& session, const libtorrent::torrent_handle& th)
    {
        Broadcast("torrent_resumed", json({
            {"info_hash", th.info_hashes()},
            {"session", session}
        }).dump());
    }

    std::unordered_set<uWS::HttpResponse<SSLS>*> m_responses;

    boost::signals2::connection m_state_update_connection;
    boost::signals2::connection m_torrent_paused_connection;
    boost::signals2::connection m_torrent_removed_connection;
    boost::signals2::connection m_torrent_resumed_connection;
};

template <bool SSL> EventsHandler<SSL>::EventsHandler(porla::Sessions& sessions)
    : m_state(std::make_shared<State<SSL>>(sessions))
{
}

template <bool SSL> EventsHandler<SSL>::~EventsHandler() = default;

template <bool SSL> void EventsHandler<SSL>::operator()(uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req)
{
    res->onAborted(
        [state = m_state, res]()
        {
            state->Remove(res);
        });

    m_state->Add(res);
}

namespace porla::Http {
    template class EventsHandler<true>;
    template class EventsHandler<false>;
}
