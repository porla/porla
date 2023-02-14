#include "flags.hpp"

#include "../../../json/lttorrentstatus.hpp"
#include "../../../session.hpp"

using porla::Workflows::Actions::Torrents::Flags;

Flags::Flags(porla::ISession& session)
    : m_session(session)
{
}

Flags::~Flags() = default;

void Flags::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    const auto& torrent = params.Render("torrent", true);

    if (torrent == nullptr)
    {
        // WARN
        return;
    }

    const lt::torrent_status& ts = torrent;
    const auto& th = m_session.Torrents().find(ts.info_hashes);

    if (th == m_session.Torrents().end())
    {
        return;
    }

    lt::torrent_flags_t set = {};
    lt::torrent_flags_t unset = {};

    const auto parse_flags = [](lt::torrent_flags_t flags, const std::vector<std::string>& input_flags)
    {
        for (const auto& f : input_flags)
        {
            if (f == "apply_ip_filter")     flags |= lt::torrent_flags::apply_ip_filter;
            if (f == "auto_managed")        flags |= lt::torrent_flags::auto_managed;
            if (f == "disable_dht")         flags |= lt::torrent_flags::disable_dht;
            if (f == "disable_lsd")         flags |= lt::torrent_flags::disable_lsd;
            if (f == "disable_pex")         flags |= lt::torrent_flags::disable_pex;
            if (f == "sequential_download") flags |= lt::torrent_flags::sequential_download;
        }
    };

    if (params.Input().contains("set"))   parse_flags(set, params.Input()["set"]);
    if (params.Input().contains("unset")) parse_flags(unset, params.Input()["unset"]);

    th->second.set_flags(set);
    th->second.unset_flags(unset);

    callback->Complete(true);
}
