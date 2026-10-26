#include <component/collider.hpp>

using namespace Engine;

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

bool Collider::IsCollidingWith(Collider& collider)
{
    if (this->m_shape == ColliderShape::Box && collider.m_shape == ColliderShape::Box)
        return CheckBoxIntersection(*this, collider);
    else if (this->m_shape == ColliderShape::Sphere && collider.m_shape == ColliderShape::Sphere)
        return CheckSphereIntersection(*this, collider);
    else if (this->m_shape == ColliderShape::Sphere && collider.m_shape == ColliderShape::Box 
        || this->m_shape == ColliderShape::Box && collider.m_shape == ColliderShape::Sphere)
        return CheckSphereBoxIntersection(*this, collider);
    else
        return false;
}

bool Collider::CheckBoxIntersection(Collider& a, Collider& b) {
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
    return (a_min.x <= b_max.x && a_max.x >= b_min.x) &&
           (a_min.y <= b_max.y && a_max.y >= b_min.y) &&
           (a_min.z <= b_max.z && a_max.z >= b_min.z);
}

bool Collider::CheckSphereIntersection(Collider& a, Collider& b) {
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

    // returns an intersection if the distance of the spheres are less than the radius of them
    return distSq <= radiiSumSq;
}

bool Collider::CheckSphereBoxIntersection(Collider& a, Collider& b)
{
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
    closestPoint.y= std::max(boxMin.y, std::min(sphereCenter.y, boxMax.y));
    closestPoint.z = std::max(boxMin.z, std::min(sphereCenter.z, boxMax.z));

    // Calculates the distance of the sphere center from the closest point on the box
    fm::vec3 dist = sphereCenter - closestPoint;

    // gets the square of the distance to get a float
    float distSq = dist.dot(dist);

    // gets the square of the radius of the sphere
    float radiusSq = a.radius * a.radius;

    // returns a intersection if the distance between the box and the sphere is less than the sphere's radius
    return distSq <= radiusSq;
}