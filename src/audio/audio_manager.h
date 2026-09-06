#pragma once

#include <cstring>
#include <memory>
#include <string_view>
#include <vector>

#include <xaudio2.h>

#include "audio/track.h"
#include "resources/resource_loader.h"
#include "utils/com_release.h"
#include "utils/log.h"
#include "utils/string_map.h"

namespace ufps
{

class AudioManager
{
  public:
    AudioManager(ResourceLoader &resource_loader);

    auto play(std::string_view track_name) -> void;

  private:
    class VoiceCallback : public ::IXAudio2VoiceCallback
    {
      public:
        VoiceCallback(AudioManager &am)
            : am_{am}
        {
        }

        auto OnStreamEnd() -> void override
        {
        }

        auto OnVoiceProcessingPassEnd() -> void override
        {
        }

        auto OnVoiceProcessingPassStart(UINT32) -> void override
        {
        }

        auto OnBufferEnd(void *pBufferContext) -> void override
        {
            auto index = std::size_t{};
            std::memcpy(&index, &pBufferContext, sizeof(index));

            am_.voices_[index].is_free = true;
        }

        auto OnBufferStart(void *) -> void override
        {
        }

        auto OnLoopEnd(void *) -> void override
        {
        }

        auto OnVoiceError(void *, HRESULT) -> void override
        {
        }

      private:
        AudioManager &am_;
    };

    static constexpr auto destroy_voice = [](auto *obj) { obj->DestroyVoice(); };

    struct Voice
    {
        std::unique_ptr<::IXAudio2SourceVoice, decltype(destroy_voice)> voice;
        bool is_free = true;
    };

    VoiceCallback voice_callback_;
    StringMap<Track> tracks_;
    std::unique_ptr<::IXAudio2, ComRelease> xaudio_;
    std::unique_ptr<::IXAudio2MasteringVoice, decltype(destroy_voice)> mastering_voice_;
    std::array<Voice, 64zu> voices_;
};

}
