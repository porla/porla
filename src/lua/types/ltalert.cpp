#include "../types.hpp"

#include <libtorrent/alert.hpp>

using porla::Lua::Types::LtAlert;

void LtAlert::Register(sol::state& lua)
{
    sol::table lt = lua["lt"].valid()
        ? lua["lt"].get<sol::table>()
        : lua.create_named_table("lt");

    lt["alert_category_t"] = lua.new_usertype<lt::alert_category_t>(
        "lt.alert_category_t",
        sol::meta_function::bitwise_or, [](lt::alert_category_t a, lt::alert_category_t b) {
            return a | b;
        });

    lt["alert"] = lua.create_table();
    lt["alert"]["error_notification"]            = sol::var(lt::alert::error_notification);
    lt["alert"]["peer_notification"]             = sol::var(lt::alert::peer_notification);
    lt["alert"]["port_mapping_notification"]     = sol::var(lt::alert::port_mapping_notification);
    lt["alert"]["storage_notification"]          = sol::var(lt::alert::storage_notification);
    lt["alert"]["tracker_notification"]          = sol::var(lt::alert::tracker_notification);
    lt["alert"]["status_notification"]           = sol::var(lt::alert::status_notification);
    lt["alert"]["ip_block_notification"]         = sol::var(lt::alert::ip_block_notification);
    lt["alert"]["performance_warning"]           = sol::var(lt::alert::performance_warning);
    lt["alert"]["dht_notification"]              = sol::var(lt::alert::dht_notification);
    lt["alert"]["session_log_notification"]      = sol::var(lt::alert::session_log_notification);
    lt["alert"]["torrent_log_notification"]      = sol::var(lt::alert::torrent_log_notification);
    lt["alert"]["peer_log_notification"]         = sol::var(lt::alert::peer_log_notification);
    lt["alert"]["incoming_request_notification"] = sol::var(lt::alert::incoming_request_notification);
    lt["alert"]["dht_log_notification"]          = sol::var(lt::alert::dht_log_notification);
    lt["alert"]["dht_operation_notification"]    = sol::var(lt::alert::dht_operation_notification);
    lt["alert"]["port_mapping_log_notification"] = sol::var(lt::alert::port_mapping_log_notification);
    lt["alert"]["picker_log_notification"]       = sol::var(lt::alert::picker_log_notification);
    lt["alert"]["file_progress_notification"]    = sol::var(lt::alert::file_progress_notification);
    lt["alert"]["block_progress_notification"]   = sol::var(lt::alert::block_progress_notification);
    lt["alert"]["all_categories"]                = sol::var(lt::alert::all_categories);
}
