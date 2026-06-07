#pragma once

namespace ufps
{

class PhysicsSystem
{
  public:
    PhysicsSystem();
    ~PhysicsSystem() = default;
    PhysicsSystem(const PhysicsSystem &) = delete;
    auto operator=(const PhysicsSystem &) -> PhysicsSystem & = delete;
    PhysicsSystem(PhysicsSystem &&) = delete;
    auto operator=(PhysicsSystem &&) -> PhysicsSystem & = delete;

    auto update() -> void;

  private:
};

}
