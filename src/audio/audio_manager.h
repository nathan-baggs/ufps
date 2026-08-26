#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include <xaudio2.h>

#include "audio/track.h"
#include "resources/resource_loader.h"
#include "utils/com_release.h"
#include "utils/string_map.h"

namespace ufps
{

class AudioManager
{
  public:
    AudioManager(ResourceLoader &resource_loader);

    auto play(std::string_view track_name) -> void;

  private:
    static constexpr auto destroy_voice = [](auto *obj) { obj->DestroyVoice(); };

    StringMap<Track> tracks_;
    std::unique_ptr<::IXAudio2, ComRelease> xaudio_;
    std::unique_ptr<::IXAudio2MasteringVoice, decltype(destroy_voice)> mastering_voice_;
    std::array<std::unique_ptr<::IXAudio2SourceVoice, decltype(destroy_voice)>, 64zu> voices_;
};

}
