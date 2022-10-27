#include "sessionparams.hpp"

#include <libtorrent/session.hpp>

#include "../statement.hpp"

using porla::Data::Models::SessionParams;

void SessionParams::Insert(sqlite3 *db, const libtorrent::session_params &params)
{
    std::vector<char> buf = lt::write_session_params_buf(
        params,
        lt::session::save_dht_state);

    Statement::Prepare(db, "INSERT INTO sessionparams (data) VALUES ($1);")
        .Bind(1, buf)
        .Execute();
}
