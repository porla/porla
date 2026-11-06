#include "zip.hpp"

#include <zip.h>

#include "logger.hpp"

using porla::Zip;

namespace
{
    BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(logger, porla::Log::Logger) { return porla::Log::Component("zip"); }
}

std::map<std::string, std::vector<char>> Zip::Load(const std::vector<char>& buffer)
{
    zip_error_t err;
    zip_error_init(&err);

    zip_source_t* source = zip_source_buffer_create(
        buffer.data(),
        buffer.size(),
        0,
        &err);

    if (source == nullptr)
    {
        BOOST_LOG_SEV(logger::get(), Log::error)
            << Log::Property("error", zip_error_strerror(&err))
            << "Failed to craete zip source from buffer";

        return {};
    }

    zip_t* archive = zip_open_from_source(source, ZIP_RDONLY, &err);

    if (archive == nullptr)
    {
        BOOST_LOG_SEV(logger::get(), Log::error)
            << Log::Property("error", zip_error_strerror(&err))
            << "Failed to open zip archive from source";

        zip_source_close(source);
        return {};
    }

    zip_int64_t num_entries = zip_get_num_entries(archive, ZIP_FL_UNCHANGED);

    std::map<std::string, std::vector<char>> files;

    for (int i = 0; i < num_entries; i++)
    {
        zip_stat_t st;
        zip_stat_init(&st);

        if (zip_stat_index(archive, i, 0, &st) != 0)
        {
            BOOST_LOG_SEV(logger::get(), Log::warning)
                << Log::Property("error", zip_error_strerror(zip_get_error(archive)))
                << "Failed to stat zip index " << i;

            continue;
        }

        if (st.size == 0)
        {
            continue;
        }

        zip_file* file = zip_fopen_index(archive, i, ZIP_FL_UNCHANGED);

        if (file == nullptr)
        {
            BOOST_LOG_SEV(logger::get(), Log::warning)
                << Log::Property("error", zip_error_strerror(zip_get_error(archive)))
                << Log::Property("file", st.name)
                << "Failed to open file " << st.name;

            continue;
        }

        std::vector<char> buffer;
        buffer.resize(st.size);

        zip_fread(file, &buffer[0], st.size);
        zip_fclose(file);

        files.emplace(st.name, buffer);
    }

    zip_close(archive);
    zip_source_close(source);

    return files;
}
