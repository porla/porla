#include "torrentflags.hpp"

#include <utility>

#include <boost/log/trivial.hpp>

#include "../../../session.hpp"

using porla::Lua::Workflows::Actions::TorrentFlags;
using porla::Lua::Workflows::Actions::TorrentFlagsOptions;

TorrentFlags::TorrentFlags(TorrentFlagsOptions opts)
    : m_opts(std::move(opts))
{
}

TorrentFlags::~TorrentFlags() = default;

void TorrentFlags::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    const lt::torrent_handle& handle = params.context["lt:torrent_handle"];

    lt::torrent_flags_t set   = {};
    lt::torrent_flags_t unset = {};

    const auto parse_flags = [](lt::torrent_flags_t& flags, const std::vector<std::string>& input_flags)
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

    parse_flags(set,   m_opts.set);
    parse_flags(unset, m_opts.unset);

    handle.set_flags(set);
    handle.unset_flags(unset);

    callback->Complete();
}
