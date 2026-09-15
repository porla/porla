#pragma once

#include "../../typedmethod.hpp"

#include "fsspace_reqres.hpp"

namespace porla::Rpc::Methods::Fs
{
    class FsSpace : public TypedMethod<FsSpaceReq, FsSpaceRes>
    {
    public:
        explicit FsSpace();

    protected:
        void Execute(const FsSpaceReq& req, ResponseWriterHandle cb) override;
    };
}
