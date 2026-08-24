#pragma once

#include "core/storage_manager.h"
#include "graphics/point_light.h"

namespace ufps
{

class LightManager : public StorageManager<PointLight>
{
  public:
    using handle_type = StorageManager<PointLight>::handle_type;
};

using LightHandle = LightManager::handle_type;

}
