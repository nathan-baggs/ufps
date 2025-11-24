#include <windows.h>

#include "config.h"
#include "utils/formatter.h"
#include "utils/hello.h"
#include "utils/log.h"
#include "utils/system_info.h"

int main()
{
    ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    ufps::log::info("μfps version: {}.{}.{}", ufps::version::major, ufps::version::minor, ufps::version::patch);
    ufps::log::info("{}", ufps::system_info());

    return 0;
}
