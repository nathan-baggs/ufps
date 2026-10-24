#pragma once

#include "graphics/colour.h"
#include "maths/bounded_number.h"

namespace ufps
{

struct ToneMapOptions
{
    BoundedFloat<0.0f, 100.0f> max_brightness = 5.0f;
    BoundedFloat<0.0f, 50.0f> contrast = 1.0f;
    BoundedFloat<0.0f, 1.0f> linear_section_start = 0.3f;
    BoundedFloat<0.0f, 1.0f> linear_section_length = 0.5f;
    BoundedFloat<0.0f, 3.0f> black_tightness = 0.0f;
    BoundedFloat<0.0f, 1.0f> pedestal = 0.0f;
    BoundedFloat<0.0f, 5.0f> gamma = 2.2f;
};

struct SSAOOptions
{
    bool enabled = true;
    BoundedUint32<1u, 64u> sample_count = 64u;
    BoundedFloat<0.1f, 2.0f> radius = 0.75f;
    BoundedFloat<0.01f, 0.1f> bias = 0.025f;
    BoundedFloat<1.0f, 4.0f> power = 2.0f;
};

struct ExposureOptions
{
    BoundedFloat<-10.0f, 10.0f> min_log_luminance = -3.0f;
    BoundedFloat<-10.0f, 10.0f> max_log_luminance = 1.0f;
};

struct FogOptions
{
    Colour colour = colours::black;
    BoundedFloat<0.0f, 0.2f> density = 0.005f;
};

struct ChromaticAberrationOptions
{
    BoundedFloat<-0.1f, 0.1f> red_offset = 0.009f;
    BoundedFloat<-0.1f, 0.1f> green_offset = 0.006f;
    BoundedFloat<-0.1f, 0.1f> blue_offset = -0.006f;
    BoundedFloat<0.0f, 1.0f> strength = 0.5f;
};

struct VignetteOptions
{
    Colour colour = colours::black;
    BoundedFloat<0.0f, 1.0f> strength = 0.5f;
    BoundedFloat<0.0f, 1.0f> feather = 0.1f;
};

struct FilmGrainOptions
{
    BoundedFloat<0.0f, 1.0f> strength = 0.01f;
};

struct BloomOptions
{
    BoundedFloat<0.0f, 0.1f> filter_radius = 0.005f;
    BoundedFloat<0.0f, 1.0f> mix_amount = 0.04f;
    BoundedFloat<0.0f, 10.0f> threshold = 1.0f;
};

struct PostProcessingOptions
{
    ToneMapOptions tone_map_options;
    SSAOOptions ssao_options;
    ExposureOptions exposure_options;
    FogOptions fog_options;
    ChromaticAberrationOptions chromatic_aberration_options;
    VignetteOptions vignette_options;
    FilmGrainOptions film_grain_options;
    BloomOptions bloom_options;
};

}
