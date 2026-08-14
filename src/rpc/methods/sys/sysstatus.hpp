#pragma once

#include <map>
#include <string>

#include <sqlite3.h>

#include "../../typedmethod.hpp"

namespace porla::Rpc::Methods::Sys
{
    class SysStatus : public TypedMethod<nlohmann::json, std::map<std::string, nlohmann::json>>
    {
    public:
        explicit SysStatus(sqlite3* db);

    protected:
        bool CanInvoke(Token token) override
        {
            // This method can always be invoked.
            return true;
        }

        void Execute(const nlohmann::json& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
    };
}
