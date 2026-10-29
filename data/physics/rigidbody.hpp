// I am Isaac Newton the one and only

#pragma once
#include <component/component.hpp>
#include <fm/fmath.hpp>

namespace Engine {

    namespace Physics {
        // Allows entities to have physics
        class Rigidbody : public IComponent {
        private:
            // current entity velocity
            fm::vec3 m_velocity = fm::vec3::zero();
            // last entity position
            fm::vec3 lastPosition = fm::vec3::zero();

            // all the force types with velocity change and impulse setting velocity and default and accelerating adding
            // force over time
            // ---------------------------------------------------------------------------------------------------------
            void addImplicitForce(double delta);
            void addVelocityChangeForce(fm::vec3 force);
            void addImpulseForce(fm::vec3 force);
            void addDefaultForce(fm::vec3 force);
            void addAcceleratingForce(fm::vec3 force);
            // ---------------------------------------------------------------------------------------------------------

            // force to apply on next physics tick
            fm::vec3 m_force = fm::vec3::zero();
        public:
            // whether the component should use gravity
            bool m_useGravity = false;
            // not implemented yet
            float m_drag = 0.f;
            // mass of the entity
            float m_mass = 1.f;

            float m_bounciness = 0.f; // 0 = no bounce, 1 = perfectly elastic will be funny i guarentee

            // force modes for force application
            enum ForceMode
            {
                // Continuous force ignoring mass
                Acceleration,
                // Instant velocity change accounting for mass
                Impulse,
                // Continuous force accounting for mass
                Force,
                // Instant velocity change ignoring mass
                VelocityChange
            };
            
            // Applies a constant downward force of 9.8 m/s every physcis tick
            void applyGravity(double delta);
            // returns the entities velocity
            fm::vec3 getVelocity();
            // runs the physics tick for the rigidbody
            void physicsUpdate(double delta) override;
            // applies a force to the rigidbody
            void addForce(fm::vec3 force, ForceMode forceMode);

            float getInverseMass() const;
        };
    }  

}