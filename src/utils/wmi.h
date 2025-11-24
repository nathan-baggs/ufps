#pragma once

#include <memory>
#include <string>
#include <vector>

#include <comdef.h>
#include <wbemidl.h>
#include <windows.h>

#include "utils/com_release.h"

namespace ufps
{

class Wmi
{
  public:
    Wmi();
    auto query(const std::string &wql, const std::string &property_name) const -> std::vector<std::string>;

  private:
    std::unique_ptr<::IWbemLocator, ComRelease> locator_;
    std::unique_ptr<::IWbemServices, ComRelease> services_;
};

}
