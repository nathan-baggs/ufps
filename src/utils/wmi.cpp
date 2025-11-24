#include "wmi.h"

#include <memory>
#include <string>
#include <vector>

#include <comdef.h>
#include <wbemidl.h>
#include <windows.h>

#include "utils/error.h"
#include "utils/text_utils.h"

namespace
{

struct AutoVariant
{
    ~AutoVariant()
    {
        ::VariantClear(&var);
    }

    VARIANT &var;
};
}

namespace ufps
{

Wmi::Wmi()
    : locator_{}
    , services_{}
{
    ensure(
        ::CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, std::out_ptr(locator_)) ==
            S_OK,
        "failed to create locator");

    ensure(
        locator_->ConnectServer(_bstr_t(L"root\\CIMV2"), nullptr, nullptr, 0, 0, 0, 0, std::out_ptr(services_)) == S_OK,
        "failed to create services");

    ensure(
        ::CoSetProxyBlanket(
            services_.get(),
            RPC_C_AUTHN_WINNT,
            RPC_C_AUTHZ_NONE,
            nullptr,
            RPC_C_AUTHN_LEVEL_CALL,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            nullptr,
            EOAC_NONE) == S_OK,
        "failed to set blanket");
}

auto Wmi::query(const std::string &wql, const std::string &property_name) const -> std::vector<std::string>
{
    auto enumerator = std::unique_ptr<::IEnumWbemClassObject, ComRelease>{};
    const auto wql_wide = text_widen(wql);
    ensure(
        services_->ExecQuery(
            _bstr_t(L"WQL"),
            _bstr_t(wql_wide.c_str()),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            nullptr,
            std::out_ptr(enumerator)) == S_OK,
        "failed to execute query: {}",
        wql);

    auto obj = std::unique_ptr<::IWbemClassObject, ComRelease>{};
    ::ULONG ret{};

    const auto property_name_wide = text_widen(property_name);
    auto values = std::vector<std::string>{};

    while (enumerator)
    {
        if (enumerator->Next(static_cast<long>(WBEM_INFINITE), 1, std::out_ptr(obj), &ret) != S_OK)
        {
            break;
        }

        {
            auto prop = ::VARIANT{};
            auto auto_var = AutoVariant{prop};

            if (obj->Get(property_name_wide.c_str(), 0, &prop, 0, 0) == S_OK)
            {
                const auto wstr = std::wstring(prop.bstrVal, ::SysStringLen(prop.bstrVal));
                values.push_back(text_narrow(wstr));
            }
        }
    }

    return values;
}

}
