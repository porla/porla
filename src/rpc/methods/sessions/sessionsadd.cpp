#include "sessionsadd.hpp"

#include <map>

#include <libtorrent/session.hpp>

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../utils/ltsettings.hpp"

using porla::Rpc::Methods::Sessions::SessionsAdd;
using porla::Rpc::Methods::Sessions::SessionsAddReq;
using porla::Rpc::Methods::Sessions::SessionsAddRes;
using porla::Utils::LibtorrentSettingsPack;

namespace
{
    static const std::map<std::string, std::function<lt::settings_pack()>> packs
    {
        { "default", &lt::default_settings },
        { "min_memory_usage", &lt::min_memory_usage },
        { "high_performance_seed", &lt::high_performance_seed }
    };
}

SessionsAdd::SessionsAdd(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionsAdd::Execute(const SessionsAddReq& req, ResponseWriterHandle cb)
{
    std::string settings_base = req.settings_base.value_or("default");

    const auto found_pack = packs.find(settings_base);

    if (found_pack == packs.end())
    {
        return cb->Error(-32602, "Invalid session settings base", {{"field","settings_base"}});
    }

    lt::settings_pack settings = found_pack->second();

    LibtorrentSettingsPack::Update(
        settings,
        req.settings.value_or(std::map<std::string, nlohmann::json>()));

    const auto session = porla::Sessions::Record{
        .id                    = -1,
        .name                  = req.name,
        .is_default            = false,
        .metadata              = req.metadata.value_or(std::map<std::string, nlohmann::json>()),
        .params                = lt::session_params(settings),
        .timer_dht_stats       = req.timer_dht_stats.value_or(5000),
        .timer_save_state      = req.timer_save_state.value_or(300000),
        .timer_session_stats   = req.timer_session_stats.value_or(5000),
        .timer_torrent_updates = req.timer_torrent_updates.value_or(1000)
    };

    cb->Ok(SessionsAddRes{
        .id = m_sessions.Add(session)
    });
}
