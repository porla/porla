#include "executor.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/torrent_status.hpp>

#include "action.hpp"
#include "actioncallback.hpp"

#include "../data/models/torrentsmetadata.hpp"
#include "../session.hpp"

using porla::Actions::Action;
using porla::Actions::ActionCallback;
using porla::Actions::Executor;
using porla::Data::Models::TorrentsMetadata;

struct ActionState
{
    std::shared_ptr<Action>  action;
    std::vector<std::string> arguments;
};

struct LoopingActionCallback : public ActionCallback, public std::enable_shared_from_this<LoopingActionCallback>
{
    explicit LoopingActionCallback(const libtorrent::info_hash_t& hash, const std::vector<ActionState>& action_states)
        : m_hash(hash)
        , m_current_index(0)
        , m_action_states(action_states)
    {
    }

    void Invoke(bool success) override
    {
        if (success)
        {
            m_current_index++;

            if (m_current_index >= m_action_states.size())
            {
                return;
            }

            Run();
        }
    }

    void Run()
    {
        const auto& state = m_action_states.at(m_current_index);
        state.action->Invoke(m_hash, state.arguments, shared_from_this());
    }

private:
    libtorrent::info_hash_t m_hash;
    std::vector<ActionState> m_action_states;
    int m_current_index;
};

Executor::Executor(const ExecutorOptions& options)
    : m_db(options.db)
    , m_io(options.io)
    , m_session(options.session)
    , m_actions(options.actions)
    , m_presets(options.presets)
{
    m_torrent_added_connection = m_session.OnTorrentFinished([this](auto && s) { OnTorrentFinished(s); });
}

Executor::~Executor()
{
    m_torrent_added_connection.disconnect();
}

void Executor::OnTorrentFinished(const libtorrent::torrent_status& ts)
{
    // Get the preset for this torrent. If it has none, use the default
    // if it exists.
    auto const torrent_metadata = TorrentsMetadata::GetAll(m_db, ts.info_hashes);
    auto const preset_name = torrent_metadata.contains("preset")
        ? torrent_metadata.at("preset").get<std::string>()
        : "default";

    auto const& preset = m_presets.find(preset_name);

    if (preset == m_presets.end())
    {
        return;
    }

    // Run all actions in the preset
    std::vector<ActionState> action_states;

    for (const auto& preset_action : preset->second.on_finished)
    {
        auto action = m_actions.find(preset_action.action_name);

        if (action == m_actions.end())
        {
            BOOST_LOG_TRIVIAL(warning) << "Unknown action name: " << preset_action.action_name;
            continue;
        }

        action_states.emplace_back(ActionState{
            .action    = action->second,
            .arguments = preset_action.arguments
        });
    }

    if (action_states.empty())
    {
        return;
    }

    BOOST_LOG_TRIVIAL(info) << "Running " << action_states.size() << " action(s) for torrent " << ts.name;

    std::make_shared<LoopingActionCallback>(ts.info_hashes, action_states)->Run();
}
