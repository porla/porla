#include "logger.hpp"
 
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <format>
#include <iostream>
#include <random>

#include <boost/core/null_deleter.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/attributes/constant.hpp>
#include <boost/log/attributes/clock.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sinks/unlocked_frontend.hpp>
#include <boost/log/utility/exception_handler.hpp>
 
namespace porla::Log
{
    namespace
    {
        BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)

        Properties PropertiesOf(const boost::log::record_view& rec)
        {
            Properties properties;

            for (const auto& [name, value] : rec.attribute_values())
            {
                if (const auto v = value.extract<Value>())
                {
                    properties.emplace_back(name.string(), v.get());
                }
            }

            std::ranges::sort(properties, {}, &Properties::value_type::first);

            return properties;
        }

        void WriteValue(boost::log::formatting_ostream& os, std::string_view value)
        {
            const bool quote = value.empty() || std::ranges::any_of(value, [](unsigned char c)
            {
                return c <= ' ' || c == '=' || c == '"' || c == 0x7f;
            });

            if (!quote)
            {
                os << value;
                return;
            }

            os << '"';

            for (const unsigned char c : value)
            {
                switch (c)
                {
                    case '"':  os << "\\\""; break;
                    case '\\': os << "\\\\"; break;
                    case '\n': os << "\\n"; break;
                    case '\r': os << "\\r"; break;
                    case '\t': os << "\\t"; break;

                    default:
                        if (c < 0x20 || c == 0x7f)
                        {
                            os << std::format("\\u{:04x}", c);
                        }
                        else
                        {
                            os << static_cast<char>(c);
                        }
                }
            }

            os << '"';
        }

        void WriteValue(boost::log::formatting_ostream& os, const Value& value)
        {
            std::visit(
                [&os](const auto& v)
                {
                    if constexpr (std::same_as<std::decay_t<decltype(v)>, std::monostate>)
                    {
                        os << "null";
                    }
                    else if constexpr (std::same_as<std::decay_t<decltype(v)>, std::string>)
                    {
                        WriteValue(os, std::string_view(v));
                    }
                    else
                    {
                        os << std::format("{}", v);
                    }
                },
                value);
        }

        void Format(const boost::log::record_view& rec, boost::log::formatting_ostream& os)
        {
            const auto ts      = rec[timestamp];
            const auto level   = rec[boost::log::trivial::severity];
            const auto comp    = rec[component];
            const auto message = rec[boost::log::expressions::smessage];

            if (ts)
            {
                const auto date = ts.get().date();
                const auto time = ts.get().time_of_day();

                os << std::format(
                    "ts={:04}-{:02}-{:02}T{:02}:{:02}:{:02}.{:03}Z ",
                    static_cast<int>(date.year()),
                    static_cast<int>(date.month()),
                    static_cast<int>(date.day()),
                    time.hours(),
                    time.minutes(),
                    time.seconds(),
                    time.total_milliseconds() % 1000);
            }

            os << "level=" << (level ? boost::log::trivial::to_string(level.get()) : "info");

            if (comp)
            {
                os << " component=";
                WriteValue(os, std::string_view(comp.get()));
            }

            os << " msg=";
            WriteValue(os, message ? std::string_view(message.get()) : std::string_view());

            for (const auto& [name, value] : PropertiesOf(rec))
            {
                os << ' ' << name << '=';
                WriteValue(os, value);
            }
        }

        class BufferBackend : public boost::log::sinks::basic_sink_backend<boost::log::sinks::concurrent_feeding>
        {
        public:
            explicit BufferBackend(std::shared_ptr<Buffer> buffer)
                : m_buffer(std::move(buffer))
            {
            }

            void consume(const boost::log::record_view& rec)
            {
                static const boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));

                const auto ts      = rec[timestamp];
                const auto level   = rec[boost::log::trivial::severity];
                const auto comp    = rec[component];
                const auto message = rec[boost::log::expressions::smessage];

