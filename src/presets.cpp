#include "presets.hpp"

#include "events.hpp"

using porla::Presets;

using Model = porla::Data::Models::Presets;

Presets::Presets(const PresetsOptions& options)
    : m_options(options)
{
}

int Presets::Add(const std::string& name)
{
    const int id = Model::Insert(m_options.db, name);

    Event event("preset.added", {{ "preset_id", id }});
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

    Event event("preset.removed", {
        { "preset_id", id },
        { "preset_name", preset->name }
    });

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

void Presets::PublishUpdated(int id)
{
    Event event("preset.updated", {{ "preset_id", id }});
    m_options.events.Publish(std::move(event));
}
