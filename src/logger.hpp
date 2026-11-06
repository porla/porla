#pragma once

#include <array>
#include <concepts>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <format>
#include <iterator>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <boost/log/attributes/attribute_value_impl.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/program_options.hpp>

#define PORLA_LOG(lg, lvl, ...) ::porla::Log::Write((lg), ::porla::Log::lvl, __VA_ARGS__)

namespace porla::Log
{
    using Level = boost::log::trivial::severity_level;
    using enum boost::log::trivial::severity_level;

    using Logger = boost::log::sources::severity_channel_logger_mt<Level, std::string>;

    // store component in the Channel attribute
    BOOST_LOG_ATTRIBUTE_KEYWORD(component, "Channel", std::string)

    using Value      = std::variant<std::monostate, bool, std::int64_t, std::uint64_t, double, std::string>;
    using Properties = std::vector<std::pair<std::string, Value>>;

    Logger Component(const std::string& name);
    Logger Plugin(int plugin_id);

    void AddProperty(Logger& logger, const std::string& name, Value value);

    template <typename T>
    struct ValueOf;

    template <typename T>
    Value ToValue(const T& value);

    template <>
    struct ValueOf<std::filesystem::path>
    {
        static Value From(const std::filesystem::path& path) { return path.string(); }
    };

    template <typename T>
    struct ValueOf<std::optional<T>>
    {
        static Value From(const std::optional<T>& value) { return value ? ToValue(*value) : Value(); }
    };

    template <typename T>
    Value ToValue(const T& value)
    {
        if constexpr (std::same_as<T, bool>)                                { return value; }
        else if constexpr (std::signed_integral<T>)                         { return static_cast<std::int64_t>(value); }
        else if constexpr (std::unsigned_integral<T>)                       { return static_cast<std::uint64_t>(value); }
        else if constexpr (std::floating_point<T>)                          { return static_cast<double>(value); }
        else if constexpr (std::convertible_to<const T&, std::string_view>) { return std::string(std::string_view(value)); }
        else                                                                { return ValueOf<T>::From(value); }
    }

    template <typename T>
    auto Property(const std::string& name, const T& value)
    {
        return boost::log::add_value(name, ToValue(value));
    }

    namespace Detail
    {
        // Not constexpr, so a bad template fails to compile with this message in the error.
        inline void TemplateError(const char* message)
        {
            throw std::invalid_argument(message);
        }

        constexpr bool IsName(std::string_view name)
        {
            if (name.empty() || name[0] < 'a' || name[0] > 'z')
            {
                return false;
            }

            if (name == "ts" || name == "level" || name == "component" || name == "msg")
            {
                return false;
            }

            for (const char c : name)
            {
                if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'))
                {
                    return false;
                }
            }

            return true;
        }

        template <typename OnText, typename OnHole>
        constexpr void Parse(std::string_view text, OnText on_text, OnHole on_hole)
        {
            std::size_t start = 0;
            std::size_t i     = 0;

            while (i < text.size())
            {
                const bool doubled = i + 1 < text.size() && text[i + 1] == text[i];

                if ((text[i] == '{' || text[i] == '}') && doubled)
                {
                    on_text(text.substr(start, i + 1 - start));
                    i     += 2;
                    start  = i;
                    continue;
                }

                if (text[i] == '}')
                {
                    TemplateError("log template has a '}' without a '{'");
                }

                if (text[i] == '{')
                {
                    const auto close = text.find('}', i);

                    if (close == std::string_view::npos)
                    {
                        TemplateError("log template has a '{' without a '}'");
                    }

                    on_text(text.substr(start, i - start));
                    on_hole(text.substr(i + 1, close - i - 1));

                    i     = close + 1;
                    start = i;
                    continue;
                }

                i++;
            }

            on_text(text.substr(start));
        }
    }

    template <std::size_t N>
    class Template
    {
    public:
        template <typename S>
            requires std::convertible_to<const S&, std::string_view>
        consteval Template(const S& text)
            : m_text(text)
        {
            std::array<std::string_view, N> names{};
            std::size_t                     count = 0;

            Detail::Parse(
                m_text,
                [](std::string_view) {},
                [&](std::string_view name)
                {
                    if (!Detail::IsName(name))
                    {
                        Detail::TemplateError("log template holes must be snake_case names, like {torrent}, and not ts, level, component or msg");
                    }

                    if (count == N)
                    {
                        Detail::TemplateError("log template has more holes than arguments");
                    }

                    for (std::size_t i = 0; i < count; i++)
                    {
                        if (names[i] == name)
                        {
                            Detail::TemplateError("log template uses a hole name twice");
                        }
                    }

                    names[count++] = name;
                });

            if (count != N)
            {
                Detail::TemplateError("log template has fewer holes than arguments");
            }
        }

        constexpr std::string_view Text() const { return m_text; }

    private:
        std::string_view m_text;
    };

    template <typename... Args>
    void Write(Logger& logger, Level level, Template<sizeof...(Args)> tpl, const Args&... args)
    {
        auto rec = logger.open_record(boost::log::keywords::severity = level);

        if (!rec)
        {
            return;
        }

        std::array<Value, sizeof...(Args)> values{ ToValue(args)... };
        std::string                        message;
        std::size_t                        index = 0;

        Detail::Parse(
            tpl.Text(),
            [&](std::string_view text) { message.append(text); },
            [&](std::string_view name)
            {
                std::visit(
                    [&](const auto& v)
                    {
                        if constexpr (std::same_as<std::decay_t<decltype(v)>, std::monostate>)
                        {
                            message.append("null");
                        }
                        else
                        {
                            std::format_to(std::back_inserter(message), "{}", v);
                        }
                    },
                    values[index]);

                rec.attribute_values().insert(
                    boost::log::attribute_name(std::string(name)),
                    boost::log::attributes::make_attribute_value(std::move(values[index])));

                index++;
            });

        boost::log::record_ostream strm(rec);
        strm << message;
        strm.flush();

        logger.push_record(std::move(rec));
    }

    struct Entry
    {
        std::uint64_t              seq;
        std::int64_t               ts;
        Level                      level;
        std::optional<std::string> component;
        Properties                 properties;
        std::string                message;
    };

    struct Query
    {
        std::optional<std::uint64_t> after;
        std::optional<std::string>   component;
        std::size_t                  limit     = 500;
        Level                        min_level = trace;
        std::optional<int>           plugin_id;
    };

    struct Page
    {
        std::vector<Entry> entries;
        std::uint64_t      cursor;
        std::string        run_id;
    };

    class Buffer
    {
    public:
        static constexpr std::size_t MaxMessageSize = 8 * 1024;

        explicit Buffer(std::size_t capacity);

        void Push(Entry entry);
        Page List(const Query& query) const;

    private:
        std::size_t        m_capacity;
        std::deque<Entry>  m_entries;
        std::uint64_t      m_last_seq = 0;
        mutable std::mutex m_mutex;
        const std::string  m_run_id;
        std::size_t        m_size     = 0;
    };

    std::shared_ptr<Buffer> Setup(const boost::program_options::variables_map& cmd) noexcept;
}
