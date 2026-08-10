#include "mmdb.hpp"

#include <boost/log/trivial.hpp>
#include <maxminddb.h>

using json = nlohmann::json;
using porla::Mmdb;

class Mmdb::State
{
public:
    MMDB_s mmdb;
};

struct MmdbEntryDeleter
{
    void operator()(MMDB_entry_data_list_s* s)
    {
        MMDB_free_entry_data_list(s);
    }
};

static MMDB_entry_data_list_s* MmdbEntryToJson(MMDB_entry_data_list_s* entry, json& output)
{
    if (entry == nullptr)
    {
        return nullptr;
    }

    switch (entry->entry_data.type)
    {
        case MMDB_DATA_TYPE_UTF8_STRING:
            output = std::string(
                entry->entry_data.utf8_string,
                entry->entry_data.data_size);
            entry = entry->next;
            break;

        case MMDB_DATA_TYPE_DOUBLE:
            output = entry->entry_data.double_value;
            entry = entry->next;
            break;

        case MMDB_DATA_TYPE_UINT16:
            output = entry->entry_data.uint16;
            entry = entry->next;
            break;

        case MMDB_DATA_TYPE_UINT32:
            output = entry->entry_data.uint32;
            entry = entry->next;
            break;

        case MMDB_DATA_TYPE_MAP:
        {
            output = json::object();
            uint32_t size = entry->entry_data.data_size;

            for (entry = entry->next; size && entry; size--)
            {
                if (entry->entry_data.type != MMDB_DATA_TYPE_UTF8_STRING)
                {
                    return nullptr;
                }

                std::string key(
                    entry->entry_data.utf8_string,
                    entry->entry_data.data_size);

                entry = entry->next;

                json value;
                entry = MmdbEntryToJson(entry, value);

                output[key] = value;
            }

            break;
        }

        case MMDB_DATA_TYPE_INT32:
            output = entry->entry_data.int32;
            entry = entry->next;
            break;

        case MMDB_DATA_TYPE_ARRAY:
        {
            output = json::array();
            uint32_t size = entry->entry_data.data_size;

            for (entry = entry->next; size && entry; size--)
            {
                json value;
                entry = MmdbEntryToJson(entry, value);
                output.emplace_back(value);
            }

            break;
        }

        case MMDB_DATA_TYPE_BOOLEAN:
            output = entry->entry_data.boolean;
            entry = entry->next;
            break;

        case MMDB_DATA_TYPE_FLOAT:
            output = entry->entry_data.float_value;
            entry = entry->next;
            break;

        default:
            BOOST_LOG_TRIVIAL(warning) << "MMDB type not handled: " << entry->entry_data.type;
            entry = entry->next;
            break;
    }

    return entry;
}

std::unique_ptr<Mmdb> Mmdb::Load(const std::filesystem::path& path)
{
    auto state = std::make_shared<Mmdb::State>();

    int result = MMDB_open(path.c_str(), MMDB_MODE_MMAP, &state->mmdb);

    if (result != MMDB_SUCCESS)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to open MMDB file: " << MMDB_strerror(result);
        return nullptr;
    }

    auto mmdb = std::make_unique<Mmdb>();
    mmdb->m_state = std::move(state);

    return std::move(mmdb);
}

Mmdb::~Mmdb()
{
    MMDB_close(&m_state->mmdb);
}

nlohmann::json Mmdb::Lookup(const std::string& key)
{
    int err_getaddrinfo;
    int err_mmdb;

    MMDB_lookup_result_s result = MMDB_lookup_string(
        &m_state->mmdb,
        key.c_str(),
        &err_getaddrinfo,
        &err_mmdb);

    if (err_getaddrinfo != 0)
    {
        return nullptr;
    }

    if (err_mmdb != MMDB_SUCCESS)
    {
        return nullptr;
    }

    if (result.found_entry)
    {
        MMDB_entry_data_list_s* entry_data_list = nullptr;

        int status = MMDB_get_entry_data_list(
            &result.entry,
            &entry_data_list);

        if (status != MMDB_SUCCESS)
        {
            BOOST_LOG_TRIVIAL(error) << "MMDB failed to get entry data: " << MMDB_strerror(status);
            return nullptr;
        }

        std::unique_ptr<MMDB_entry_data_list_s, MmdbEntryDeleter> entry(entry_data_list);
        json output;
        MmdbEntryToJson(entry.get(), output);
        return output;
    }

    return nullptr;
}
