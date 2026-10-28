#include <fm/fmath.hpp>
#include <physics/rigidbody.hpp>
#include <entity.hpp>
#include <logger.hpp>
#include <physics/physics.hpp>

using namespace Engine;
using namespace Physics;

void Rigidbody::applyGravity(double delta)
{
    if (!m_useGravity) return;

    addAcceleratingForce(fm::vec3(0, 9.8, 0));
}

fm::vec3 Rigidbody::getVelocity()
{
   return m_velocity;
}

void Rigidbody::physicsUpdate(double delta)
{
    addImplicitForce(delta);
    applyGravity(delta);
    lastPosition = entity.transform.getPosition();
}

void Rigidbody::addForce(fm::vec3 force, Rigidbody::ForceMode forceMode)
{
    // Triggers a different force application function based on the force mode
    switch (forceMode)
    {
        case ForceMode::Force:
            addDefaultForce(force);
            break;
        case ForceMode::Acceleration:
            addAcceleratingForce(force);
            break;
        case ForceMode::Impulse:
            addImpulseForce(force);
            break;
        case ForceMode::VelocityChange:
            addVelocityChangeForce(force);
            break;
        default:
            addDefaultForce(force);
    }
}

void Rigidbody::addDefaultForce(fm::vec3 force)
{
    m_force += force / fm::vec3(m_mass); 
}

void Rigidbody::addAcceleratingForce(fm::vec3 force)
{
    m_force += force; 
}

void Rigidbody::addImpulseForce(fm::vec3 force)
{
    m_velocity += force / fm::vec3(m_mass); 
}

void Rigidbody::addVelocityChangeForce(fm::vec3 force)
{  
    m_velocity += force;
}

void Rigidbody::addImplicitForce(double delta)
{
    // Sets the acceleration for the object and applies it to the velocity by multiplying it by delta ti,e
    // then applies the position to the entity
    fm::vec3 acceleration = m_force;
    m_velocity += acceleration * static_cast<float>(delta);
    entity.transform.addPosition(m_velocity * static_cast<float>(delta));
    m_force = fm::vec3::zero();
}

float Rigidbody::getInverseMass() const {
    return m_mass > 0.1f ? 1.f / m_mass : 0.f;
}