#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "presetsremove_reqres.hpp"

namespace porla::Rpc::Methods::Presets
{
    class PresetsRemove : public TypedMethod<PresetsRemoveReq, PresetsRemoveRes>
    {
    public:
        explicit PresetsRemove(sqlite3* db);

    protected:
        void Execute(const PresetsRemoveReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
    };
}
