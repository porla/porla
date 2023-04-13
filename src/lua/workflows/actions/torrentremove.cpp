#include "torrentremove.hpp"

#include "../../../session.hpp"

using porla::Lua::Workflows::ActionCallback;
using porla::Lua::Workflows::Actions::TorrentRemove;
using porla::Lua::Workflows::Actions::TorrentRemoveOptions;

struct WorkItem
{
    std::shared_ptr<ActionCallback> callback;
    lt::info_hash_t                 info_hash;
    std::shared_ptr<void>           state;
};

class TorrentRemove::State : public std::enable_shared_from_this<TorrentRemove::State>
{
public:
    explicit State(const TorrentRemoveOptions& opts)
        : m_opts(opts)
    {
        m_on_torrent_removed = opts.session.OnTorrentRemoved([this](auto && s) { OnTorrentRemoved(s); });
    }

    ~State()
    {
        m_on_torrent_removed.disconnect();
    }

    void Remove(const lt::torrent_handle& th, const std::shared_ptr<ActionCallback>& callback)
    {
        if (m_work_item != nullptr)
        {
            return;
        }

        m_work_item            = std::make_unique<WorkItem>();
        m_work_item->callback  = callback;
        m_work_item->info_hash = th.info_hashes();
        m_work_item->state     = shared_from_this();

        m_opts.session.Remove(th.info_hashes(), m_opts.remove_files);
    }

private:
    void OnTorrentRemoved(const lt::info_hash_t& hash)
    {
        if (m_work_item == nullptr)         return;
        if (hash != m_work_item->info_hash) return;

        m_work_item->callback->Complete();
        m_work_item->state = nullptr;
    }

    boost::signals2::connection     m_on_torrent_removed;
    TorrentRemoveOptions            m_opts;
    std::unique_ptr<WorkItem>       m_work_item;
};

TorrentRemove::TorrentRemove(TorrentRemoveOptions opts)
    : m_state(std::make_shared<State>(opts))
{
}

TorrentRemove::~TorrentRemove() = default;

void TorrentRemove::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    const lt::torrent_handle& th = params.context["lt:torrent_handle"];
    m_state->Remove(th, callback);
}
