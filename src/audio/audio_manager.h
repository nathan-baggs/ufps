#pragma once

#include <vector>

#include "audio/track.h"
#include "resources/resource_loader.h"
#include "utils/string_map.h"

namespace ufps
{

class AudioManager
{
  public:
    AudioManager(ResourceLoader &resource_loader);

  private:
    StringMap<Track> tracks_;
};

}
