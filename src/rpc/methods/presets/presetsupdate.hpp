#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"

#include "presetsupdate_reqres.hpp"

namespace porla::Rpc::Methods::Presets
{
    class PresetsUpdate : public TypedMethod<PresetsUpdateReq, PresetsUpdateRes>
    {
    public:
        explicit PresetsUpdate(sqlite3* db);

    protected:
        void Execute(const PresetsUpdateReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
    };
}
