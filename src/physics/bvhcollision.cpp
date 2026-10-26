#include <physics/bvhcollision.hpp>
#include <component/collider.hpp>
#include <logger.hpp>

using namespace Engine;
using namespace Physics;

void DynamicBVH::findOverlaps(BVHNode* nodeA, BVHNode* nodeB, std::vector<Collider::CollisionPair>& outPairs)
{
    if (!nodeA || !nodeB) return;

    if (!nodeA->box.intersects(nodeB->box)) return;

    if (nodeA->isLeaf() && nodeB->isLeaf())
    {
        if (nodeA != nodeB)
        {
            outPairs.push_back({nodeA->collider, nodeB->collider});
        }
        return;
    }

    if (nodeA->isLeaf()) {
            findOverlaps(nodeA, nodeB->left.get(), outPairs);
            findOverlaps(nodeA, nodeB->right.get(), outPairs);
        } 
        else if (nodeB->isLeaf()) {
            findOverlaps(nodeA->left.get(), nodeB, outPairs);
            findOverlaps(nodeA->right.get(), nodeB, outPairs);
        } 
        else {
            findOverlaps(nodeA->left.get(), nodeB->left.get(), outPairs);
            findOverlaps(nodeA->left.get(), nodeB->right.get(), outPairs);
            findOverlaps(nodeA->right.get(), nodeB->left.get(), outPairs);
            findOverlaps(nodeA->right.get(), nodeB->right.get(), outPairs);
        }
}

std::vector<Collider::CollisionPair> DynamicBVH::getCollisionPairs()
{
    std::vector<Collider::CollisionPair> pairs;
    findOverlaps(m_root.get(), m_root.get(), pairs);
    return pairs;
}

void DynamicBVH::insert(Collider* collider)
{
    auto leaf = std::shared_ptr<BVHNode>(new BVHNode());
    collider->m_bvhNode.reset();
    collider->m_bvhNode = leaf;

    leaf->collider = collider;

    leaf->left = nullptr;
    leaf->right = nullptr;
    
    

    if (m_root == nullptr)
    {
        m_root = leaf;
        m_root->parent = nullptr;
        return;
    }


    auto index = m_root;
    while (!index->isLeaf())
    {
        auto left = index->left;
        auto right = index->right;

        // cost of putting the leaf under left vs right
        float combinedArea = BoundingBox::merge(index->box, leaf->box).getSurfaceArea();
        
        // direct cost of creating a new parent with left and right leaves
        float directCost = 2.0f * combinedArea;
        
        // inheritance cost increase for children
        float inheritedCost = 2.0f * (combinedArea - index->box.getSurfaceArea());

        float costLeft = inheritedCost + (left->isLeaf() ? 
            BoundingBox::merge(left->box, leaf->box).getSurfaceArea() : 
            BoundingBox::merge(left->box, leaf->box).getSurfaceArea() - left->box.getSurfaceArea());

        float costRight = inheritedCost + (right->isLeaf() ? 
            BoundingBox::merge(right->box, leaf->box).getSurfaceArea() : 
            BoundingBox::merge(right->box, leaf->box).getSurfaceArea() - right->box.getSurfaceArea());

        // choose the branch with the lower cost
        if (costLeft < costRight)
            index = left;
        else
            index = right;
    }

    std::shared_ptr<BVHNode> oldSibling = index;
    std::shared_ptr<BVHNode> oldParent = oldSibling->parent;
    std::shared_ptr<BVHNode> newParent = std::shared_ptr<BVHNode>(new BVHNode());

    newParent->parent = oldParent;
    newParent->box = BoundingBox::merge(oldSibling->box, leaf->box);
    
    if (oldParent)
    {
        if (oldParent->left == oldSibling)
            oldParent->left = newParent;
        else
            oldParent->right = newParent;
    }
    else
    {
        m_root = newParent;
    }

    newParent->left = oldSibling;
    newParent->right = leaf;
    oldSibling->parent = newParent;
    leaf->parent = newParent;

    auto curr = newParent->parent;
    while (curr != nullptr)
    {
        curr->box = BoundingBox::merge(curr->left->box, curr->right->box);
        curr = curr->parent;
    }
}

void DynamicBVH::remove(BVHNode* leaf)
{
    if (!leaf) return;

    if (leaf == m_root.get())
    {
        m_root = nullptr;
        leaf = nullptr;
        return;
    }


    auto parent = leaf->parent;
    auto grandParent = parent->parent;
    auto sibling = (parent->left.get() == leaf) ? parent->right : parent->left;

    if (grandParent)
    {
        if (grandParent->left == parent)
            grandParent->left = sibling;
        else
            grandParent->right = sibling;
        
        sibling->parent = grandParent;
        parent = nullptr;
        leaf = nullptr;

        // refit upwards
        auto curr = grandParent;
        while (curr != nullptr)
        {
            curr->box = BoundingBox::merge(curr->left->box, curr->right->box);
            curr = curr->parent;
        }
    }
    else
    {
        m_root = sibling;
        sibling->parent = nullptr;
        parent = nullptr;
        leaf = nullptr;
    }
}

// Every frame, if an object moves:
void DynamicBVH::update(BVHNode* leaf)
{
    if (!leaf || !leaf->collider) return;

    remove(leaf);
    insert(leaf->collider); // Insert creates a fresh node
}