#pragma once
#include <fm/fmath.hpp>
#include <debug.hpp>
#include <component/component.hpp>
#include <component/renderable.hpp>
#include <physics/ray.hpp>
#include <component/boundingbox.hpp>

namespace Engine
{
    using ColliderID = uint16_t;

    enum class ColliderShape
    {
        Box, Sphere
    };

    class Collider : public IRenderable
    {
        protected:
            // ID pool for easy and efficient unordered map management
            static inline IDPool<ColliderID> s_colliderIDPool; 

            // the list of bounding boxes. i realised using weak_ptr here is a bit silly (thank you for the insightful comment faye - hunter)
            static inline std::unordered_map<ColliderID, Collider*> s_colliderList;
    
            // this instance's id
            ColliderID m_ColliderID;
        private:
            static bool CheckBoxIntersection(Collider& a, Collider& b);
            static bool CheckSphereIntersection(Collider& a, Collider& b);
            static bool CheckSphereBoxIntersection(Collider& a, Collider& b);

            void _drawBox(fm::vec3 min, fm::vec3 max, fm::mat4 model, Debug::DebugLine& line);
            void _drawSphere(fm::vec3 center, fm::vec3 radiusScaled, fm::mat4 model, Debug::DebugLine& line);

        public:

            fm::vec4 m_drawCol = fm::vec4(0.0f, 0.75f, 1.0f, 1.0f);

            ColliderShape m_shape;
            //BoundingBox m_boundingBox;

            fm::vec3 offset = fm::vec3::zero();
            fm::vec3 center = fm::vec3::zero();
            fm::vec3 m_min = -fm::vec3::one() * 0.5f;
            fm::vec3 m_max = fm::vec3::one() * 0.5f;
            float radius = 1.0f;

            Collider(const fm::vec3& min, const fm::vec3& max);

            Collider(const fm::vec3& center, const float& radius);

            Collider(ColliderShape shape);

            bool IsCollidingWith(Collider& collider);

            fm::vec4 getColour() { return m_material->m_data.m_albedoFac; }
            void setColour(fm::vec4 col);

            // get this box ID
            ColliderID getColliderID();

            void init() override;
            void onDestroy() override;
            void draw() override;

            // is a ray intersecting a bounding box?
            bool checkRayIntersection(const Physics::Ray& ray, Physics::RaycastHit& outRaycastHit);

            // check for an intersection with an axis-aligned box
            static bool rayAABoxIntersection(const Physics::Ray& ray, const Collider* box, Physics::RaycastHit& outRaycastHit);
            // check for an intersection with an axis-aligned sphere
            static bool rayAASphereIntersection(const Physics::Ray& ray, const Collider* sphere, Physics::RaycastHit& outRaycastHit);
        
            // get a list of valid bounding boxes
            static std::unordered_map<ColliderID, Collider*> getColliderList();
    };
}