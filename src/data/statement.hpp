#pragma once

#include <cstdint>

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include <sqlite3.h>

namespace porla::Data
{
    class Statement
    {
    public:
        class IRow
        {
        public:
            virtual std::vector<char> GetBuffer(const std::string& col) const = 0;
            virtual int GetInt32(const std::string& col) const = 0;
            virtual std::int64_t GetInt64(const std::string& col) const = 0;
            virtual std::optional<int> GetOptionalInt32(const std::string& col) const = 0;
            virtual std::optional<std::int64_t> GetOptionalInt64(const std::string& col) const = 0;
            virtual std::string GetStdString(const std::string& col) const = 0;
            virtual std::optional<std::string> GetOptionalStdString(const std::string& col) const = 0;
        };

        ~Statement();
        Statement(const Statement&) = delete;

        static Statement Prepare(sqlite3* db, std::string_view sql);

        Statement& Bind(const std::string& param, bool value);
        Statement& Bind(const std::string& param, int value);
        Statement& Bind(const std::string& param, std::int64_t value);
        Statement& Bind(const std::string& param, std::uint64_t value);
        Statement& Bind(const std::string& param, std::string_view value);
        Statement& Bind(const std::string& param, const char* value);
        Statement& Bind(const std::string& param, const std::vector<char>& buffer);
        Statement& Bind(const std::string& param, std::nullopt_t);

        template <typename T>
        Statement& Bind(const std::string& param, const std::optional<T>& value)
        {
            return value.has_value()
                ? Bind(param, *value)
                : Bind(param, std::nullopt);
        }

        void Execute();
        void Step(const std::function<int(const IRow&)>& cb);

    private:
        explicit Statement(sqlite3_stmt* stmt);

        Statement& Check(int res, const std::string& param);
        int Index(const std::string& param) const;

        sqlite3_stmt* m_stmt;
    };
}
