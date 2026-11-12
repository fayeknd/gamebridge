#pragma once
#include <fm/fmath.hpp>
#include <debug.hpp>
#include <component/component.hpp>
#include <component/modeldraw.hpp>
#include <physics/ray.hpp>
#include <physics/boundingbox.hpp>
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
        Box, Sphere, Cylinder, Capsule
    };

    class Collider : public ModelDraw
    {
        component_public:
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
            static bool CheckSphereBoxIntersection(fm::vec3 c, float r, Collider& b, Contact& out);
            static bool CheckSphereBoxIntersection(Collider& a, Collider& b, Contact& out);
            static bool CheckBoxCylinderIntersection(Collider& a, Collider& b, Contact& out);
            static bool CheckSphereCylinderIntersection(Collider& a, Collider& b, Contact& out);
            static bool CheckCylinderIntersection(Collider& a, Collider& b, Contact& out);
            static bool CheckBoxCapsuleIntersection(Collider& a, Collider& b, Contact& out);
            static bool CheckSphereCapsuleIntersection(Collider& a, Collider& b, Contact& out);
            static bool CheckCylinderCapsuleIntersection(Collider& a, Collider& b, Contact& out);
            static bool CheckCapsuleIntersection(Collider& a, Collider& b, Contact& out);
            // ---------------------------------------------------------------------------

            // gets the world end points of a cylinder/capsule
            static void getSegmentEnds(Collider& col, fm::vec3& outA, fm::vec3& outB);
            static void closestPointBetweenSegments(fm::vec3 p1, fm::vec3 q1, fm::vec3 p2, fm::vec3 q2, fm::vec3& c1, fm::vec3& c2);

            // draws a collider for debug purposes
            // -------------------------------------------------------------------------------------
            void _drawBox(fm::vec3 min, fm::vec3 max, fm::mat4 mat, Debug::DebugLine& line);
            void _drawSphere(fm::vec3 center, fm::vec3 radius, fm::mat4 mat, Debug::DebugLine& line);
            void _drawCylinder(float r, float h, fm::mat4 model, Debug::DebugLine& line);
            void _drawCapsule(float r, float h, fm::mat4 model, Debug::DebugLine& line);
            // -------------------------------------------------------------------------------------

            // Whether the collider is static (doesn't move during runtime)
            bool m_isStatic = true;
        public:

            PUBLIC fm::vec4 m_drawCol = fm::vec4(0.0f, 0.75f, 1.0f, 1.0f);

            PUBLIC ColliderShape m_shape;
            Rigidbody* m_rb = nullptr;

            PUBLIC fm::vec3 m_offset = fm::vec3::zero();
            PUBLIC fm::vec3 m_center = fm::vec3::zero();
            PUBLIC float m_radius = 1.0f;

            // height for a capsule and cylinder
            PUBLIC float m_height = 2.f;

            // the min and max bounds of a box
            PUBLIC fm::vec3 m_min = -fm::vec3::one() * 0.5f;
            PUBLIC fm::vec3 m_max = fm::vec3::one() * 0.5f;
            // convert local bounds to world
            fm::vec3 getWorldCenter();
            float getWorldRadius();
            float getWorldHeight();

            // Collider constructors
            Collider(const fm::vec3& min, const fm::vec3& max);
            Collider(const fm::vec3& center, const float& radius);
            Collider(ColliderShape shape, const float& radius, const float& height);
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
            static bool rayAACylinderIntersection(const Physics::Ray& ray, const Collider* cyl, Physics::RaycastHit& outRaycastHit);
            static bool rayAACapsuleIntersection(const Physics::Ray& ray, const Collider* cap, Physics::RaycastHit& outRaycastHit);

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