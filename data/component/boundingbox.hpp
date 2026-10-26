#pragma once
#include <debug.hpp>
#include <fm/fmath.hpp>
#include <component/component.hpp>
#include <physics/physics.hpp>
#include <unordered_map>
#include <idpool.hpp>
#include <shader.hpp>
#include <mesh.hpp>
#include <component/collider.hpp>
#include <component/renderable.hpp>

// TODO : calculate bounds if the entity transform is dirty rather than just when drawing

namespace Engine
{
    using namespace Physics;
    using BoundingBoxID = uint16_t;

    // Bounding box data for collisions, anything between minimum and maximum
    class BoundingBox : public IRenderable  /* includes IComponent */{
    protected:

        // ID pool for easy and efficient unordered map management
        static inline IDPool<BoundingBoxID> s_boundingBoxIDPool; 

        // the list of bounding boxes. i realised using weak_ptr here is a bit silly (thank you for the insightful comment faye - hunter)
        static inline std::unordered_map<BoundingBoxID, BoundingBox*> s_boundingBoxList;
    
        // this instance's id
        BoundingBoxID m_boxID;

    public:

        fm::vec4 getColour() { return m_material->m_data.m_albedoFac; }
        void setColour(fm::vec4 col);

        // get this box ID
        BoundingBoxID getBoxID();

        std::shared_ptr<Collider> collider = nullptr;
        fm::vec3 m_min =-fm::vec3::one() * 0.5f; // bottom-left-front bounds
        fm::vec3 m_max = fm::vec3::one() * 0.5f; // top-right-back bounds

        void init() override;
        void onDestroy() override;
        void draw() override;

        // Checks for a collision and returns true if two bounding boxes overlap each other
        void setBounds(fm::vec3 min, fm::vec3 max);

        // is a ray intersecting a bounding box?
        bool checkRayIntersection(const Ray& ray, RaycastHit& outRaycastHit);

        // check for an intersection with an axis-aligned box
        static bool rayAABoxIntersection(const Ray& ray, const Collider* box, RaycastHit& outRaycastHit);
        // check for an intersection with an axis-aligned sphere
        static bool rayAASphereIntersection(const Ray& ray, const Collider* sphere, RaycastHit& outRaycastHit);
        
        // get a list of valid bounding boxes
        static std::unordered_map<BoundingBoxID, BoundingBox*> getBoundingBoxList();

        bool intersects(const BoundingBox& other) const;

        static BoundingBox merge(const BoundingBox& a, const BoundingBox& b);

        float getSurfaceArea() const;

        
    };
}