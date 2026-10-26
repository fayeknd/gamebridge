#pragma once
#include <fm/fmath.hpp>
#include <debug.hpp>
#include <component/component.hpp>

namespace Engine
{
    namespace Physics { struct BVHNode; }

    enum class ColliderShape
    {
        Box, Sphere
    };

    class Collider
    {
        public:
            struct IntersectionData
            {
                bool isIntersection = false;
                fm::vec3 normal = fm::vec3::zero();
                float penetration = 0.f; // hehe sex
            };
        private:
            static IntersectionData CheckBoxIntersection(Collider& a, Collider& b);
            static IntersectionData CheckSphereIntersection(Collider& a, Collider& b);
            static IntersectionData CheckSphereBoxIntersection(Collider& a, Collider& b);

            void _drawBox(fm::vec3 min, fm::vec3 max, Debug::DebugLine& line);
            void _drawSphere(fm::vec3 center, fm::vec3 radiusScaled, Debug::DebugLine& line);

        public:
            std::shared_ptr<Physics::BVHNode> m_bvhNode;
            fm::vec4 m_drawCol = fm::vec4(0.0f, 0.75f, 1.0f, 1.0f);

            ColliderShape m_shape;

            fm::vec3 offset = fm::vec3::zero();
            fm::vec3 m_min =-fm::vec3::one() * 0.5f; // bottom-left-front bounds
            fm::vec3 m_max = fm::vec3::one() * 0.5f; // top-right-back bounds

            fm::vec3 center = fm::vec3::zero();
            float radius = 1.0f;

            Collider(const fm::vec3& min, const fm::vec3& max);

            Collider(const fm::vec3& center, const float& radius);

            Collider(ColliderShape shape);

            void draw(fm::vec3 pos, fm::vec3 scale);

            IntersectionData IsCollidingWith(Collider& collider);

            struct CollisionPair
            {
                Collider* colliderA;
                Collider* colliderB;

                bool operator==(const CollisionPair& other) const
                {
                    return (colliderA == other.colliderA && colliderB == other.colliderB) ||
                           (colliderA == other.colliderB && colliderB == other.colliderA);
                }   
            };
            
            
    };
}