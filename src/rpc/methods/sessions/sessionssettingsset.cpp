#include "sessionssettingsset.hpp"

#include <boost/log/trivial.hpp>

#include "../resolve.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../utils/ltsettings.hpp"

using porla::Rpc::Methods::Sessions::SessionsSettingsSet;
using porla::Rpc::Methods::Sessions::SessionsSettingsSetReq;
using porla::Utils::LibtorrentSettingsPack;

SessionsSettingsSet::SessionsSettingsSet(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void SessionsSettingsSet::Execute(const SessionsSettingsSetReq &req, ResponseWriterHandle cb)
{
    const auto session = ResolveSession(m_sessions, req.id, cb);

    if (!session)
    {
        return;
    }

    lt::settings_pack settings = req.settings;

    LibtorrentSettingsPack::UpdateStatic(settings);

    session->session->Libtorrent().apply_settings(settings);
    session->session->Persist();

    BOOST_LOG_TRIVIAL(info) << "Session settings for " << session->session->Name() << " updated";

    cb->Ok();
}
