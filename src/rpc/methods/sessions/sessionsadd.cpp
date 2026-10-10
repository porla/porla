#include "sessionsadd.hpp"

#include <libtorrent/session.hpp>

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../utils/ltsettings.hpp"

using porla::Rpc::Methods::Sessions::SessionsAdd;
using porla::Rpc::Methods::Sessions::SessionsAddReq;
using porla::Rpc::Methods::Sessions::SessionsAddRes;
using porla::Utils::LibtorrentSettingsPack;

SessionsAdd::SessionsAdd(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionsAdd::Execute(const SessionsAddReq& req, ResponseWriterHandle cb)
{
    std::string settings_base = req.settings_base.value_or("default");

    lt::settings_pack settings;
    if (settings_base == "default")               settings = lt::default_settings();
    if (settings_base == "min_memory_usage")      settings = lt::min_memory_usage();
    if (settings_base == "high_performance_seed") settings = lt::high_performance_seed();

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
