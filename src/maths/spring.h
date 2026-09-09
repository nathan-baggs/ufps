#pragma once

#include <chrono>
#include <cmath>

#include "core/clock.h"

namespace ufps
{

class Spring
{
  public:
    struct State
    {
        float position;
        float velocity;
    };

    enum class DampingMode
    {
        OVER,
        CRITICAL,
        UNDER,
    };

    constexpr Spring(
        float position,
        float velocity,
        float equilibrium_position,
        float angular_frequency,
        DampingMode damping_mode,
        float damping_ratio);

    constexpr auto add_position(float pos_delta) -> void;

    constexpr auto update(Duration delta) -> State;

  private:
    State state_;
    float equilibrium_position_;
    float angular_frequency_;
    DampingMode damping_mode_;
    float damping_ratio_;
    float pos_pos_coef_;
    float pos_vel_coef_;
    float vel_pos_coef_;
    float vel_vel_coef_;
};

constexpr Spring::Spring(
    float position,
    float velocity,
    float equilibrium_position,
    float angular_frequency,
    DampingMode damping_mode,
    float damping_ratio)
    : state_{.position = position, .velocity = velocity}
    , equilibrium_position_{equilibrium_position}
    , angular_frequency_{angular_frequency}
    , damping_mode_{damping_mode}
    , damping_ratio_{damping_ratio}
    , pos_pos_coef_{1.0f}
    , pos_vel_coef_{}
    , vel_pos_coef_{}
    , vel_vel_coef_{1.0f}
{
    contract_assert(damping_ratio_ >= 0.0f);
    contract_assert(angular_frequency_ >= 0.0f);

    if (damping_mode_ == DampingMode::OVER)
    {
        contract_assert(damping_ratio_ > 1.0f);
    }
    else if (damping_mode_ == DampingMode::UNDER)
    {
        contract_assert(damping_ratio_ < 1.0f);
    }
}

constexpr auto Spring::add_position(float pos_delta) -> void
{
    state_.position += pos_delta;
}

constexpr auto Spring::update(Duration delta) -> State
{
    const auto delta_time = std::chrono::duration_cast<std::chrono::duration<float>>(delta).count();

    switch (damping_mode_)
    {
        using enum DampingMode;

        case OVER:
        {
            const auto za = -angular_frequency_ * damping_ratio_;
            const auto zb = angular_frequency_ * std::sqrt(damping_ratio_ * damping_ratio_ - 1.0f);
            const auto z1 = za - zb;
            const auto z2 = za + zb;

            const auto e1 = std::exp(z1 * delta_time);
            const auto e2 = std::exp(z2 * delta_time);

            const auto invTwoZb = 1.0f / (2.0f * zb);

            const auto e1_Over_TwoZb = e1 * invTwoZb;
            const auto e2_Over_TwoZb = e2 * invTwoZb;

            const auto z1e1_Over_TwoZb = z1 * e1_Over_TwoZb;
            const auto z2e2_Over_TwoZb = z2 * e2_Over_TwoZb;

            pos_pos_coef_ = e1_Over_TwoZb * z2 - z2e2_Over_TwoZb + e2;
            pos_vel_coef_ = -e1_Over_TwoZb + e2_Over_TwoZb;

            vel_pos_coef_ = (z1e1_Over_TwoZb - z2e2_Over_TwoZb + e2) * z2;
            vel_vel_coef_ = -z1e1_Over_TwoZb + z2e2_Over_TwoZb;

            break;
        };
        case CRITICAL:
        {
            const auto expTerm = std::exp(-angular_frequency_ * delta_time);
            const auto timeExp = delta_time * expTerm;
            const auto timeExpFreq = timeExp * angular_frequency_;

            pos_pos_coef_ = timeExpFreq + expTerm;
            pos_vel_coef_ = timeExp;

            vel_pos_coef_ = -angular_frequency_ * timeExpFreq;
            vel_vel_coef_ = -timeExpFreq + expTerm;

            break;
        };
        case UNDER:
        {
            const auto omegaZeta = angular_frequency_;
            const auto alpha = angular_frequency_ * std::sqrt(1.0f - damping_ratio_ * damping_ratio_);

            const auto expTerm = std::exp(-omegaZeta * delta_time);
            const auto cosTerm = std::cos(alpha * delta_time);
            const auto sinTerm = std::sin(alpha * delta_time);

            const auto invAlpha = 1.0f / alpha;

            const auto expSin = expTerm * sinTerm;
            const auto expCos = expTerm * cosTerm;
            const auto expOmegaZetaSin_Over_Alpha = expTerm * omegaZeta * sinTerm * invAlpha;

            pos_pos_coef_ = expCos + expOmegaZetaSin_Over_Alpha;
            pos_vel_coef_ = expSin * invAlpha;

            vel_pos_coef_ = -expSin * alpha - omegaZeta * expOmegaZetaSin_Over_Alpha;
            vel_vel_coef_ = expCos - expOmegaZetaSin_Over_Alpha;

            break;
        };
    }

    const auto old_pos = state_.position - equilibrium_position_;
    const auto old_vel = state_.velocity;

    state_.position = old_pos * pos_pos_coef_ + old_vel * pos_vel_coef_ + equilibrium_position_;
    state_.velocity = old_pos * vel_pos_coef_ + old_vel * vel_vel_coef_;

    return state_;
}

}
