#include "torrentmove.hpp"

#include <utility>

#include <boost/log/trivial.hpp>

#include "../../../session.hpp"

using porla::Lua::Workflows::ActionCallback;
using porla::Lua::Workflows::Actions::TorrentMove;
using porla::Lua::Workflows::Actions::TorrentMoveOptions;

struct WorkItem
{
    std::shared_ptr<ActionCallback> callback;
    lt::info_hash_t                 info_hash;
    std::shared_ptr<void>           state;
};

class TorrentMove::State : public std::enable_shared_from_this<TorrentMove::State>
{
public:
    explicit State(TorrentMoveOptions opts)
        : m_opts(std::move(opts))
    {
        m_on_storage_moved = m_opts.session.OnStorageMoved([this](auto && s) { OnStorageMoved(s); });
    }

    ~State()
    {
        m_on_storage_moved.disconnect();
    }

    void Move(const lt::torrent_handle& th, const sol::table& context, std::shared_ptr<ActionCallback> callback)
    {
        if (m_work_item != nullptr)
        {
            return;
        }

        m_work_item            = std::make_unique<WorkItem>();
        m_work_item->callback  = std::move(callback);
        m_work_item->info_hash = th.info_hashes();
        m_work_item->state     = shared_from_this();

        std::string path;

        if (m_opts.path.is<std::string>())
        {
            path = m_opts.path.as<std::string>();
        }

        if (m_opts.path.is<std::function<std::string(sol::table)>>())
        {
            path = m_opts.path.as<std::function<std::string(sol::table)>>()(context);
        }

        th.move_storage(path);
    }

private:
    void OnStorageMoved(const libtorrent::torrent_handle& th)
    {
        if (m_work_item == nullptr)                     return;
        if (th.info_hashes() != m_work_item->info_hash) return;

        m_work_item->callback->Complete();
        m_work_item->state = nullptr;
    }

    TorrentMoveOptions          m_opts;
    boost::signals2::connection m_on_storage_moved;
    std::unique_ptr<WorkItem>   m_work_item;
};

TorrentMove::TorrentMove(const TorrentMoveOptions& opts)
    : m_state(std::make_shared<State>(opts))
{
}

TorrentMove::~TorrentMove() = default;

void TorrentMove::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    const lt::torrent_handle& th = params.context["lt:torrent_handle"];
    m_state->Move(th, params.context, callback);
}
