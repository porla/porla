#include "../packages.hpp"

#include <zip.h>

#include "../types/poerror.hpp"

using porla::Lua::Packages::Zip;
using porla::Lua::Types::PoError;

namespace
{
    sol::table ZipError(lua_State* L, zip_error_t* error)
    {
        sol::table err = PoError::Make(L, porla::Errors::Code::decode_error, zip_error_strerror(error));
        err["category"] = "libzip";
        err["value"]    = zip_error_code_zip(error);

        return err;
    }
}

sol::object Zip::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl.set_function("read", [](sol::this_state ts, const std::string& buffer) -> std::tuple<sol::object, sol::object>
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
            return { sol::lua_nil, ZipError(ts, &err) };
        }

        zip_t* archive = zip_open_from_source(source, ZIP_RDONLY, &err);

        if (archive == nullptr)
        {
            zip_source_free(source);
            return { sol::lua_nil, ZipError(ts, &err) };
        }

        const zip_int64_t num_entries = zip_get_num_entries(archive, ZIP_FL_UNCHANGED);

        if (num_entries < 0)
        {
            sol::table error = ZipError(ts, zip_get_error(archive));
            zip_close(archive);
            return { sol::lua_nil, error };
        }

        sol::table files = lua.create_table();

        for (int i = 0; i < num_entries; i++)
        {
            zip_stat_t st;
            zip_stat_init(&st);

            if (zip_stat_index(archive, i, 0, &st) != 0)
            {
                sol::table error = ZipError(ts, zip_get_error(archive));

                zip_close(archive);

                return { sol::lua_nil, error };
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
                sol::table error = ZipError(ts, zip_get_error(archive));

                zip_close(archive);

                return { sol::lua_nil, error };
            }

            std::string contents;
            contents.resize(st.size);

            const auto read = zip_fread(file, contents.data(), contents.size());

            if (read < 0 || static_cast<zip_uint64_t>(read) != st.size)
            {
                zip_error_t short_read;
                zip_error_init_with_code(&short_read, ZIP_ER_READ);

                sol::table error = ZipError(ts, read < 0 ? zip_file_get_error(file) : &short_read);

                zip_error_fini(&short_read);

                zip_fclose(file);
                zip_close(archive);

                return { sol::lua_nil, error };
            }

            zip_fclose(file);

            files[file_name] = contents;
        }

        zip_close(archive);

        return { files, sol::lua_nil };
    });

    return tbl;
}
