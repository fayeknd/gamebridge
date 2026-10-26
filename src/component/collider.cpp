#include <component/collider.hpp>
#include <debug.hpp>

using namespace Engine;
using namespace Engine::Debug;

// :face_with_raised_eyebrow:       -faye

Collider::Collider(const fm::vec3& min, const fm::vec3& max)
{
    m_shape = ColliderShape::Box;
    this->m_min = min;
    this->m_max = max;
}

Collider::Collider(const fm::vec3& center, const float& radius)
{
    m_shape = ColliderShape::Box;
    this->center = center;
    this->radius = radius;
}

Collider::Collider(ColliderShape shape)
{
    this->m_shape = shape;
}

Collider::IntersectionData Collider::IsCollidingWith(Collider& collider)
{

    switch (this->m_shape) {
        case (ColliderShape::Box) : {

            switch (collider.m_shape) {

                case (ColliderShape::Sphere) : {
                    return CheckSphereBoxIntersection(*this, collider);
                }
                case (ColliderShape::Box) : {
                    return CheckBoxIntersection(*this, collider);
                }

            }
            break;
        }
        case (ColliderShape::Sphere) : {

            switch (collider.m_shape) {

                case (ColliderShape::Sphere) : {
                    return CheckSphereBoxIntersection(*this, collider);
                }
                case (ColliderShape::Box) : {
                    return CheckSphereBoxIntersection(collider, *this);
                }

            }
            break;
        }
    }

    IntersectionData nullData;
    return nullData;
}

Collider::IntersectionData Collider::CheckBoxIntersection(Collider& a, Collider& b) {
    IntersectionData data;

    // easy fucking peasy 

    // get the entity global scale
    fm::vec3 a_scale = fm::vec3::one();//a.getEntity().transform.getGlobalScale();
    fm::vec3 b_scale = fm::vec3::one();//b.getEntity().transform.getGlobalScale();

    // transform the bounds by the global scale
    fm::vec3 a_min = a.m_min * a_scale;
    fm::vec3 a_max = a.m_max * a_scale;

    fm::vec3 b_min = b.m_min * b_scale;
    fm::vec3 b_max = b.m_max * b_scale;

    // calculate intersection in any axis
    data.isIntersection = (a_min.x <= b_max.x && a_max.x >= b_min.x) &&
                          (a_min.y <= b_max.y && a_max.y >= b_min.y) &&
                          (a_min.z <= b_max.z && a_max.z >= b_min.z);

    if (!data.isIntersection)
        return data;

    float overlapX1 = b_max.x - a_min.x; // How much A overlaps B from the left
    float overlapX2 = a_max.x - b_min.x; // How much A overlaps B from the right
    float minOverlapX = std::min(overlapX1, overlapX2);

    float overlapY1 = b_max.y - a_min.y;
    float overlapY2 = a_max.y - b_min.y;
    float minOverlapY = std::min(overlapY1, overlapY2);

    float overlapZ1 = b_max.z - a_min.z;
    float overlapZ2 = a_max.z - b_min.z;
    float minOverlapZ = std::min(overlapZ1, overlapZ2);

    // Find the absolute smallest overlap axis
    data.penetration = minOverlapX;
    data.normal = fm::vec3(1.0f, 0.0f, 0.0f);

    if (minOverlapY < data.penetration) {
        data.penetration = minOverlapY;
        data.normal = fm::vec3(0.0f, 1.0f, 0.0f);
    }
    if (minOverlapZ < data.penetration) {
        data.penetration = minOverlapZ;
        data.normal = fm::vec3(0.0f, 0.0f, 1.0f);
    }


    fm::vec3 centerA = (a_min + a_max) * 0.5f;
    fm::vec3 centerB = (b_min + b_max) * 0.5f;
    fm::vec3 dir = centerA - centerB;

    if (dir.dot(data.normal) < 0.0f) {
        data.normal = data.normal * -1.0f; // Flip normal if facing the wrong fucking way
    }

    return data;
}

Collider::IntersectionData Collider::CheckSphereIntersection(Collider& a, Collider& b) {
    IntersectionData data;
    
    // Gets center of sphere A and sphere B accounting for offset
    fm::vec3 centerA = a.center + a.offset;
    fm::vec3 centerB = b.center + b.offset;

    // gets distance between the centers
    fm::vec3 dist = centerB - centerA;

    // gets the magnitude of the distances
    float distSq = dist.dot(dist);

    // adds the radii and squares them
    float radiiSum = a.radius + b.radius;
    float radiiSumSq = a.radius * b.radius;

    data.isIntersection = distSq <= radiiSumSq;
    data.normal = dist / std::sqrt(distSq);
    data.penetration = radiiSum - std::sqrt(distSq);

    // returns an intersection if the distance of the spheres are less than the radius of them
    return data;
}

