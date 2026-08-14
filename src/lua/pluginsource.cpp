#include "pluginsource.hpp"

#include <fstream>

#include <boost/log/trivial.hpp>

#include "../zip.hpp"

using porla::Lua::PluginSource;

static std::optional<std::vector<char>> ReadFile(const fs::path& path)
{
    std::ifstream file(path, std::ios::binary);

    if (!file)
    {
        return std::nullopt;
    }

    return std::vector<char>(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>());
}

std::optional<PluginSource> PluginSource::Load(const fs::path& path)
{
    if (!path.is_absolute())
    {
        BOOST_LOG_TRIVIAL(error) << "Provided path is not absolute: " << path;
        return std::nullopt;
    }

    if (fs::is_regular_file(path) && path.extension() == ".lua")
    {
        const auto file_data = ReadFile(path);

        if (!file_data.has_value())
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to read file " << path;
            return std::nullopt;
        }

        const auto file_name = path.filename();

        return PluginSource{
            .entrypoint = file_name,
            .sources    = {
                {file_name, std::move(file_data.value())}
            }
        };
    }

    if (fs::is_regular_file(path) && path.extension() == ".zip")
    {
        auto archive_buffer = ReadFile(path);

        if (!archive_buffer.has_value())
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to load plugin sources from " << path;
            return std::nullopt;
        }

        return PluginSource{
            .entrypoint = "plugin.lua",
            .sources    = Zip::Load(*archive_buffer)
        };
    }

    if (fs::is_directory(path))
    {
        PluginSource source{
            .entrypoint = "plugin.lua",
            .sources    = {}
        };

        for (const auto entry : fs::recursive_directory_iterator(path))
        {
            if (!entry.is_regular_file())
            {
                continue;
            }

            const fs::path relative = fs::relative(entry.path(), path);
            const auto     contents = ReadFile(entry.path());

            if (!contents.has_value())
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to read file " << entry.path();
                continue;
            }

            source.sources.insert_or_assign(relative, contents.value());
        }

        return source;
    }

    return std::nullopt;
}
