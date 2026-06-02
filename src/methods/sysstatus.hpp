#pragma once

#include <map>
#include <string>

#include <sqlite3.h>

#include "method.hpp"

namespace porla::Methods
{
    class SysStatus : public Method<json, std::map<std::string, json>>
    {
    public:
        explicit SysStatus(sqlite3* db);

    protected:
        bool CanInvoke(Token token) override
        {
            // This method can always be invoked.
            return true;
        }

        void Invoke(const json& req, WriteCb<std::map<std::string, json>> cb) override;

    private:
        sqlite3* m_db;
    };
}
