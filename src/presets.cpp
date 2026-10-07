#include "presets.hpp"

#include "data/transaction.hpp"
#include "events.hpp"
#include "sessions.hpp"

using porla::Presets;
using porla::PresetEvent;

using Model = porla::Data::Models::Presets;

Presets::Presets(const PresetsOptions& options)
    : m_options(options)
{
    m_session_removed = m_options.events.On("session.removed", [this](const Event& event)
    {
        ClearSession(static_cast<const SessionEvent&>(event).session_id);
    });
}

int Presets::Add(const std::string& name)
{
    const int id = Model::Insert(m_options.db, name);

    PresetEvent event("preset.added");
    event.preset_id = id;
    m_options.events.Publish(std::move(event));

    return id;
}

std::vector<Presets::Preset> Presets::All() const
{
    std::vector<Preset> presets;

    Model::ForEach(
        m_options.db,
        [&presets](const Preset& preset)
        {
            presets.push_back(preset);
        });

    return presets;
}

std::optional<Presets::Preset> Presets::Get(int id) const
{
    return Model::GetById(m_options.db, id);
}

std::optional<Presets::Preset> Presets::GetByName(const std::string& name) const
{
    return Model::GetByName(m_options.db, name);
}

std::optional<Presets::Preset> Presets::GetDefault() const
{
    return Model::GetDefault(m_options.db);
}

void Presets::Remove(int id)
{
    const auto preset = Model::GetById(m_options.db, id);

    if (!preset.has_value())
    {
        return;
    }

    Model::Remove(m_options.db, id);

    PresetEvent event("preset.removed", { { "preset_name", preset->name } });
    event.preset_id = id;

    m_options.events.Publish(std::move(event));
}

void Presets::Update(const Presets::Preset& preset)
{
    const auto previous_default = Model::GetDefault(m_options.db);

    Model::Update(m_options.db, preset);

    PublishUpdated(preset.id);

    // making a preset the default clears the flag on the old default - that changed too
    if (preset.is_default && previous_default.has_value() && previous_default->id != preset.id)
    {
        PublishUpdated(previous_default->id);
    }
}

void Presets::ClearSession(int session_id)
{
    std::vector<int> cleared;

    {
        Data::Transaction tx(m_options.db);

        for (auto preset : All())
        {
            if (preset.session_id != session_id)
            {
                continue;
            }

            preset.session_id = std::nullopt;

            Model::Update(m_options.db, preset);

            cleared.push_back(preset.id);
        }
    }

    for (const int id : cleared)
    {
        PublishUpdated(id);
    }
}

void Presets::PublishUpdated(int id)
{
    PresetEvent event("preset.updated");
    event.preset_id = id;

    m_options.events.Publish(std::move(event));
}
