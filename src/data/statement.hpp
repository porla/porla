#pragma once

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
            virtual std::optional<int> GetOptionalInt32(const std::string& col) const = 0;
            virtual std::string GetStdString(const std::string& col) const = 0;
            virtual std::optional<std::string> GetOptionalStdString(const std::string& col) const = 0;
        };

        ~Statement();
        Statement(const Statement&) = delete;

        static Statement Prepare(sqlite3* db, const std::string_view& sql);

        Statement& Bind(const std::string& param, int value);
        Statement& Bind(const std::string& param, const std::optional<int>& value);
        Statement& Bind(const std::string& param, const std::string& value);
        Statement& Bind(const std::string& param, const std::optional<std::string>& value);
        Statement& Bind(const std::string& param, const std::vector<char>& buffer);

        void Execute();
        void Step(const std::function<int(const IRow&)>& cb);

    private:
        explicit Statement(sqlite3_stmt* stmt);

        sqlite3_stmt* m_stmt;
    };
}
