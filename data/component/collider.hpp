#include <fm/fmath.hpp>
#include <component/component.hpp>
#include <physics.hpp>

namespace Engine
{
    // Bounding box data for collisions, anything between minimum and maximum
    class BoundingBox : public IComponent, public std::enable_shared_from_this<BoundingBox>
    {

        public:
        fm::vec3 m_min; // bottom-left-front bounds
        fm::vec3 m_max; // top-right-back bounds
        static inline std::vector<std::weak_ptr<BoundingBox>> s_boundingBoxList;
        void init() override;
        // Checks for a collision and returns true if two bounding boxes overlap each other
        void SetBounds(fm::vec3 min, fm::vec3 max);
        static bool CheckBoxIntersection(const BoundingBox& a, const BoundingBox& b);
        bool CheckRayIntersection(const Ray& ray, RaycastHit& outRaycastHit);
        static bool RayAABoxIntersection(const Ray& ray, const BoundingBox& box, HitInfo& outHitInfo);
    };  
}