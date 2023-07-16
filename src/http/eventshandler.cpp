#include "eventshandler.hpp"

#include <boost/log/trivial.hpp>
#include <nlohmann/json.hpp>

#include "../json/all.hpp"
#include "../session.hpp"

using json = nlohmann::json;
using porla::Http::EventsHandler;

class EventsHandler::State
{
public:
    explicit State(porla::ISession& session)
    {
        m_state_update_connection = session.OnStateUpdate([this](auto s) { OnStateUpdate(s); });
        m_torrent_paused_connection = session.OnTorrentPaused([this](auto s) { OnTorrentPaused(s); });
        m_torrent_removed_connection = session.OnTorrentRemoved([this](auto s) { OnTorrentRemoved(s); });
        m_torrent_resumed_connection = session.OnTorrentResumed([this](auto s) { OnTorrentResumed(s); });
    }

    ~State()
    {
        m_state_update_connection.disconnect();
        m_torrent_paused_connection.disconnect();
        m_torrent_removed_connection.disconnect();
        m_torrent_resumed_connection.disconnect();
    }

    void Add(uWS::HttpResponse<false>* res)
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

    void Remove(uWS::HttpResponse<false>* res)
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

    void OnStateUpdate(const std::vector<libtorrent::torrent_status>& torrents)
    {
        json state = json::array();

        for (const auto& status : torrents)
        {
            json j = {
                {"info_hash", status.info_hashes}
            };

            state.push_back(j);
        }

        Broadcast("state_update", state.dump());
    }

    void OnTorrentPaused(const libtorrent::torrent_handle& th)
    {
        Broadcast("torrent_paused", json({"info_hash", th.info_hashes()}).dump());
    }

    void OnTorrentRemoved(const libtorrent::info_hash_t& hash)
    {
        Broadcast("torrent_removed", json({"info_hash", hash}).dump());
    }

    void OnTorrentResumed(const libtorrent::torrent_status& status)
    {
        Broadcast("torrent_resumed", json({"info_hash", status.info_hashes}).dump());
    }

    std::unordered_set<uWS::HttpResponse<false>*> m_responses;

    boost::signals2::connection m_state_update_connection;
    boost::signals2::connection m_torrent_paused_connection;
    boost::signals2::connection m_torrent_removed_connection;
    boost::signals2::connection m_torrent_resumed_connection;
};

EventsHandler::EventsHandler(porla::ISession& session)
    : m_state(std::make_shared<State>(session))
{
}

EventsHandler::~EventsHandler() = default;

void EventsHandler::operator()(uWS::HttpResponse<false>* res, uWS::HttpRequest* req)
{
    res->onAborted(
        [state = m_state, res]()
        {
            state->Remove(res);
        });

    m_state->Add(res);
}
