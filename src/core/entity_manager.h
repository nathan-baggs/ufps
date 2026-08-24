#pragma once

#include "core/entity.h"
#include "core/storage_manager.h"

namespace ufps
{

class EntityManager : public StorageManager<Entity>
{
  public:
    using handle_type = StorageManager<Entity>::handle_type;
};

using EntityHandle = EntityManager::handle_type;

}
