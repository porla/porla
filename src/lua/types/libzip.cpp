#include "../types.hpp"

#include <zip.h>

using porla::Lua::Types::Libzip;

class ZipArchive
{
public:
    explicit ZipArchive(zip_t* archive)
        : m_archive(archive)
    {
    }

    ~ZipArchive()
    {
        if (m_archive != nullptr)
        {
            zip_close(m_archive);
        }
    }

    void Close()
    {
        if (m_archive == nullptr)
        {
            return;
        }

        zip_close(m_archive);

        m_archive = nullptr;
    }

    sol::table List(sol::this_state L)
    {
        if (m_archive == nullptr)
        {
            return sol::lua_nil;
        }

        sol::table result(L, sol::create);

        for (zip_int64_t i = 0; i < zip_get_num_entries(m_archive, 0); i++)
        {
            const char* name = zip_get_name(m_archive, i, 0);

            if (!name)
            {
                continue;
            }

            result[i + 1] = std::string(name);
        }

        return result;
    }

    sol::object Read(sol::this_state L, const std::string& filename)
    {
        zip_file_t* file = zip_fopen(m_archive, filename.c_str(), 0);

        if (!file)
        {
            return sol::lua_nil;
        }

        zip_stat_t stat;
        zip_stat(m_archive, filename.c_str(), 0, &stat);

        std::string buffer(stat.size, '\0');
        zip_fread(file, buffer.data(), stat.size);
        zip_fclose(file);

        return sol::make_object(L, buffer);
    }

private:
    zip_t* m_archive = nullptr;
};

void Libzip::Register(sol::state& lua)
{
    sol::table libzip = lua["libzip"].valid()
        ? lua["libzip"].get<sol::table>()
        : lua.create_named_table("libzip");

    lua.new_usertype<ZipArchive>(
        "libzip.archive",
        sol::no_constructor,
        "close", &ZipArchive::Close,
        "list", &ZipArchive::List,
        "read", &ZipArchive::Read
    );

    libzip["open"] = [](const std::string& path)
    {
        int err;
        zip_t* archive = zip_open(path.c_str(), ZIP_RDONLY, &err);

        return std::make_shared<ZipArchive>(archive);
    };
}
