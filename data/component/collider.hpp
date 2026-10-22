#include <fm/fmath.hpp>
#include <component/component.hpp>
#include <physics.hpp>
#include <unordered_map>
#include <idpool.hpp>

using namespace Engine::Physics;

namespace Engine
{

    using BoundingBoxID = uint16_t;

    // Bounding box data for collisions, anything between minimum and maximum
    class BoundingBox : public IComponent {
    protected:

        // ID pool for easy and efficient unordered map management
        static inline IDPool<BoundingBoxID> s_boundingBoxIDPool; 

        // the list of bounding boxes. i realised using weak_ptr here is a bit silly
        static inline std::unordered_map<BoundingBoxID, BoundingBox*> s_boundingBoxList;
    
        // this instance's id
        BoundingBoxID m_boxID;

    public:

        BoundingBoxID getBoxID();

        fm::vec3 m_min =-fm::vec3::one() * 0.5f; // bottom-left-front bounds
        fm::vec3 m_max = fm::vec3::one() * 0.5f; // top-right-back bounds

        void init() override;
        void onDestroy() override;

        // Checks for a collision and returns true if two bounding boxes overlap each other
        void setBounds(fm::vec3 min, fm::vec3 max);

        // are these boxes intersecting?
        static bool checkBoxIntersection(BoundingBox& a, BoundingBox& b);

        // is a ray intersecting a bounding box?
        bool checkRayIntersection(const Ray& ray, RaycastHit& outRaycastHit);

        // check for an intersection with an axis-aligned box
        static bool rayAABoxIntersection(const Ray& ray, const BoundingBox& box, RaycastHit& outRaycastHit);
        
        // get a list of valid bounding boxes
        static std::unordered_map<BoundingBoxID, BoundingBox*> getBoundingBoxList();

    };  
}