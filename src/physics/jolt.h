#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#if !defined(JPH_DEBUG_RENDERER)
#define JPH_DEBUG_RENDERER
#endif

#include <Jolt/Jolt.h>

#include <Jolt/Core/Core.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystem.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Math/Quat.h>
#include <Jolt/Math/Real.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/ObjectVsBroadPhaseLayerFilterMask.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/ObjectLayerPairFilterMask.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/EActivation.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Renderer/DebugRenderer.h>
#include <Jolt/Renderer/DebugRendererSimple.h>

#pragma GCC diagnostic pop

#include "graphics/colour.h"
#include "maths/quaternion.h"
#include "maths/vector3.h"

namespace ufps
{

inline auto to_native(const ::JPH::Vec3 &vec) -> Vector3
{
    return {vec.GetX(), vec.GetY(), vec.GetZ()};
}

inline auto to_native(const ::JPH::Color &c) -> Colour
{
    return {c.r / 255.0f, c.g / 255.0f, c.b / 255.0f};
}

inline auto to_jolt(ufps::Vector3 vec) -> ::JPH::Vec3
{
    return {vec.x, vec.y, vec.z};
}

inline auto to_jolt(ufps::Quaternion q) -> ::JPH::Quat
{
    return {q.x, q.y, q.z, q.w};
}

}
