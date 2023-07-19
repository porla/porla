#include "crashpad.hpp"

#include <string>
#include <vector>

#include <boost/log/trivial.hpp>
#include <client/crashpad_client.h>
#include <client/settings.h>

void porla::MaybeStartCrashpad()
{
    base::FilePath database("crashpad");
    // Path to the out-of-process handler executable
    base::FilePath handler("");
    // URL used to submit minidumps to
    std::string url("");

    // Optional annotations passed via --annotations to the handler
    std::map<std::string, std::string> annotations;

    // Optional arguments to pass to the handler
    std::vector<std::string> arguments;

    crashpad::CrashpadClient client;

    bool success = client.StartHandler(
        handler,
        database,
        database,
        url,
        annotations,
        arguments,
        /* restartable */ true,
        /* asynchronous_start */ false);

    if (success)
    {
        BOOST_LOG_TRIVIAL(info) << "Crashpad started";
    }
}
