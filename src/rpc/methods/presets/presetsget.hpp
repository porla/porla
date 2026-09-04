#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"

#include "presetsget_reqres.hpp"

namespace porla::Rpc::Methods::Presets
{
    class PresetsGet : public TypedMethod<PresetsGetReq, PresetsGetRes>
    {
    public:
        explicit PresetsGet(sqlite3* db);

    protected:
        void Execute(const PresetsGetReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
    };
}
