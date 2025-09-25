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
            virtual std::vector<char> GetBuffer(int index) const = 0;
            virtual int GetInt32(int index) const = 0;
            virtual std::optional<int> GetOptionalInt32(int index) const = 0;
            virtual std::string GetStdString(int index) const = 0;
        };

        ~Statement();
        Statement(const Statement&) = delete;

        static Statement Prepare(sqlite3* db, const std::string_view& sql);

        Statement& Bind(int pos, int value);
        Statement& Bind(int pos, const std::optional<int>& value);
        Statement& Bind(int pos, const std::string_view& value);
        Statement& Bind(int pos, const std::optional<std::string_view>& value);
        Statement& Bind(int pos, const std::vector<char>& buffer);

        void Execute();
        void Step(const std::function<int(const IRow&)>& cb);

    private:
        explicit Statement(sqlite3_stmt* stmt);

        sqlite3_stmt* m_stmt;
    };
}