Collider::IntersectionData Collider::CheckSphereBoxIntersection(Collider& a, Collider& b)
{
    IntersectionData data;

    // A is always = sphere and b is always = box
    if (b.m_shape == ColliderShape::Sphere && a.m_shape == ColliderShape::Box)
    {
        Collider& c = b;
        a = b;
        a = c;
    }

    // Get the box's min and max values adjusted for the offset and the spheres center
    fm::vec3 boxMin = b.m_min + b.offset;
    fm::vec3 boxMax = b.m_max + b.offset;
    fm::vec3 sphereCenter = a.center + a.offset;

    fm::vec3 closestPoint;

    // gets the closest point of each of the box's axis to the center of the sphere in each axis to get the distance
    closestPoint.x = std::max(boxMin.x, std::min(sphereCenter.x, boxMax.x));
    closestPoint.y = std::max(boxMin.y, std::min(sphereCenter.y, boxMax.y));
    closestPoint.z = std::max(boxMin.z, std::min(sphereCenter.z, boxMax.z));

    // Calculates the distance of the sphere center from the closest point on the box
    fm::vec3 dist = sphereCenter - closestPoint;

    // gets the square of the distance to get a float
    float distSq = dist.dot(dist);

    // gets the square of the radius of the sphere
    float radiusSq = a.radius * a.radius;

    data.isIntersection = distSq <= radiusSq;

    if (!data.isIntersection) return data;


    float distance = std::sqrt(distSq);

    // Handle edge case: Sphere center is strictly inside the box
    if (distance < 0.0001f) {
        data.normal = fm::vec3(0.0f, 1.0f, 0.0f); // Default fallback normal
        data.penetration = a.radius;       // Full radius penetration estimate
    } else {
        data.normal = dist / distance;         // Normal pointing out of the box
        data.penetration = a.radius - distance; // Correct linear penetration
    }

    // returns a intersection if the distance between the box and the sphere is less than the sphere's radius
    return data;
}

void Collider::draw(fm::vec3 pos, fm::vec3 scale) {

    static DebugLine line;
    fm::vec3 globalPos = pos;
    fm::vec3 entityScale = scale;
    
    switch (m_shape) {
        case (ColliderShape::Box) : {
            fm::vec3 min = (m_min * entityScale) + globalPos;
            fm::vec3 max = (m_max * entityScale) + globalPos;
            _drawBox(min, max, line);
            break;
        }
        case (ColliderShape::Sphere) : {

            fm::vec3 globalCenter = (center * entityScale) + globalPos;
            fm::vec3 scaledRadius = fm::vec3(radius) * entityScale;

            _drawSphere(globalCenter, scaledRadius, line);
            break;
        }
    }
}

void Collider::_drawBox(fm::vec3 min, fm::vec3 max, Debug::DebugLine& line) {
    fm::vec3 v0 = min;
    fm::vec3 v1 = fm::vec3(max.x, min.y, min.z);
    fm::vec3 v2 = fm::vec3(max.x, max.y, min.z);
    fm::vec3 v3 = fm::vec3(min.x, max.y, min.z);

    fm::vec3 v4 = fm::vec3(min.x, min.y, max.z);
    fm::vec3 v5 = fm::vec3(max.x, min.y, max.z);
    fm::vec3 v6 = max;
    fm::vec3 v7 = fm::vec(min.x, max.y, max.z);
    
    line.draw({
        Point(v0, m_drawCol),
        Point(v1, m_drawCol),
        Point(v2, m_drawCol),
        Point(v3, m_drawCol),
        Point(v0, m_drawCol),
        Point(v4, m_drawCol),
        Point(v5, m_drawCol),
        Point(v1, m_drawCol),
        Point(v5, m_drawCol),
        Point(v6, m_drawCol),
        Point(v2, m_drawCol),
        Point(v6, m_drawCol),
        Point(v7, m_drawCol),
        Point(v3, m_drawCol),
        Point(v7, m_drawCol),
        Point(v4, m_drawCol),
    });
}

void Collider::_drawSphere(fm::vec3 center, fm::vec3 scaledRadius, Debug::DebugLine &line) {
    
    // how many segments each circle has
    const int segments = 48;
    // how big is the angle for each step through the circle?
    const float angleStep = 2.0f * fm::PI / static_cast<float>(segments);
    
    // XZ plane
    std::vector<Point> xzPoints;
    xzPoints.reserve(segments + 1);

    for (int i = 0; i <= segments; ++i) {

        float angle = i * angleStep;
        // this same math can be used to calculate directional vectors i believe. fun fact
        fm::vec3 pos(
            center.x + std::cos(angle) * scaledRadius.x,
            center.y,
            center.z + std::sin(angle) * scaledRadius.z
        );
        xzPoints.push_back(Point(pos, m_drawCol));
    }

    line.draw(xzPoints); // draw ring 1

    // XY plane
    std::vector<Point> xyPoints;
    xyPoints.reserve(segments + 1);

    for (int i = 0; i <= segments; ++i) {
        
        float angle = i * angleStep;
        
        fm::vec3 pos(
            center.x + std::cos(angle) * scaledRadius.x,
            center.y + std::sin(angle) * scaledRadius.y,
            center.z
        );
        xyPoints.push_back(Point(pos, m_drawCol));
    }

    line.draw(xyPoints); // draw ring 2

    // YZ plane
    std::vector<Point> yzPoints;
    yzPoints.reserve(segments + 1);

    for (int i = 0; i <= segments; ++i) {

        float angle = i * angleStep;
        fm::vec3 pos(
            center.x,
            center.y + std::cos(angle) * scaledRadius.y,
            center.z + std::sin(angle) * scaledRadius.z
        );
        yzPoints.push_back(Point(pos, m_drawCol));
    }
    line.draw(yzPoints); // draw ring 3
}