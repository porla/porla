#pragma once

#include "session.hpp"
#include "sessionevent.hpp"
#include "torrentevent.hpp"

#include "../events.hpp"
#include "../json/all.hpp"

namespace porla
{
    template<typename T>
    void Session::Publish(T event)
    {
        static_assert(std::is_base_of_v<SessionEvent, T>);

        event.session_id = m_id;
        event.session    = weak_from_this();

        m_options.events.Publish(std::move(event));
    }

    template<typename Alert>
    void Session::EmitTorrentEvent(std::string name, const Alert& alert, nlohmann::json extra)
    {
        if (!m_options.events.HasSubscribers(name) || !alert.handle.is_valid())
        {
            return;
        }

        nlohmann::json data = alert;

        if (extra.is_object())
        {
            data.update(extra);
        }

        TorrentEvent event(std::move(name), std::move(data));
        event.torrent_handle = alert.handle;
        event.info_hash      = alert.handle.info_hashes();

        Publish(std::move(event));
    }
}
