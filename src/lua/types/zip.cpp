#include "../types.hpp"

#include <zip.h>

using porla::Lua::Types::Zip;

struct ZipFileT
{
    zip_file_t* file;
};

struct ZipT
{
    zip_t* zip;
};

void Zip::Register(sol::state& lua)
{
    sol::table zip = lua["zip"].valid()
        ? lua["zip"].get<sol::table>()
        : lua.create_named_table("zip");

    zip["zip_stat_t"] = lua.new_usertype<zip_stat_t>(
        "zip.zip_stat_t",
        sol::no_constructor,
        "valid", &zip_stat_t::valid,
        "name", &zip_stat_t::name,
        "index", &zip_stat_t::index,
        "size", &zip_stat_t::size,
        "comp_size", &zip_stat_t::comp_size,
        "mtime", &zip_stat_t::mtime,
        "crc", &zip_stat_t::crc,
        "comp_method", &zip_stat_t::comp_method,
        "encryption_method", &zip_stat_t::encryption_method,
        "flags", &zip_stat_t::flags
    );

    zip["zip_file_t"] = lua.new_usertype<ZipFileT>(
        "zip.zip_file_t",
        sol::no_constructor,
        "fclose", [](const ZipFileT& file) { return zip_fclose(file.file); },
        "fread", [](sol::this_state L, const ZipFileT& file, int bytes)
        {
            std::string buf(bytes, '\0');
            zip_int64_t n = zip_fread(file.file, buf.data(), buf.size());

            if (n < 0)
            {
                sol::variadic_results res;
                res.push_back(sol::make_object(L, sol::lua_nil));
                res.push_back(sol::make_object(L, "zip_fread failed"));

                return res;
            }

            buf.resize(static_cast<std::size_t>(n));

            sol::variadic_results res;
            res.push_back(sol::make_object(L, std::move(buf)));

            return res;
        }
    );

    zip["zip_t"] = lua.new_usertype<ZipT>(
        "zip.zip_t",
        sol::no_constructor,
        "close", [](const ZipT& zip)
        {
            return zip_close(zip.zip);
        },
        "discard", [](const ZipT& zip)
        {
            return zip_discard(zip.zip);
        },
        "fopen_index", [](sol::this_state L, const ZipT& zip, int index, int flags)
        {
            zip_file_t* file = zip_fopen_index(zip.zip, index, flags);

            if (file == nullptr)
            {
                zip_error_t* error = zip_get_error(zip.zip);

                sol::variadic_results res;
                res.push_back(sol::make_object(L, sol::lua_nil));
                res.push_back(sol::make_object(L, zip_error_strerror(error)));

                return res;
            }

            sol::variadic_results res;
            res.push_back(sol::make_object(L, ZipFileT{file}));

            return res;
        },
        "get_num_entries", [](const ZipT& zip, int flags)
        {
            return zip_get_num_entries(zip.zip, flags);
        },
        "stat_index", [](const ZipT& zip, int index, sol::this_state L)
        {
            zip_stat_t st;
            zip_stat_init(&st);

            int err = zip_stat_index(zip.zip, index, 0, & st);

            if (err < 0)
            {
                zip_error_t* error = zip_get_error(zip.zip);

                sol::variadic_results res;
                res.push_back(sol::make_object(L, sol::lua_nil));
                res.push_back(sol::make_object(L, zip_error_strerror(error)));

                return res;
            }

            sol::variadic_results res;
            res.push_back(sol::make_object(L, st));

            return res;
        }
    );

    zip["zip_t"]["open"] = [](const std::string& path, int flags, sol::this_state ts)
    {
        int err;
        zip_t* zip = zip_open(path.c_str(), flags, &err);

        if (zip == nullptr)
        {
            zip_error_t error;
            zip_error_init_with_code(&error, err);

            sol::variadic_results res;
            res.push_back(sol::make_object(ts, sol::lua_nil));
            res.push_back(sol::make_object(ts, zip_error_strerror(&error)));

            zip_error_fini(&error);

            return res;
        }

        sol::variadic_results res;
        res.push_back(sol::make_object(ts, ZipT{zip}));

        return res;
    };

    zip["ZIP_CREATE"]    = ZIP_CREATE;
    zip["ZIP_EXCL"]      = ZIP_EXCL;
    zip["ZIP_CHECKCONS"] = ZIP_CHECKCONS;
    zip["ZIP_TRUNCATE"]  = ZIP_TRUNCATE;
    zip["ZIP_RDONLY"]    = ZIP_RDONLY;

    zip["ZIP_FL_UNCHANGED"] = ZIP_FL_UNCHANGED;
}
