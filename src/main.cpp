#include <windows.h>

#include <objbase.h>

#include "config.h"
#include "core/game.h"
#include "utils/exception.h"
#include "utils/log.h"
#include "utils/system_info.h"

int start()
{
    // Daz_Da_Cat: First stream done.
    // Daz_Da_Cat: You can't handle the Daz!
    ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    ufps::log::info(
        "μfps version: {}.{}.{}.{}",
        ufps::version::year,
        ufps::version::month,
        ufps::version::day,
        ufps::version::tweak);
    ufps::log::info("{}", ufps::system_info());

    auto game = ufps::Game{};

    game.run();

    return 0;
}

int main()
{
    try
    {
        return start();
    }
    catch (const ufps::Exception &e)
    {
        ufps::log::error("{}", e);
        return -1;
    }
    catch (const std::exception &e)
    {
        ufps::log::error("{}", e.what());
        return -1;
    }
    catch (...)
    {
        ufps::log::error("unhandled unknown exception");
        return -1;
    }
}
