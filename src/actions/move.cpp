#include "move.hpp"

#include <boost/log/trivial.hpp>

#include "actioncallback.hpp"
#include "../session.hpp"

using porla::Actions::Move;

struct Move::MoveContext
{
    std::shared_ptr<ActionCallback> callback;
};

Move::Move(ISession &session)
    : m_session(session)
{
    m_storageMovedConnection = m_session.OnStorageMoved([this](auto && s) { OnStorageMoved(s); });
}

Move::~Move()
{
    m_storageMovedConnection.disconnect();
}

void Move::Invoke(const libtorrent::info_hash_t& hash, const toml::array& args, const std::shared_ptr<ActionCallback>& callback)
{
    auto const& torrents = m_session.Torrents();
    auto const& handle = torrents.find(hash);

    if (handle == torrents.end())
    {
        BOOST_LOG_TRIVIAL(warning) << "(move) Could not find torrent";
        return;
    }

    if (args.empty() || !args[0].is_string())
    {
        return;
    }

    handle->second.move_storage(*args[0].value<std::string>());

    auto ctx = std::make_unique<MoveContext>();
    ctx->callback = callback;

    m_state.insert({ hash, std::move(ctx)});
}

void Move::OnStorageMoved(const libtorrent::torrent_handle& th)
{
    auto ctx = m_state.find(th.info_hashes());
    if (ctx == m_state.end()) return;

    ctx->second->callback->Invoke(true);

    m_state.erase(ctx);
}
