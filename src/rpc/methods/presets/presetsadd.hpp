#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"

#include "presetsadd_reqres.hpp"

namespace porla::Rpc::Methods::Presets
{
    class PresetsAdd : public TypedMethod<PresetsAddReq, PresetsAddRes>
    {
    public:
        explicit PresetsAdd(sqlite3* db);

    protected:
        void Execute(const PresetsAddReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
    };
}
