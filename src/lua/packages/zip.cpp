#include "zip.hpp"

#include <zip.h>

using porla::Lua::Packages::Zip;

sol::object Zip::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl.set_function("read", [](sol::this_state ts, const std::string& buffer) -> std::tuple<std::optional<sol::table>, std::optional<std::string>>
    {
        sol::state_view lua(ts);

        zip_error_t err;
        zip_error_init(&err);

        zip_source_t* source = zip_source_buffer_create(
            buffer.data(),
            buffer.size(),
            0,
            &err);

        if (source == nullptr)
        {
            return std::make_tuple(std::nullopt, std::string(zip_error_strerror(&err)));
        }

        zip_t* archive = zip_open_from_source(source, ZIP_RDONLY, &err);

        if (archive == nullptr)
        {
            zip_source_free(source);
            return std::make_tuple(std::nullopt, std::string(zip_error_strerror(&err)));
        }

        const zip_int64_t num_entries = zip_get_num_entries(archive, ZIP_FL_UNCHANGED);

        if (num_entries < 0)
        {
            const auto error = std::string(zip_error_strerror(zip_get_error(archive)));
            zip_close(archive);
            return std::make_tuple(std::nullopt, error);
        }

        sol::table files = lua.create_table();

        for (int i = 0; i < num_entries; i++)
        {
            zip_stat_t st;
            zip_stat_init(&st);

            if (zip_stat_index(archive, i, 0, &st) != 0)
            {
                const auto error = std::string(zip_error_strerror(zip_get_error(archive)));

                zip_close(archive);

                return std::make_tuple(std::nullopt, error);
            }

            if (!(st.valid & ZIP_STAT_SIZE))
            {
                continue;
            }

            const auto file_name = std::string(st.name);

            if (file_name.ends_with('/'))
            {
                continue;
            }

            zip_file* file = zip_fopen_index(archive, i, ZIP_FL_UNCHANGED);

            if (file == nullptr)
            {
                const auto error = std::string(zip_error_strerror(zip_get_error(archive)));

                zip_close(archive);

                return std::make_tuple(std::nullopt, error);
            }

            std::string contents;
            contents.resize(st.size);

            zip_fread(file, contents.data(), contents.size());
            zip_fclose(file);

            files[file_name] = contents;
        }

        zip_close(archive);

        return std::make_tuple(files, std::nullopt);
    });

    return tbl;
}
