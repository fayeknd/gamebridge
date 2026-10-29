#include "gl_context.hpp"
#include <component/boundingbox.hpp>
#include <entity.hpp>
#include <component/collider.hpp>


using namespace Engine;

// returns the center of an axis aligned box
static fm::vec3 centre(const AABB& b) {
    return (b.m_min + b.m_max) * 0.5f;
}

// returns the lonest axis of the axis aligned box 0 = x, 1 = y, 2 = z
static int longestAxis(const AABB& b) {
    fm::vec3 d = b.m_max - b.m_min;
    if (d.x > d.y && d.x > d.z) return 0;
    if (d.y > d.z)              return 1;
    return 2;
}

// binds two boxes together to create a larger box
AABB AABB::merge(const AABB& a, const AABB& b) {
    return {
        { std::min(a.m_min.x, b.m_min.x), std::min(a.m_min.y, b.m_min.y), std::min(a.m_min.z, b.m_min.z) },
        { std::max(a.m_max.x, b.m_max.x), std::max(a.m_max.y, b.m_max.y), std::max(a.m_max.z, b.m_max.z) }
    };
}

// builds an bounding volume hierarchy
int BVH::build(std::vector<Item>& items, size_t b, size_t e)
{
    // reserves a slot in the memory for the tree and claims a slot for its children
    // before they exist
    int index = (int)m_nodes.size();
    m_nodes.emplace_back();

    // if the range has one item the item is a leaf, store its box and collider and return
    if (e - b == 1)
    {
        m_nodes[index].bounds = items[b].box;
        m_nodes[index].collider = items[b].c;
        return index;
    }


    // compute the total bounds of everything in the range by merging all the bounding boxes into one
    // then picks the longest axis of that box
    AABB total = AABB((b + e) / 2);
    for (size_t i = b + 1; i < e; i++)
    {
        total = AABB::merge(total, items[i].box);
    }
    int axis = longestAxis(total);

    // splits the item down the median of the list, rearranges the range of the items at the median
    // then using that it fills the nodes left and right nodes by going from the median ensuring the tree is symmetrical
    // this is actually pretty fucking cool since it makes it so when travelling down a tree it gets to its requested item faster
    // pretty much it makes the depth log2(n)
    size_t mid = (b + e) / 2;
    std::nth_element(items.begin() + b, items.begin() + mid, items.begin() + e,
        [axis](const Item& a, const Item& c) {
            return centre(a.box)[axis] < centre(c.box)[axis];
        });

    int l = build(items, b, mid);
    int r = build(items, mid, e);

    // WARNING: NO FUCKING POINTERS FAYYYYEEEEE DONT POINTER THIS TO AN AABB
    // we're using indices for the left and right of the nodes vector because if we create pointers
    // to the specific nodes we might reallocate them in memory which means that the tree won't be recursive
    m_nodes[index].left = l;
    m_nodes[index].right = r;
    m_nodes[index].bounds = total;
    return index;
}

// returns which colliders boxes are overlapping a given box
void BVH::query(const AABB& box, std::vector<Collider*>& out) const {
    // push the root
    if (m_root < 0) return;
    int stack[64]; int sp = 0;
    stack[sp++] = m_root;
    while (sp) {
        const BVNode& n = m_nodes[stack[--sp]];
        // pop a node and if it doesn't overlap continue
        if (!n.bounds.overlap(box)) continue;
        // if its a leaf add the collider otherwise push both children
        if (n.isLeaf()) out.push_back(n.collider);
        else { stack[sp++] = n.left; stack[sp++] = n.right; }
    }
}

// returns whether a ray has hit a bounding volume
bool AABB::rayHit(const fm::vec3& o, const fm::vec3& invDir, float tMax, float& tEnter) const {
    // gets the difference of the bounds to ray
    float t1 = (m_min.x - o.x) * invDir.x, t2 = (m_max.x - o.x) * invDir.x;

    // gets the near and far points of the ray
    float tNear = std::min(t1, t2), tFar = std::max(t1, t2);

    // converts to world space
    // --------------------------------------------------------------
    t1 = (m_min.y - o.y) * invDir.y; t2 = (m_max.y - o.y) * invDir.y;
    tNear = std::max(tNear, std::min(t1, t2));
    tFar  = std::min(tFar,  std::max(t1, t2));

    t1 = (m_min.z - o.z) * invDir.z; t2 = (m_max.z - o.z) * invDir.z;
    tNear = std::max(tNear, std::min(t1, t2));
    tFar  = std::min(tFar,  std::max(t1, t2));
    // --------------------------------------------------------------

    // returns false if the object is behind the ray or outside of render distance
    if (tFar < 0.0f) return false;
    if (tNear > tFar || tNear > tMax) return false;

    // sets to enter point and returns true
    tEnter = std::max(tNear, 0.0f);
    return true;
}

// casts a ray and searches through bounding volumes to get the collission
bool BVH::raycast(const Physics::Ray& ray, Physics::RaycastHit& outHit, LayerMask mask, float maxDist) const {
    // no bounding volume
    if (m_root < 0) return false;

    // gets the inverse of the ray direction and sets the closest point to outside rendering range
    fm::vec3 inv(1.0f / ray.m_direction.x, 1.0f / ray.m_direction.y, 1.0f / ray.m_direction.z);
    float closest = maxDist;
    bool hitAny = false;

    // creates a stack of integars to search throught the bounding volumes with
    int stack[64]; int sp = 0;

    // gets the root of the tree
    stack[sp++] = m_root;

    // iterates through each bounding volume to find the collider
    while (sp) {
        // gets a node from the stack
        const BVNode& n = m_nodes[stack[--sp]];

        // shoots a raycast forward again and checks if there is no hit, coontinue if so
        float tEnter;
        if (!n.bounds.rayHit(ray.m_origin, inv, closest, tEnter)) continue;

        // not a collider check the left and right nodse
        if (!n.isLeaf())
        {
            stack[sp++] = n.left;
            stack[sp++] = n.right;
            continue;
        }

        // gets the colliders entity and checks its mask
        Entity& e = n.collider->getEntity();
        if ((e.getLayerID() & mask) == 0 || e.getLayer()->m_ignoreRaycast) continue;

        // creates the hit variable and sets the mask
        Physics::RaycastHit h;
        h.m_hitInfo.m_mask = mask;

        // shoots the raycast intersection to check if it hits a collider
        if (n.collider->checkRayIntersection(ray, h) && h.m_hitInfo.m_distance < closest)
        {
            // sets the variables
            closest = h.m_hitInfo.m_distance;
            outHit = h;
            hitAny = true;
        }
    }
    // returns the hit
    return hitAny;
}

// rebuild the bounding volume tree
void BVH::rebuild(const std::vector<Collider*>& colliders) {
    // clears the current tree and root node
    m_nodes.clear();
    m_root = -1;
    // no colliders = return
    if (colliders.empty()) return;

    // creates a new vector for the bvh tree's items
    std::vector<Item> items;
    items.reserve(colliders.size()); // reserves space in memory
    for (Collider* c : colliders) items.push_back({ c->getWorldAABB(), c }); // pushes back the collider and makes a new bounding volume from its size

    // reserves the node tree in memory
    m_nodes.reserve(items.size() * 2);   // a binary tree with n leaves has 2n-1 nodes
    // build a new tree
    m_root = build(items, 0, items.size());
}