                m_buffer->Push(Entry{
                    .seq        = 0,
                    .ts         = ts ? (ts.get() - epoch).total_milliseconds() : 0,
                    .level      = level ? level.get() : info,
                    .component  = comp ? std::optional(comp.get()) : std::nullopt,
                    .properties = PropertiesOf(rec),
                    .message    = message ? message.get() : std::string()
                });
            }

        private:
            std::shared_ptr<Buffer> m_buffer;
        };

        std::size_t SizeOf(const Entry& entry)
        {
            std::size_t size = sizeof(Entry) + entry.message.size();

            for (const auto& [name, value] : entry.properties)
            {
                size += sizeof(Properties::value_type) + name.size();

                if (const auto* str = std::get_if<std::string>(&value))
                {
                    size += str->size();
                }
            }

            return size;
        }

        std::string MakeRunId()
        {
            std::random_device                           rd;
            std::uniform_int_distribution<std::uint64_t> dist;

            char buf[17];
            std::snprintf(buf, sizeof(buf), "%016llx", static_cast<unsigned long long>(dist(rd)));

            return buf;
        }
    }

    void SetProperty(Logger& logger, const std::string& name, Value value)
    {
        const boost::log::attributes::constant<Value> attr(std::move(value));

        if (auto [it, added] = logger.add_attribute(name, attr); !added)
        {
            logger.remove_attribute(it);
            logger.add_attribute(name, attr);
        }
    }

    Logger Component(const std::string& name)
    {
        return Logger(boost::log::keywords::channel = name);
    }

    Logger Plugin(int id)
    {
        auto logger = Component("plugin");
        SetProperty(logger, "plugin_id", std::int64_t{ id });
        return logger;
    }

    Buffer::Buffer(std::size_t capacity)
        : m_capacity(capacity)
        , m_run_id(MakeRunId())
    {
    }

    void Buffer::Push(Entry entry)
    {
        if (entry.message.size() > MaxMessageSize)
        {
            entry.message.resize(MaxMessageSize);
        }

        for (auto& [name, value] : entry.properties)
        {
            if (auto* str = std::get_if<std::string>(&value); str != nullptr && str->size() > MaxMessageSize)
            {
                str->resize(MaxMessageSize);
            }
        }

        std::lock_guard lock(m_mutex);

        entry.seq = ++m_last_seq;

        m_size += SizeOf(entry);
        m_entries.push_back(std::move(entry));

        while (m_size > m_capacity && m_entries.size() > 1)
        {
            m_size -= SizeOf(m_entries.front());
            m_entries.pop_front();
        }
    }

    Page Buffer::List(const Query& query) const
    {
        const auto matches = [&query](const Entry& entry)
        {
            if (entry.level < query.min_level)
            {
                return false;
            }

            if (query.component.has_value() && entry.component != query.component)
            {
                return false;
            }

            if (query.plugin_id.has_value())
            {
                const auto it = std::ranges::find(entry.properties, "plugin_id", &Properties::value_type::first);

                return it != entry.properties.end() && it->second == Value(std::int64_t{ *query.plugin_id });
            }

            return true;
        };

        std::lock_guard lock(m_mutex);

        Page page{ .entries = {}, .cursor = m_last_seq, .run_id = m_run_id };

        if (!query.after.has_value())
        {
            for (auto it = m_entries.rbegin(); it != m_entries.rend() && page.entries.size() < query.limit; ++it)
            {
                if (matches(*it)) { page.entries.push_back(*it); }
            }

            std::reverse(page.entries.begin(), page.entries.end());

            return page;
        }

        if (*query.after >= m_last_seq || m_entries.empty())
        {
            return page;
        }

        const auto first = m_entries.front().seq;
        const auto skip  = *query.after < first ? 0 : *query.after - first + 1;

        for (auto it = m_entries.begin() + static_cast<std::ptrdiff_t>(skip); it != m_entries.end(); ++it)
        {
            if (!matches(*it)) { continue; }

            page.entries.push_back(*it);

            if (page.entries.size() == query.limit)
            {
                page.cursor = it->seq;
                break;
            }
        }

        return page;
    }

    std::shared_ptr<Buffer> Setup(const boost::program_options::variables_map& cmd) noexcept
    {
        std::string configured_level;

        if (auto env_level = std::getenv("PORLA_LOG_LEVEL"))
        {
            configured_level = env_level;
        }

        if (cmd.count("log-level"))
        {
            configured_level = cmd["log-level"].as<std::string>();
        }

        boost::log::trivial::severity_level log_level = boost::log::trivial::info;

        if (configured_level == "trace")   { log_level = boost::log::trivial::severity_level::trace; }
        if (configured_level == "debug")   { log_level = boost::log::trivial::severity_level::debug; }
        if (configured_level == "info")    { log_level = boost::log::trivial::severity_level::info; }
        if (configured_level == "warning") { log_level = boost::log::trivial::severity_level::warning; }
        if (configured_level == "error")   { log_level = boost::log::trivial::severity_level::error; }
        if (configured_level == "fatal")   { log_level = boost::log::trivial::severity_level::fatal; }

        auto buffer  = std::make_shared<Buffer>(4 * 1024 * 1024);
        auto console = boost::make_shared<boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend>>();

        console->locked_backend()->add_stream(boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));
        console->locked_backend()->auto_flush(true);
        console->locked_backend()->set_auto_newline_mode(boost::log::sinks::auto_newline_mode::always_insert);
        console->set_formatter(&Format);

        auto core = boost::log::core::get();

        core->add_global_attribute("TimeStamp", boost::log::attributes::utc_clock());

        // Logging from destructors and catch blocks must not throw, e.g. when the buffer can't allocate.
        core->set_exception_handler(boost::log::make_exception_suppressor());
        core->add_sink(console);
        core->add_sink(boost::make_shared<boost::log::sinks::unlocked_sink<BufferBackend>>(boost::make_shared<BufferBackend>(buffer)));
        core->set_filter(boost::log::trivial::severity >= log_level);

        return buffer;
    }
}
