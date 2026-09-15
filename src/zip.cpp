#include "zip.hpp"

#include <boost/log/trivial.hpp>
#include <zip.h>

using porla::Zip;

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
        BOOST_LOG_TRIVIAL(error) << "Failed to create zip source from buffer: " << zip_error_strerror(&err);
        return {};
    }

    zip_t* archive = zip_open_from_source(source, ZIP_RDONLY, &err);

    if (archive == nullptr)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to open zip archive from source: " << zip_error_strerror(&err);
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
            BOOST_LOG_TRIVIAL(warning) << "Failed to stat zip index " << i << ": " << zip_error_strerror(zip_get_error(archive));
            continue;
        }

        if (st.size == 0)
        {
            continue;
        }

        zip_file* file = zip_fopen_index(archive, i, ZIP_FL_UNCHANGED);

        if (file == nullptr)
        {
            BOOST_LOG_TRIVIAL(warning) << "Failed to open file " << st.name << " in archive: " << zip_error_strerror(zip_get_error(archive));
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
