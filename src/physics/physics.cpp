#include <physics/physics.hpp>
#include <logger.hpp>
#include <component/collider.hpp>
#include <vector>
#include <entity.hpp>

using namespace Engine;
using namespace Physics;


RaycastHit Physics::Raycast(Ray ray, LayerMask mask, float distance) {

    RaycastHit result;
    result.m_hitInfo.m_mask = mask;
    CollisionWorld::get().raycast(ray, result, mask, distance);
    return result;
}

void PhysicsImpl::pollPhysics()
{
    // begin the timer if it hasn't started
    if (!physicsTick.isCounting()) physicsTick.start();
    double d = 1.0 / static_cast<double>(tps);

    // is the timer past the tick interval?
    if (physicsTick.count() >= d) {
        // yes, push through physics poll and reset the timer
        Entity::_pollPhysicsBehaviours(d);
        CollisionWorld::get().step();
        physicsTick.reset(); 
    }
}