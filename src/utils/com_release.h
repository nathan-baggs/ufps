#pragma once

namespace ufps
{

struct ComRelease
{
    template <class T>
    auto operator()(T *com_ptr) -> void
    {
        com_ptr->Release();
    }
};

}
