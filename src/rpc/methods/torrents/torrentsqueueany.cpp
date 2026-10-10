#include "torrentsqueueany.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsQueueAnyReq;
using porla::Rpc::Methods::Torrents::TorrentsQueueAnyRes;

#define QUEUE_IMPL(suffix, operation) \
    using porla::Rpc::Methods::Torrents::TorrentsQueue##suffix; \
    TorrentsQueue##suffix::TorrentsQueue##suffix(sqlite3* db, porla::Sessions& sessions) \
        : m_db(db) , m_sessions(sessions) {} \
    void TorrentsQueue##suffix::Execute(const TorrentsQueueAnyReq &req, ResponseWriterHandle cb) \
    { \
        const auto session = req.session_id.has_value() \
            ? Data::Models::Sessions::GetById(m_db, req.session_id.value()) \
            : Data::Models::Sessions::GetDefault(m_db); \
        if (!session) { return cb->Error(-1, "Session not found"); } \
        const auto& session_state = m_sessions.Get(session->id); \
        if (session_state == nullptr) { return cb->Error(-2, "Session not loaded"); } \
        const auto& it = session_state->Torrents().find(req.info_hash); \
        if (it == session_state->Torrents().end()) { return cb->Error(-3, "Torrent not found in session"); } \
        if (!it->second.status.handle.is_valid()) { return cb->Error(-4, "Torrent not valid"); } \
        it->second.status.handle.operation(); \
        cb->Ok(TorrentsQueueAnyRes{}); \
    }

QUEUE_IMPL(Bottom, queue_position_bottom)
QUEUE_IMPL(Down,   queue_position_down)
QUEUE_IMPL(Top,    queue_position_top)
QUEUE_IMPL(Up,     queue_position_up)
