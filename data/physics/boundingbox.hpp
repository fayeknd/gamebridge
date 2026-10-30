#pragma once
#include <fm/fmath.hpp>
#include <algorithm>
#include <vector>
#include <physics/ray.hpp>
#include <layer.hpp>

namespace Engine
{
    class Collider;

    // Axis aligned bounding box, encapsulates a collider and extends from there to create a BVH
    struct AABB {
        fm::vec3 m_min, m_max;

        // returns whether two axis aligned bounding boxes overlap eachother
        bool overlap (const AABB& o) const
        {
            return m_min.x <= o.m_max.x && m_max.x >= o.m_min.x &&
               m_min.y <= o.m_max.y && m_max.y >= o.m_min.y &&
               m_min.z <= o.m_max.z && m_max.z >= o.m_min.z;
        }

        // binds two bounding boxes together to create a larger box
        static AABB merge(const AABB& a, const AABB& b);
        // returns if a ray hit the bounding box
        bool rayHit(const fm::vec3& o, const fm::vec3& invDir, float tMax, float& tEnter) const;
    };

    // A bounding volume, stores its bounds and the volumes to the left and right of it and the collider it contains
    // if it is a leaf node
    struct BVNode
    {
        AABB bounds;
        int left = -1;
        int right = -1;
        Collider* collider = nullptr;
        bool isLeaf() const { return collider != nullptr; };
    };

    // Bounding Volume Hierarchy to manage collisions
    class BVH {
    private:
        std::vector<BVNode> m_nodes;
        int m_root = -1;

        struct Item { AABB box; Collider* c; };
        int build(std::vector<Item>& items, size_t begin, size_t end);
    public:
        void rebuild(const std::vector<Collider*>& colliders);
        void query(const AABB& box, std::vector<Collider*>& out) const;
        bool raycast(const Physics::Ray& ray, Physics::RaycastHit& outHit, LayerMask mask, float maxDist) const;
    };
}