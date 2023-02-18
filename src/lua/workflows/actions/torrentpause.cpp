#include "torrentpause.hpp"

#include <utility>

#include "../../../session.hpp"

using porla::Lua::Workflows::ActionCallback;
using porla::Lua::Workflows::Actions::TorrentPause;
using porla::Lua::Workflows::Actions::TorrentPauseOptions;

struct WorkItem
{
    std::shared_ptr<ActionCallback> callback;
    lt::info_hash_t                 info_hash;
    std::shared_ptr<void>           state;
};

class TorrentPause::State : public std::enable_shared_from_this<TorrentPause::State>
{
public:
    explicit State(const TorrentPauseOptions& opts)
        : m_opts(opts)
    {
        m_on_torrent_paused = m_opts.session.OnTorrentPaused([this](auto && s) { OnTorrentPaused(s); });
    }

    ~State()
    {
        m_on_torrent_paused.disconnect();
    }

    void Pause(const lt::torrent_handle& th, std::shared_ptr<ActionCallback> callback)
    {
        if (m_work_item != nullptr)
        {
            return;
        }

        m_work_item            = std::make_unique<WorkItem>();
        m_work_item->callback  = std::move(callback);
        m_work_item->info_hash = th.info_hashes();
        m_work_item->state     = shared_from_this();

        th.pause();
    }

private:
    void OnTorrentPaused(const libtorrent::torrent_handle &th)
    {
        if (m_work_item == nullptr)                     return;
        if (th.info_hashes() != m_work_item->info_hash) return;

        m_work_item->callback->Complete();
        m_work_item->state = nullptr;
        m_work_item        = nullptr;
    }

    TorrentPauseOptions         m_opts;
    boost::signals2::connection m_on_torrent_paused;
    std::unique_ptr<WorkItem>   m_work_item;
};

TorrentPause::TorrentPause(const TorrentPauseOptions& opts)
    : m_state(std::make_shared<State>(opts))
{
}

TorrentPause::~TorrentPause() = default;

void TorrentPause::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    const lt::torrent_handle& th = params.context["lt:torrent_handle"];
    m_state->Pause(th, callback);
}
