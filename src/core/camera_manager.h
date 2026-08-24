#pragma once

#include "core/camera.h"
#include "core/storage_manager.h"

namespace ufps
{

class CameraManager : public StorageManager<Camera>
{
  public:
    using handle_type = StorageManager<Camera>::handle_type;
};

using CameraHandle = CameraManager::handle_type;

}
