#pragma once

#include <filesystem>
#include <memory>
#include <optional>

#include <boost/signals2.hpp>

#include "../../typedmethod.hpp"

#include "mmdblookup_reqres.hpp"

namespace porla
{
    class Events;
    class KeyValue;
    class Mmdb;
}

namespace porla::Rpc::Methods::Mmdb
{
    class MmdbLookup : public TypedMethod<MmdbLookupReq, MmdbLookupRes>
    {
    public:
        explicit MmdbLookup(KeyValue& kv, Events& events);

    protected:
        void Execute(const MmdbLookupReq& req, ResponseWriterHandle cb) override;

    private:
        struct State;
        std::shared_ptr<State> m_state;
    };
}
