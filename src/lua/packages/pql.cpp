#include "pql.hpp"

#include <boost/log/trivial.hpp>

#include "../../query/pql.hpp"

using porla::Lua::Packages::PQL;

void PQL::Register(sol::state& lua)
{
    auto pql_type = lua.new_usertype<porla::Query::PQL::Filter>(
        "porla.PQL.Filter",
        sol::no_constructor,
        "includes", &porla::Query::PQL::Filter::Includes);

    lua["package"]["preload"]["pql"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table pql = lua.create_table();

        pql["parse"] = [](const std::string& input)
        {
            return porla::Query::PQL::Parse(input);
        };

        return pql;
    };
}
