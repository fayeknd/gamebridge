#pragma once
#include <component/boundingbox.hpp>

namespace Engine{

    class Collider;

    namespace Physics{

    // Node for a Bounding volume storing a bounding box, collider and its parent and left and right leafs
    struct BVHNode
    {
        BoundingBox box;
        std::shared_ptr<BVHNode> parent = nullptr;
        std::shared_ptr<BVHNode> left = nullptr;
        std::shared_ptr<BVHNode> right = nullptr;
        Collider* collider = nullptr; // only a leaf node will have a collider

        bool isLeaf() const { return collider != nullptr; }
    };

    // The class for going through each bounding volume in a hierarchy for collision checks
    class DynamicBVH
    {
        private:
            // root node for a boundng volume
            std::shared_ptr<BVHNode> m_root = nullptr;

            void findOverlaps(BVHNode* nodeA, BVHNode* nodeB, std::vector<Collider::CollisionPair>& outPairs);
        public:
            // insert a new collider into the tree
            void insert(Collider* collider);

            // remove a collider when destroyed
            void remove(BVHNode* leaf);

            // update position when a collider moves
            void update(BVHNode* leaf);

            //  ask the tree to find all colliding pairs
            void queryOverlap(BVHNode* node, std::vector<Collider::CollisionPair>& outPairs);

            std::vector<Collider::CollisionPair> getCollisionPairs();
    };
    }
}

