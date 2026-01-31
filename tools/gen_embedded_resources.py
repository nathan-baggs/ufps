#!/usr/bin/env python

import os
from pathlib import Path

cpp_template: str = """\
#include "resources/embedded_resource_loader.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>

#include "resources/resource_loader.h"
#include "utils/data_buffer.h"
#include "utils/error.h"

namespace
{{

{0}

template <class T>
auto to_container(std::span<const std::uint8_t> data) -> T
{{
    static_assert(sizeof(typename T::value_type) == 1);

    const auto *ptr = reinterpret_cast<const T::value_type *>(data.data());
    return T{{ptr, ptr + data.size()}};
}}
}}

namespace ufps
{{

EmbeddedResourceLoader::EmbeddedResourceLoader()
{{
    lookup_ = {{
        {1}
    }};
}}

auto EmbeddedResourceLoader::load_string(std::string_view name) -> std::string
{{
    const auto resource = lookup_.find(name);
    expect(resource != std::ranges::cend(lookup_), "resource {{}} does not exist", name);

    return to_container<std::string>(resource->second);
}}

auto EmbeddedResourceLoader::load_data_buffer(std::string_view name) -> DataBuffer
{{
    const auto resource = lookup_.find(name);
    expect(resource != std::ranges::cend(lookup_), "resource {{}} does not exist", name);

    return to_container<DataBuffer>(resource->second);
}}
}}
"""

constexpr_template: str = """\
constexpr const std::uint8_t {0}[] = {{
#embed "../../{1}"
}};
"""

lookup_entry_template: str = '        {{"{0}", {1}}},'

if __name__ == "__main__":
    paths: list[str] = ["assets", "secret-assets"]
    asset_paths: list[str] = ["textures", "models", "shaders", "scripts", "sounds"]

    constexprs: list[str] = []
    lookups: list[str] = []

    for path in paths:
        for asset_path in asset_paths:
            root_path: str = os.path.join(path, asset_path)
            for asset in os.listdir(root_path):
                full_path: os.PathLike = Path(os.path.join(root_path, asset))
                relative_path: os.PathLike = Path(os.path.join(asset_path, asset))

                var_name: str = full_path.stem
                if asset_path == "shaders":
                    var_name += f"_{full_path.suffix[1:]}"
                if os.path.isfile(full_path):
                    if asset.startswith("."):
                        continue
                    constexprs.append(constexpr_template.format(var_name, full_path))
                    lookups.append(
                        lookup_entry_template.format(
                            str(relative_path).replace("/", "\\\\"), var_name
                        )
                    )

    print(cpp_template.format("\n\n".join(constexprs), "\n".join(lookups)))
