#pragma once
#include <fm/fmath.hpp>
#include <debug.hpp>
#include <component/component.hpp>
#include <component/renderable.hpp>
#include <physics/ray.hpp>
#include <component/boundingbox.hpp>
#include <layer.hpp>
#include <physics/rigidbody.hpp>

namespace Engine
{
    using ColliderID = uint16_t;
    using Physics::Rigidbody;
    using Engine::LayerMask;

    // The shape of the collider
    enum class ColliderShape
    {
        Box, Sphere
    };

    class Collider : public IRenderable
    {
        public:
            // contact point, stores the normal to where it was penetrated and how deep it penetrated
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

            // All the intersection checks for colliders
            // ---------------------------------------------------------------------------
            static bool CheckBoxIntersection(Collider& a, Collider& b, Contact& out);
            static bool CheckSphereIntersection(Collider& a, Collider& b, Contact& out);
            static bool CheckSphereBoxIntersection(Collider& a, Collider& b, Contact& out);
            // ---------------------------------------------------------------------------

            // draws a collider for debug purposes
            // -------------------------------------------------------------------------------------
            void _drawBox(fm::vec3 min, fm::vec3 max, fm::mat4 mat, Debug::DebugLine& line);
            void _drawSphere(fm::vec3 center, fm::vec3 radius, fm::mat4 mat, Debug::DebugLine& line);
            // -------------------------------------------------------------------------------------

            // Whether the collider is static (doesn't move during runtime)
            bool m_isStatic = true;
        public:

            fm::vec4 m_drawCol = fm::vec4(0.0f, 0.75f, 1.0f, 1.0f);

            // The colliders shape, its attributes rigidbody if it has one
            ColliderShape m_shape;
            Rigidbody* m_rb = nullptr;

            // offset from the center of the entity
            fm::vec3 offset = fm::vec3::zero();

            // the center point of a sphere and its radius
            fm::vec3 center = fm::vec3::zero();
            float radius = 1.0f;

            // the min and max bounds of a box
            fm::vec3 m_min = -fm::vec3::one() * 0.5f;
            fm::vec3 m_max = fm::vec3::one() * 0.5f;

            // convert local bounds to world
            fm::vec3 getWorldCenter();
            float getWorldRadius();

            // Collider constructors
            Collider(const fm::vec3& min, const fm::vec3& max);
            Collider(const fm::vec3& center, const float& radius);
            Collider(ColliderShape shape);

            // Collider collision checks
            bool IsCollidingWith(Collider& collider, Contact& out);
            bool IsCollidingWith(Collider& collider);

            // Color for debug stuffz
            fm::vec4 getColour() { return m_material->m_data.m_albedoFac; }
            void setColour(fm::vec4 col);

            // get this box ID
            ColliderID getColliderID();

            // Entity based stuff
            void init() override;
            void onDestroy() override;
            void draw() override;

            // is a ray intersecting a bounding box?
            bool checkRayIntersection(const Physics::Ray& ray, Physics::RaycastHit& outRaycastHit);

            // check for an intersection with an axis-aligned box
            static bool rayAABoxIntersection(const Physics::Ray& ray, const Collider* box, Physics::RaycastHit& outRaycastHit);
            // check for an intersection with an axis-aligned sphere
            static bool rayAASphereIntersection(const Physics::Ray& ray, const Collider* sphere, Physics::RaycastHit& outRaycastHit);

            // Setter and getter for static
            void setStatic(bool s);
            bool getStatic();

            // Converts the boxes bounds to world bounds
            AABB getWorldAABB();
    };

    class CollisionWorld {
    private:
        // The static and dynamic colliders lists
        std::vector<Collider*> m_static, m_dynamic;

        // The static and dynamic bounding volume trees
        BVH m_staticTree, m_dynamicTree;

        // marker for if the static collision has changes
        bool m_staticDirty = true;
    public:
        // gets the worlds collision data
        static CollisionWorld& get()
        {
            static CollisionWorld instance;
            return instance;
        }

        // Colliding stuffs
        void add(Collider* c);
        void remove(Collider* c);
        void step();
        bool raycast(const Physics::Ray& ray, Physics::RaycastHit& hit, LayerMask mask, float maxDist = FLT_MAX);
        void resolve(Collider* a, Collider* b, const Collider::Contact& c);
        void refresh();
    };
}