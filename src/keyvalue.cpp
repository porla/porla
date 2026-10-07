#include "keyvalue.hpp"

#include "data/models/keyvaluestore.hpp"
#include "data/transaction.hpp"
#include "events.hpp"

using porla::KeyValue;
using porla::KeyValueEvent;
using porla::Data::Models::KeyValueStore;

KeyValue::KeyValue(const KeyValueOptions& options)
    : m_options(options)
{
}

nlohmann::json KeyValue::Get(const std::string& key) const
{
    return KeyValueStore::Get(m_options.db, key);
}

void KeyValue::Set(const std::string& key, const nlohmann::json& value)
{
    Set(std::map<std::string, nlohmann::json>{ { key, value } });
}

void KeyValue::Set(const std::map<std::string, nlohmann::json>& values)
{
    KeyValueEvent event("kv.updated");

    {
        Data::Transaction tx(m_options.db);

        for (const auto& [ key, value ] : values)
        {
            if (KeyValueStore::Set(m_options.db, key, value))
            {
                event.keys.push_back(key);
            }
        }
    }

    if (event.keys.empty())
    {
        return;
    }

    m_options.events.Publish(std::move(event));
}
