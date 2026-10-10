#include "torrentsqueueany.hpp"

#include "resolve.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsQueueAnyReq;
using porla::Rpc::Methods::Torrents::TorrentsQueueAnyRes;

#define QUEUE_IMPL(suffix, operation) \
    using porla::Rpc::Methods::Torrents::TorrentsQueue##suffix; \
    TorrentsQueue##suffix::TorrentsQueue##suffix(porla::Sessions& sessions) \
        : m_sessions(sessions) {} \
    void TorrentsQueue##suffix::Execute(const TorrentsQueueAnyReq &req, ResponseWriterHandle cb) \
    { \
        const auto resolved = ResolveTorrent(m_sessions, req.session_id, req.info_hash, cb); \
        if (!resolved) { return; } \
        resolved->torrent->status.handle.operation(); \
        cb->Ok(TorrentsQueueAnyRes{}); \
    }

QUEUE_IMPL(Bottom, queue_position_bottom)
QUEUE_IMPL(Down,   queue_position_down)
QUEUE_IMPL(Top,    queue_position_top)
QUEUE_IMPL(Up,     queue_position_up)
