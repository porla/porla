#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "torrentsqueueany_reqres.hpp"

namespace porla
{
    class Sessions;
}

#define QUEUE_CLASS(suffix) \
    namespace porla::Rpc::Methods::Torrents \
    { \
        class TorrentsQueue##suffix : public TypedMethod<TorrentsQueueAnyReq, TorrentsQueueAnyRes> \
        { \
        public: \
            explicit TorrentsQueue##suffix(sqlite3* db, porla::Sessions& sessions); \
        protected: \
            void Execute(const TorrentsQueueAnyReq& req, ResponseWriterHandle cb) override; \
        private: \
            sqlite3* m_db; \
            porla::Sessions& m_sessions; \
        }; \
    }

QUEUE_CLASS(Bottom)
QUEUE_CLASS(Down)
QUEUE_CLASS(Top)
QUEUE_CLASS(Up)

