#pragma once
#include <fm/fmath.hpp>
#include <debug.hpp>
#include <component/component.hpp>
#include <component/renderable.hpp>
#include <physics/ray.hpp>
#include <physics/boundingbox.hpp>
#include <layer.hpp>
#include <physics/rigidbody.hpp>

namespace Engine
{
    using ColliderID = uint16_t;
    using Physics::Rigidbody;
    using Engine::LayerMask;

    enum class ColliderShape
    {
        Box, Sphere
    };

    class Collider : public IRenderable
    {
        component_public:
            struct Contact
            {
                fm::vec3 normal;
                float penetration; // hehe sex
            };
        protected:
            // ID pool for easy and efficient unordered map management
            static inline IDPool<ColliderID> s_colliderIDPool; 
    
            // this instance's id
            ColliderID m_ColliderID;
        private:

            static bool CheckBoxIntersection(Collider& a, Collider& b, Contact& out);
            static bool CheckSphereIntersection(Collider& a, Collider& b, Contact& out);
            static bool CheckSphereBoxIntersection(Collider& a, Collider& b, Contact& out);

            

            void _drawBox(fm::vec3 min, fm::vec3 max, fm::mat4 mat, Debug::DebugLine& line);
            void _drawSphere(fm::vec3 center, fm::vec3 radius, fm::mat4 mat, Debug::DebugLine& line);

        public:

            PUBLIC fm::vec4 m_drawCol = fm::vec4(0.0f, 0.75f, 1.0f, 1.0f);

            PUBLIC ColliderShape m_shape;
            Rigidbody* m_rb = nullptr;
            PUBLIC bool m_isStatic = true;

            fm::vec3 m_offset = fm::vec3::zero();
            fm::vec3 m_center = fm::vec3::zero();
            fm::vec3 m_min = -fm::vec3::one() * 0.5f;
            fm::vec3 m_max = fm::vec3::one() * 0.5f;
            float m_radius = 1.0f;
            fm::vec3 getWorldCenter();
            float getWorldRadius();

            Collider(const fm::vec3& min, const fm::vec3& max);

            Collider(const fm::vec3& center, const float& radius);

            Collider(ColliderShape shape);

            bool IsCollidingWith(Collider& collider, Contact& out);
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

            void setStatic(bool s);
            AABB getWorldAABB();
    };

    class CollisionWorld {
    private:
        std::vector<Collider*> m_static, m_dynamic;
        BVH m_staticTree, m_dynamicTree;
        bool m_staticDirty = true;
    public:
        static CollisionWorld& get()
        {
            static CollisionWorld instance;
            return instance;
        }
        void add(Collider* c);       // push to m_static or m_dynamic by c->m_isStatic
        void remove(Collider* c);    // erase from the vector; set m_staticDirty if it was static
        void step();                 // run once per physics tick
        bool raycast(const Physics::Ray& ray, Physics::RaycastHit& hit, LayerMask mask, float maxDist = FLT_MAX);
        void resolve(Collider* a, Collider* b, const Collider::Contact& c);
        void refresh();
    };
}