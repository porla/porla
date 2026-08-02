#include "sessionsupdate.hpp"

#include <boost/log/trivial.hpp>

#include "../../sessions.hpp"
#include "../../data/models/sessions.hpp"
#include "../../utils/ltsettings.hpp"

using porla::Methods::Sessions::SessionsUpdate;
using porla::Methods::Sessions::SessionsUpdateReq;
using porla::Methods::Sessions::SessionsUpdateRes;
using porla::Utils::LibtorrentSettingsPack;

SessionsUpdate::SessionsUpdate(sqlite3* db, porla::Sessions& sessions)
    : m_db(db),
    m_sessions(sessions)
{
}

void SessionsUpdate::Invoke(const SessionsUpdateReq& req, WriteCb<SessionsUpdateRes> cb)
{
    const auto& state = m_sessions.Get(req.id);

    if (state == nullptr)
    {
        return cb.Error(-1, "Session not found");
    }

    if (req.is_default && !state->is_default)
    {
        // If we send is_default=true and this session is not the default,
        // then make this session the default.

        porla::Data::Models::Sessions::SetDefault(
            m_db,
            req.id);

        const auto& current_default = m_sessions.Default();

        if (current_default != nullptr)
        {
            current_default->is_default = false;
        }

        state->is_default = true;

        BOOST_LOG_TRIVIAL(info) << "Default session is now '" << state->name << "'";
    }

    porla::Data::Models::Sessions::Update(
        m_db,
        req.id,
        req.name,
        req.metadata);

    state->metadata = req.metadata;
    state->name = req.name;

    cb.Ok(SessionsUpdateRes{});
}
