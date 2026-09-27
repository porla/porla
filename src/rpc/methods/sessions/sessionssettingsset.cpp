#include "sessionssettingsset.hpp"

#include <boost/log/trivial.hpp>

#include "../../../data/models/sessions.hpp"
#include "../../../sessions.hpp"
#include "../../../utils/ltsettings.hpp"

using porla::Rpc::Methods::Sessions::SessionsSettingsSet;
using porla::Rpc::Methods::Sessions::SessionsSettingsSetReq;
using porla::Rpc::Methods::Sessions::SessionsSettingsSetRes;
using porla::Utils::LibtorrentSettingsPack;

SessionsSettingsSet::SessionsSettingsSet(sqlite3* db, porla::Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void SessionsSettingsSet::Execute(const SessionsSettingsSetReq &req, ResponseWriterHandle cb)
{
    const auto session = Data::Models::Sessions::GetById(m_db, req.id);

    if (!session)
    {
        return cb->Error(-1, "Session not found");
    }

    const auto& state = m_sessions.Get(session->id);

    if (state == nullptr)
    {
        return cb->Error(-2, "Session not loaded");
    }

    lt::settings_pack settings = req.settings;

    LibtorrentSettingsPack::UpdateStatic(settings);

    state->session->apply_settings(settings);

    m_sessions.SaveSessionParams(state);

    BOOST_LOG_TRIVIAL(info) << "Session settings for " << session->name << " updated";

    cb->Ok(SessionsSettingsSetRes{});
}
