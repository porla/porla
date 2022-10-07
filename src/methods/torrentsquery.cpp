#include "torrentsquery.hpp"

#include <boost/log/trivial.hpp>
#include <nlohmann/json.hpp>

#include "../session.hpp"

using json = nlohmann::json;

using porla::Methods::TorrentsQuery;
using porla::Methods::TorrentsQueryRes;

TorrentsQuery::TorrentsQuery(std::string const& path, ISession& session)
    : Method(boost::beast::http::verb::get, path)
    , m_session(session)
{
}

void TorrentsQuery::Invoke(WriteCb<TorrentsQueryRes> cb)
{
    TorrentsQueryRes res;

    m_session.Query(
        "select info_hash,name from torrents",
        [&res](sqlite3_stmt* stmt)
        {
            json j;

            for (int i = 0; i < sqlite3_column_count(stmt); i++)
            {
                std::string columnName = sqlite3_column_name(stmt, i);

                switch (sqlite3_column_type(stmt, i))
                {
                case SQLITE_INTEGER:
                    j[columnName] = sqlite3_column_int64(stmt, i);
                    continue;
                case SQLITE_TEXT:
                    j[columnName] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                    continue;
                default:
                    BOOST_LOG_TRIVIAL(warning) << "Unknown column type: " << sqlite3_column_type(stmt, i);
                    continue;
                }
            }

            res.results.push_back(j);

            return 0;
        });

    cb(res);
}
