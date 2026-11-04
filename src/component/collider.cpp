#include <component/collider.hpp>
#include <physics/physics.hpp>
#include <entity.hpp>
#include <debug.hpp>

using namespace Engine;
using namespace Physics;
using namespace Engine::Debug;

// :face_with_raised_eyebrow:       -faye
// :sob:                            - hunter

// Constructs a new box collider
Collider::Collider(const fm::vec3& min, const fm::vec3& max)
{
    m_shape = ColliderShape::Box;
    this->m_min = min;
    this->m_max = max;
}

// Constructs a new sphere collider
Collider::Collider(const fm::vec3& center, const float& radius)
{
    m_shape = ColliderShape::Sphere;
    this->m_center = center;
    this->m_radius = radius;
}

// Constructs a new capsule or cylinder collider
Collider::Collider(ColliderShape shape, const float& radius, const float& height)
{
    m_shape = shape;
    this->m_height = height;
    this->m_radius = radius;
}

// Constructs a new collider from a shape
Collider::Collider(ColliderShape shape)
{
    this->m_shape = shape;
}

// gets the world center of the collider
fm::vec3 Collider::getWorldCenter() {
    return (m_center + m_offset).transformed(getEntity().transform.getGlobalMatrix());
}

// gets the radius of the collider in world space
float Collider::getWorldRadius() {
    fm::vec3 s = getEntity().transform.getGlobalScale();
    return m_radius * std::max({ std::abs(s.x), std::abs(s.y), std::abs(s.z) });
}

// gets the height of the collider in world space
float Collider::getWorldHeight() {
    fm::vec3 s = getEntity().transform.getGlobalScale();
    return m_height * std::max({ std::abs(s.x), std::abs(s.y), std::abs(s.z) });
}

void Collider::getSegmentEnds(Collider& col, fm::vec3& outA, fm::vec3& outB)
{
    // gets the world matrix of the collider
    fm::mat4 world = col.getEntity().transform.getGlobalMatrix();
    // gets half the heigh for capsule hemispherical ends
    float halfH = std::max(0.0f, (col.m_height * 0.5f) - col.m_radius);

    // sets the collider height for half for a cylinder
    if (col.m_shape == ColliderShape::Cylinder) halfH = col.m_height * 0.5f;

    // returns the local half heights
    fm::vec3 localA(0.0f, -halfH, 0.0f);
    fm::vec3 localB(0.0f,  halfH, 0.0f);

    // transforms the local ends to world ends
    outA = (localA + col.m_offset).transformed(world);
    outB = (localB + col.m_offset).transformed(world);
}

// closest points between two line segments (p1->q1 and p2->q2), outputs them in c1 and c2
// this is the classic "closest point of approach" maths for capsules and cylinders
void Collider::closestPointBetweenSegments(fm::vec3 p1, fm::vec3 q1, fm::vec3 p2, fm::vec3 q2, fm::vec3& c1, fm::vec3& c2)
{
    // direction of each segment and the vector between their starts
    fm::vec3 d1 = q1 - p1;
    fm::vec3 d2 = q2 - p2;
    fm::vec3 r = p1 - p2;

    // squared lengths of each segment and d2 projected onto r
    float a = d1.dot(d1);
    float e = d2.dot(d2);
    float f = d2.dot(r);

    // s and t are how far along each segment the closest point is (0 = start, 1 = end)
    float s, t;

    // both segments are basically points, so the closest points are just the starts
    if (a <= 1e-6f && e <= 1e-6f) {
        c1 = p1; c2 = p2; return;
    }

    // the first segment is a point, so only t needs working out
    if (a <= 1e-6f) {
        s = 0.0f;
        t = std::clamp(f / e, 0.0f, 1.0f);
    } else {
        float c = d1.dot(r);

        // the second segment is a point, so only s needs working out
        if (e <= 1e-6f) {
            t = 0.0f;
            s = std::clamp(-c / a, 0.0f, 1.0f);
        } else {
            // general case, both segments have a length
            float b = d1.dot(d2);
            float denom = a * e - b * b; // 0 when the segments are parallel

            // if they're not parallel, find the closest point on segment 1 to segment 2
            // otherwise just pick the start (any point works for parallel lines)
            if (denom != 0.0f) {
                s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
            } else {
                s = 0.0f;
            }

            // work out t from s
            t = (b * s + f) / e;

            // if t fell off either end of segment 2, clamp it and recompute s to match
            if (t < 0.0f) {
                t = 0.0f;
                s = std::clamp(-c / a, 0.0f, 1.0f);
            } else if (t > 1.0f) {
                t = 1.0f;
                s = std::clamp((b - c) / a, 0.0f, 1.0f);
            }
        }
    }

    // turn s and t into actual points
    c1 = p1 + d1 * s;
    c2 = p2 + d2 * t;
}

// checks if a collider is colliding with another collider and outputs a contact point
bool Collider::IsCollidingWith(Collider& o, Contact& out)
{
    using S = ColliderShape;
    S a = m_shape, b = o.m_shape;

    // for helpers called with swapped arguments, flip the normal so it always points this -> o
    auto flip = [&](bool hit) { if (hit) out.normal = out.normal * -1.f; return hit; };

    // same shape vs same shape, no swapping needed
    if (a == S::Box      && b == S::Box)      return CheckBoxIntersection(*this, o, out);
    if (a == S::Sphere   && b == S::Sphere)   return CheckSphereIntersection(*this, o, out);
    if (a == S::Capsule  && b == S::Capsule)  return CheckCapsuleIntersection(*this, o, out);
    if (a == S::Cylinder && b == S::Cylinder) return CheckCylinderIntersection(*this, o, out);

    // sphere and box (the helper wants the sphere first)
    if (a == S::Sphere   && b == S::Box)      return CheckSphereBoxIntersection(*this, o, out);
    if (a == S::Box      && b == S::Sphere)   return flip(CheckSphereBoxIntersection(o, *this, out));

    // sphere and capsule (the helper wants the capsule first)
    if (a == S::Capsule  && b == S::Sphere)   return CheckSphereCapsuleIntersection(*this, o, out);
    if (a == S::Sphere   && b == S::Capsule)  return flip(CheckSphereCapsuleIntersection(o, *this, out));

    // box and capsule (the helper wants the box first)
    if (a == S::Box      && b == S::Capsule)  return CheckBoxCapsuleIntersection(*this, o, out);
    if (a == S::Capsule  && b == S::Box)      return flip(CheckBoxCapsuleIntersection(o, *this, out));

    // box and cylinder (the helper wants the box first)
    if (a == S::Box      && b == S::Cylinder) return CheckBoxCylinderIntersection(*this, o, out);
    if (a == S::Cylinder && b == S::Box)      return flip(CheckBoxCylinderIntersection(o, *this, out));

    // sphere and cylinder (the helper wants the sphere first)
    if (a == S::Sphere   && b == S::Cylinder) return CheckSphereCylinderIntersection(*this, o, out);
    if (a == S::Cylinder && b == S::Sphere)   return flip(CheckSphereCylinderIntersection(o, *this, out));

    // cylinder and capsule (the helper wants the cylinder first)
    if (a == S::Cylinder && b == S::Capsule)  return CheckCylinderCapsuleIntersection(*this, o, out);
    if (a == S::Capsule  && b == S::Cylinder) return flip(CheckCylinderCapsuleIntersection(o, *this, out));

    // no pair matched (a shape we don't support yet, like meshes)
    return false;
}

// checks if a collider is colliding with another collider
bool Collider::IsCollidingWith(Collider& collider)
{
    Contact discard;
    return IsCollidingWith(collider, discard);
}

// checks if two box colliders are intersecting
bool Collider::CheckBoxIntersection(Collider& a, Collider& b, Contact& out) {
    
    // gets the world bounding boxes
    AABB A = a.getWorldAABB();
    AABB B = b.getWorldAABB();

    // gets the bound of the a and b colliders and gets the difference in their distances
    float ox = std::min(A.m_max.x, B.m_max.x) - std::max(A.m_min.x, B.m_min.x);
    float oy = std::min(A.m_max.y, B.m_max.y) - std::max(A.m_min.y, B.m_min.y);
    float oz = std::min(A.m_max.z, B.m_max.z) - std::max(A.m_min.z, B.m_min.z);
    if (ox <= 0 || oy <= 0 || oz <= 0) return false; // returns false if they dont intersect

    // gets the center of a and b
    fm::vec3 ca = (A.m_min + A.m_max) * 0.5f;
    fm::vec3 cb = (B.m_min + B.m_max) * 0.5f;

    // sets the data for the contact point
    if (ox <= oy && ox <= oz) {
        out.penetration  = ox;
        out.normal = fm::vec3(ca.x < cb.x ? 1.f : -1.f, 0.f, 0.f);
    } else if (oy <= oz) {
        out.penetration  = oy;
        out.normal = fm::vec3(0.f, ca.y < cb.y ? 1.f : -1.f, 0.f);
    } else {
        out.penetration  = oz;
        out.normal = fm::vec3(0.f, 0.f, ca.z < cb.z ? 1.f : -1.f);
    }
    // returns a collission
    return true;
}

// checks if two spheres intersect
bool Collider::CheckSphereIntersection(Collider& a, Collider& b, Contact& out) {
    // Gets center of sphere A and sphere B accounting for offset
    fm::vec3 ca = a.getWorldCenter();
    fm::vec3 cb = b.getWorldCenter();
    float ra = a.getWorldRadius();
    float rb = b.getWorldRadius();

    // gets distance between the centers and returns false if they're too far apart
    fm::vec3 d = cb - ca;
    float distSq = d.dot(d);
    float sum = ra + rb;
    if (distSq > sum * sum) return false;

    // gets the square root of the distance to find how far the penetration is
    float dist = std::sqrt(distSq);
    out.penetration  = sum - dist;

    // if the centres coincide there's no direction, so pick any
    out.normal = dist > 1e-6f ? d * (1.f / dist) : fm::vec3(0.f, 1.f, 0.f);
    return true;
}

// checks if a sphere is colliding with a box
bool Collider::CheckSphereBoxIntersection(fm::vec3 c, float r, Collider& b, Contact& out)
{    // gets the world data of the colliders
    AABB box   = b.getWorldAABB();

    // finds the closest between the box and spheres
    fm::vec3 closest(std::clamp(c.x, box.m_min.x, box.m_max.x),
                     std::clamp(c.y, box.m_min.y, box.m_max.y),
                     std::clamp(c.z, box.m_min.z, box.m_max.z));

    // gets the closest point on the two colliders
    fm::vec3 closestPoint;
    fm::vec3 d = c - closest;     
    
    //gets the square of the distance and if they're not intersecting return false
    float distSq = d.dot(d);
    if (distSq > r * r) return false;

    // returns a hit and sets the details
    if (distSq > 1e-12f) {
        float dist = std::sqrt(distSq);
        out.normal = d * (-1.f / dist);
        out.penetration  = r - dist;
        return true;
    }

    
    // calculates a just in case for really small distances
    float dx0 = c.x - box.m_min.x, dx1 = box.m_max.x - c.x;
    float dy0 = c.y - box.m_min.y, dy1 = box.m_max.y - c.y;
    float dz0 = c.z - box.m_min.z, dz1 = box.m_max.z - c.z;

    float best = dx0; fm::vec3 exitDir(-1.f, 0.f, 0.f);
    if (dx1 < best) { best = dx1; exitDir = fm::vec3( 1.f, 0.f, 0.f); }
    if (dy0 < best) { best = dy0; exitDir = fm::vec3(0.f, -1.f, 0.f); }
    if (dy1 < best) { best = dy1; exitDir = fm::vec3(0.f,  1.f, 0.f); }
    if (dz0 < best) { best = dz0; exitDir = fm::vec3(0.f, 0.f, -1.f); }
    if (dz1 < best) { best = dz1; exitDir = fm::vec3(0.f, 0.f,  1.f); }

    out.normal = exitDir * -1.f;     
    out.penetration = best + r;
    return true;
}

// checks if a sphere collider is colliding with a box collider, wraps the vec3 version above
bool Collider::CheckSphereBoxIntersection(Collider& a, Collider& b, Contact& out) {
    return CheckSphereBoxIntersection(a.getWorldCenter(), a.getWorldRadius(), b, out);
}

// checks if a sphere and a capsule intersect, normal points capsule -> sphere
bool Collider::CheckSphereCapsuleIntersection(Collider& capsule, Collider& sphere, Contact& out)
{
    // gets the two ends of the capsule's inner line segment
    fm::vec3 pointA, pointB;
    getSegmentEnds(capsule, pointA, pointB);

    // finds the point on that segment closest to the sphere's center
    fm::vec3 sphereCenter = sphere.getWorldCenter();
    fm::vec3 closest = fm::vec3::closestPoint(pointA, pointB, sphereCenter);

    // a capsule is just a segment with a radius, so they touch when the distance is less than both radii added
    float rc = capsule.getWorldRadius();
    float rs = sphere.getWorldRadius();
    float sumRad = rc + rs;

    // distance from the segment to the sphere center, squared so we can skip the sqrt for misses
    fm::vec3 d = sphereCenter - closest;
    float distSq = d.dot(d);

    // too far apart, no collision
    if (distSq > sumRad * sumRad) return false;

    // penetration is how far the radii overlap, normal points capsule -> sphere
    // (points up if the centers are exactly on top of each other)
    float dist = std::sqrt(distSq);
    out.penetration = sumRad - dist;
    out.normal = dist > 1e-6f ? d * (1.0f / dist) : fm::vec3(0.f, 1.f, 0.f);
    return true;
}

// checks if a box and a capsule intersect, normal points box -> capsule
bool Collider::CheckBoxCapsuleIntersection(Collider& box, Collider& capsule, Contact& out) {
    // gets the capsule's inner line segment
    fm::vec3 a, b;
    getSegmentEnds(capsule, a, b);

    // gets the box in world space
    AABB bBox = box.getWorldAABB();

    // finds the point on the box closest to the middle of the capsule
    fm::vec3 segMid = (a + b) * 0.5f;
    fm::vec3 boxClosestToMid(std::clamp(segMid.x, bBox.m_min.x, bBox.m_max.x),
                             std::clamp(segMid.y, bBox.m_min.y, bBox.m_max.y),
                             std::clamp(segMid.z, bBox.m_min.z, bBox.m_max.z));

    // then finds the point on the capsule's segment closest to THAT point
    fm::vec3 closestOnSeg = fm::vec3::closestPoint(a, b, boxClosestToMid);

    // treats that point as a sphere center with the capsule's radius and reuses the sphere/box test
    // the sphere test gives sphere -> box, we want box -> capsule so flip it
    bool hit = CheckSphereBoxIntersection(closestOnSeg, capsule.getWorldRadius(), box, out);
    if (hit) out.normal = out.normal * -1.f;   
    return hit;
}

// checks if two capsules intersect, normal points A -> B
bool Collider::CheckCapsuleIntersection(Collider& capA, Collider& capB, Contact& out) {
    // gets both capsules' inner segments
    fm::vec3 a1, b1, a2, b2;
    getSegmentEnds(capA, a1, b1);
    getSegmentEnds(capB, a2, b2);

    // finds the closest points between the two segments
    fm::vec3 c1, c2;
    closestPointBetweenSegments(a1, b1, a2, b2, c1, c2);

    // they touch if those points are closer than both radii added together
    fm::vec3 d = c2 - c1;
    float distSq = d.dot(d);
    float rSum = capA.getWorldRadius() + capB.getWorldRadius();

    if (distSq > rSum * rSum) return false;

    // penetration is the radii overlap, normal points A -> B (up if the segments are on top of each other)
    float dist = std::sqrt(std::max(0.0f, distSq));
    out.penetration = rSum - dist;
    out.normal = dist > 1e-6f ? d * (1.0f / dist) : fm::vec3(0.f, 1.f, 0.f);
    return true;
}

// checks if a cylinder and a capsule intersect, normal points cylinder -> capsule
// (cylinders are treated as a segment with a radius here, so the flat caps act a bit rounded)
bool Collider::CheckCylinderCapsuleIntersection(Collider& cyl, Collider& cap, Contact& out) {
    // gets both inner segments
    fm::vec3 a, b;
    getSegmentEnds(cyl, a, b);
    fm::vec3 ca, cb;
    getSegmentEnds(cap, ca, cb);

    // finds the closest points between the two segments
    fm::vec3 c1, c2;
    closestPointBetweenSegments(a, b, ca, cb, c1, c2);

    // they touch if those points are closer than both radii added together
    fm::vec3 d = c2 - c1;
    float distSq = d.dot(d);
    float rSum = cyl.getWorldRadius() + cap.getWorldRadius();

    if (distSq > rSum * rSum) return false;

    // penetration is the radii overlap, normal points cylinder -> capsule
    float dist = std::sqrt(std::max(0.0f, distSq));
    out.penetration = rSum - dist;
    out.normal = dist > 1e-6f ? d * (1.0f / dist) : fm::vec3(0.f, 1.f, 0.f);
    return true;
}

// checks if a box and a cylinder intersect using the separating axis theorem, normal points box -> cylinder
// if there's an axis where the shapes don't overlap, they can't be touching
bool Collider::CheckBoxCylinderIntersection(Collider& box, Collider& cyl, Contact& out)
{
    // gets the cylinder's cap centers and works out its height and direction
    fm::vec3 a, b;
    getSegmentEnds(cyl, a, b);
    fm::vec3 ab = b - a;
    float h = std::sqrt(ab.dot(ab));
    if (h < 1e-6f) return false; // zero height cylinder, nothing to hit

    // the cylinder's axis, its radius and its center
    fm::vec3 axis = ab * (1.f / h);
    float r = cyl.getWorldRadius();
    fm::vec3 cc = (a + b) * 0.5f;

    // the box in world space, its center and half the size in each direction
    AABB B = box.getWorldAABB();
    fm::vec3 bc = (B.m_min + B.m_max) * 0.5f;
    fm::vec3 be = (B.m_max - B.m_min) * 0.5f;

    // keeps track of the axis with the least overlap, that's the way to push out
    float bestDepth = FLT_MAX;
    fm::vec3 bestN(0.f, 1.f, 0.f);

    // tests one axis, returns false if the shapes are separated on it (so no collision)
    auto testAxis = [&](fm::vec3 n) -> bool {
        // normalises the axis and skips it if it has no length
        float len = std::sqrt(n.dot(n));
        if (len < 1e-6f) return true;
        n = n * (1.f / len);

        // how far each shape reaches along this axis from its center
        float boxR = be.x * std::abs(n.x) + be.y * std::abs(n.y) + be.z * std::abs(n.z);
        float ca   = std::abs(n.dot(axis));
        float cylR = (h * 0.5f) * ca + r * std::sqrt(std::max(0.f, 1.f - ca * ca));

        // distance between the centers along the axis, and how much the shapes overlap on it
        float dist    = n.dot(cc - bc);
        float overlap = boxR + cylR - std::abs(dist);
        if (overlap <= 0.f) return false; // a gap on this axis, so no collision

        // remembers this axis if it's the shallowest so far, pointing it box -> cylinder
        if (overlap < bestDepth) {
            bestDepth = overlap;
            bestN = dist >= 0.f ? n : n * -1.f;
        }
        return true;
    };

    // tests the box's three axes and the cylinder's axis
    if (!testAxis(fm::vec3(1.f, 0.f, 0.f))) return false;
    if (!testAxis(fm::vec3(0.f, 1.f, 0.f))) return false;
    if (!testAxis(fm::vec3(0.f, 0.f, 1.f))) return false;
    if (!testAxis(axis)) return false;

    // tests the sideways direction from the cylinder's axis towards the closest point on the box
    fm::vec3 p(std::clamp(cc.x, B.m_min.x, B.m_max.x),
               std::clamp(cc.y, B.m_min.y, B.m_max.y),
               std::clamp(cc.z, B.m_min.z, B.m_max.z));
    fm::vec3 v = p - cc;
    v = v - axis * v.dot(axis); // removes the part along the axis, leaving just the sideways bit
    if (!testAxis(v)) return false;

    // no axis separated them, so use the shallowest one as the contact
    out.normal = bestN;
    out.penetration = bestDepth;
    return true;
}

// checks if a sphere and a cylinder intersect, normal points sphere -> cylinder
bool Collider::CheckSphereCylinderIntersection(Collider& sphere, Collider& cyl, Contact& out)
{
    // gets the cylinder's cap centers and works out its height and direction
    fm::vec3 a, b;
    getSegmentEnds(cyl, a, b);
    fm::vec3 ab = b - a;
    float h = std::sqrt(ab.dot(ab));
    if (h < 1e-6f) return false; // zero height cylinder, nothing to hit

    // the cylinder's axis, half height, radius and the sphere's radius
    fm::vec3 axis = ab * (1.f / h);
    float hh = h * 0.5f;
    float r  = cyl.getWorldRadius();
    float sr = sphere.getWorldRadius();

    // the cylinder's center and the sphere's center
    fm::vec3 cc = (a + b) * 0.5f;
    fm::vec3 s  = sphere.getWorldCenter();

    // sphere center relative to the cylinder: t is how far along the axis, rd is how far out sideways
    fm::vec3 rel = s - cc;
    float t = rel.dot(axis);
    fm::vec3 rv = rel - axis * t;
    float rd = std::sqrt(rv.dot(rv));

    // the direction pointing sideways out from the axis
    // if the sphere is right on the axis there isn't one, so pick any direction perpendicular to it
    fm::vec3 radialDir;
    if (rd > 1e-6f) {
        radialDir = rv * (1.f / rd);
    } else {
        fm::vec3 ref = std::abs(axis.y) < 0.9f ? fm::vec3(0.f, 1.f, 0.f) : fm::vec3(1.f, 0.f, 0.f);
        radialDir = ref - axis * ref.dot(axis);
        radialDir = radialDir * (1.f / std::sqrt(radialDir.dot(radialDir)));
    }

    // is the sphere's center between the caps, and inside the cylinder's radius?
    bool insideAxial  = std::abs(t) <= hh;
    bool insideRadial = rd <= r;

    if (insideAxial && insideRadial) {
        // the center is inside the solid cylinder, so leave through whichever surface is nearest
        float toCap  = hh - std::abs(t);
        float toSide = r - rd;

        fm::vec3 exitDir;           // points cylinder -> sphere
        float depthToSurface;
        if (toCap < toSide) {
            // closer to a flat cap
            exitDir = axis * (t >= 0.f ? 1.f : -1.f);
            depthToSurface = toCap;
        } else {
            // closer to the curved side
            exitDir = radialDir;
            depthToSurface = toSide;
        }

        // flip it to sphere -> cylinder, and add the radius since the whole sphere has to clear the surface
        out.normal = exitDir * -1.f;
        out.penetration = depthToSurface + sr;
        return true;
    }

    // the center is outside, so find the closest point on the solid cylinder to it
    float tc = std::clamp(t, -hh, hh);
    float rc = std::min(rd, r);
    fm::vec3 closest = cc + axis * tc + radialDir * rc;

    // touching if that point is within the sphere's radius
    fm::vec3 d = s - closest;
    float distSq = d.dot(d);
    if (distSq > sr * sr) return false;

    // penetration is how far the sphere reaches past the surface, normal points sphere -> cylinder
    float dist = std::sqrt(distSq);
    out.penetration = sr - dist;
    out.normal = dist > 1e-6f ? d * (-1.f / dist) : (radialDir * -1.f);
    return true;
}

// checks if two cylinders intersect, normal points A -> B
// (treats them as segments with a radius, so stacking is a bit spongy)
bool Collider::CheckCylinderIntersection(Collider& cylA, Collider& cylB, Contact& out) {
    // gets both inner segments
    fm::vec3 a1, b1, a2, b2;
    getSegmentEnds(cylA, a1, b1);
    getSegmentEnds(cylB, a2, b2);

    // finds the closest points between the two segments
    fm::vec3 c1, c2;
    closestPointBetweenSegments(a1, b1, a2, b2, c1, c2);

    // they touch if those points are closer than both radii added together
    fm::vec3 d = c2 - c1;
    float distSq = d.dot(d);
    float rSum = cylA.getWorldRadius() + cylB.getWorldRadius();

    if (distSq > rSum * rSum) return false;

    // penetration is the radii overlap, normal points A -> B
    float dist = std::sqrt(std::max(0.0f, distSq));
    out.penetration = rSum - dist;
    out.normal = dist > 1e-6f ? d * (1.0f / dist) : fm::vec3(0.f, 1.f, 0.f);
    return true;
}

// resolves two collisions
void CollisionWorld::resolve(Collider* a, Collider* b, const Collider::Contact& c) {
    
    // gets the rigidbody of both the colliders
    Physics::Rigidbody* ra = a->m_rb;
    Physics::Rigidbody* rb = b->m_rb;

    // gets an infinite mass if it doesnt have a rigid body
    float ia = ra ? ra->getInverseMass() : 0.f;
    float ib = rb ? rb->getInverseMass() : 0.f;
    float total = ia + ib;

    // both colliders are static
    if (total == 0.f) return;   

    // create a correction magnitude which corrects a percentage of the penetration
    float safePenetration = std::min(c.penetration, 0.2f);
   const float slop = 0.01f;
    const float percent = 0.4f; 
    float correctionMag = std::max(0.0f, safePenetration - slop) / total * percent;
    
    // 1. push apart objects
    if (ra) a->getEntity().transform.addPosition(c.normal * (-correctionMag * ia));
    if (rb) b->getEntity().transform.addPosition(c.normal * ( correctionMag * ib));

    // 2. bouncyyyyy i love trampolining https://klipy.com/gifs/gmail-dog-3
    fm::vec3 va = ra ? ra->getVelocity() : fm::vec3::zero();
    fm::vec3 vb = rb ? rb->getVelocity() : fm::vec3::zero();
    float vn = (vb - va).dot(c.normal);             
    if (vn > 0.f) return; // no bouncy :c               

    // makes the bounce resist over time
    float e = std::min(ra ? ra->m_bounciness : 0.0f, rb ? rb->m_bounciness : 0.0f); // Default to 0 for stability
    float restThreshold = 9.8f * (1.0f / PhysicsImpl::tps) * 3.0f; // Slightly wider threshold

    if (std::abs(vn) < restThreshold) {
        e = 0.0f; // Kill bounce completely
    }

    float j = -(1.f + e) * vn / total;              // kill tiny bounces so resting objects settle

    // adds a velocity change to the rigidbody
    if (std::abs(vn) < restThreshold) {
        j = -vn / total; 
    }

    if (ra) ra->addForce(-c.normal * (j * ia), Rigidbody::ForceMode::VelocityChange);
    if (rb) rb->addForce( c.normal * (j * ib), Rigidbody::ForceMode::VelocityChange);
}

// draws the debug shape for whatever kind of collider this is
void Collider::draw() {

    // one line renderer shared between every collider
    static DebugLine line;

    switch (m_shape) {
        case (ColliderShape::Box) : {
            _drawBox(m_min, m_max, entity.getRealMatrix(), line);
            break;
        }
        case (ColliderShape::Sphere) : {
            _drawSphere(m_center, m_radius, entity.getRealMatrix(), line);
            break;
        }
        case (ColliderShape::Cylinder) : {
            _drawCylinder(m_radius, m_height, entity.getRealMatrix(), line);
            break;
        }
        case (ColliderShape::Capsule) : {
            _drawCapsule(m_radius, m_height, entity.getRealMatrix(), line);
            break;
        }
    }
}

// draws a wireframe box from a min and max corner
void Collider::_drawBox(fm::vec3 min, fm::vec3 max, fm::mat4 model, Debug::DebugLine& line) {

    std::vector<Point> points;

    // the four corners on the min z face
    points.push_back({min, m_drawCol});
    points.push_back({fm::vec3(max.x, min.y, min.z), m_drawCol});
    points.push_back({fm::vec3(max.x, max.y, min.z), m_drawCol});
    points.push_back({fm::vec3(min.x, max.y, min.z), m_drawCol});

    // the four corners on the max z face
    points.push_back({fm::vec3(min.x, min.y, max.z), m_drawCol});
    points.push_back({fm::vec3(max.x, min.y, max.z), m_drawCol});
    points.push_back({max, m_drawCol});
    points.push_back({fm::vec3(min.x, max.y, max.z), m_drawCol});

    // one continuous line that traces every edge of the box
    line.draw({
        points[0],
        points[1],
        points[2],
        points[3],
        points[0],
        points[4],
        points[5],
        points[1],
        points[5],
        points[6],
        points[2],
        points[6],
        points[7],
        points[3],
        points[7],
        points[4]
    }, model);
}

void Collider::_drawSphere(fm::vec3 center, fm::vec3 scaledRadius, fm::mat4 model, Debug::DebugLine &line) {
    
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

    line.draw(xzPoints, model); // draw ring 1

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

    line.draw(xyPoints, model); // draw ring 2

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
    line.draw(yzPoints, model); // draw ring 3
}

void Collider::_drawCylinder(float r, float h, fm::mat4 model, Debug::DebugLine& line) {
    const int segments = 24;
    const float angleStep = 2.0f * fm::PI / static_cast<float>(segments);
    float halfH = h * 0.5f;

    std::vector<Point> topRing;
    std::vector<Point> bottomRing;
    std::vector<Point> pillars;

    topRing.reserve(segments + 1);
    bottomRing.reserve(segments + 1);

    for (int i = 0; i <= segments; ++i) {
        float angle = i * angleStep;
        float x = std::cos(angle) * r;
        float z = std::sin(angle) * r;

        fm::vec3 topPos(x, halfH, z);
        fm::vec3 botPos(x, -halfH, z);

        topRing.push_back(Point(topPos, m_drawCol));
        bottomRing.push_back(Point(botPos, m_drawCol));

        // Draw 4 vertical pillar lines at quarter marks
        if (i % (segments / 4) == 0) {
            pillars.push_back(Point(topPos, m_drawCol));
            pillars.push_back(Point(botPos, m_drawCol));
        }
    }

    line.draw(topRing, model);
    line.draw(bottomRing, model);
    line.draw(pillars, model);
}

void Collider::_drawCapsule(float r, float h, fm::mat4 model, Debug::DebugLine& line) {
    const int segments = 24;
    const float angleStep = 2.0f * fm::PI / static_cast<float>(segments);
    float halfCylH = std::max(0.0f, (h * 0.5f) - r);

    // 1. Draw central cylinder body rings
    std::vector<Point> topRing, bottomRing, pillars;
    for (int i = 0; i <= segments; ++i) {
        float angle = i * angleStep;
        float x = std::cos(angle) * r;
        float z = std::sin(angle) * r;

        fm::vec3 topPos(x, halfCylH, z);
        fm::vec3 botPos(x, -halfCylH, z);

        topRing.push_back(Point(topPos, m_drawCol));
        bottomRing.push_back(Point(botPos, m_drawCol));

        if (i % (segments / 4) == 0) {
            pillars.push_back(Point(topPos, m_drawCol));
            pillars.push_back(Point(botPos, m_drawCol));
        }
    }

    line.draw(topRing, model);
    line.draw(bottomRing, model);
    line.draw(pillars, model);

    // 2. Draw hemispherical caps (top and bottom semi-circles)
    std::vector<Point> topCapXZ, bottomCapXZ;
    for (int i = 0; i <= segments; ++i) {
        float angle = i * angleStep;
        // Using sphere drawing logic offset by halfCylH
        fm::vec3 topArc(std::cos(angle) * r, halfCylH, std::sin(angle) * r);
        fm::vec3 botArc(std::cos(angle) * r, -halfCylH, std::sin(angle) * r);
        topCapXZ.push_back(Point(topArc, m_drawCol));
        bottomCapXZ.push_back(Point(botArc, m_drawCol));
    }
    line.draw(topCapXZ, model);
    line.draw(bottomCapXZ, model);
}


// returns the colliders ID
ColliderID Collider::getColliderID() {
    return m_ColliderID;
}

void Collider::setColour(fm::vec4 col) {
    m_materials[0]->m_data.m_albedoFac = col;
    //m_program->useProgram();
    //m_program->setUniform("boxCol", col);
}


// initializes a collider
void Collider::init() {

    // get the next box id
    m_ColliderID = s_colliderIDPool.getNewID();

    // create a static material to satisfy IRenderable.
    // this does not need to exist really. it just holds 
    // colour. 
    m_materials.push_back(Material::create());
    m_material->m_data.m_albedoFac = {1.0f, 0.9f, 0.8f, 1.0f};

    // gets the rigidbody from the entity and sets the static variable based on that then creates the world collider
    m_rb = getEntity().getComponent<Physics::Rigidbody>();
    m_isStatic = (m_rb == nullptr);
    CollisionWorld::get().add(this);
    entity.transform.m_lockTransform = m_isStatic;
}

// DESTROYS EVERYTHING MWHAHAHA and destroys the collider
void Collider::onDestroy() {

    // removes the world collider
    CollisionWorld::get().remove(this);
}

// checks for a ray intersection
bool Collider::checkRayIntersection(const Ray& ray, RaycastHit& outRaycastHit) {

    // gets the entity and the hit box
    outRaycastHit.m_hitEntity = &getEntity();
    outRaycastHit.m_hitBox = this;

    // some nerdy maths shit
    fm::mat4 worldMatrixInversed = outRaycastHit.m_hitEntity->transform.getGlobalMatrix().inversed();

    // construct our local ray 
    Ray localRay(
        ray.m_origin.transformed(worldMatrixInversed), // local origin
        ray.m_direction.transformed(worldMatrixInversed, true) // local direction
    );

    // get bounding box from entity. 
    Collider& boundingBox = *outRaycastHit.m_hitBox;

    // does a ray check depending on the shape
    switch(m_shape)
    {
        case ColliderShape::Box:
            return rayAABoxIntersection(localRay, this, outRaycastHit);
        case ColliderShape::Sphere:
            return rayAASphereIntersection(localRay, this, outRaycastHit);
        case ColliderShape::Cylinder:
            return rayAACylinderIntersection(localRay, this, outRaycastHit);
        case ColliderShape::Capsule:
            return rayAACapsuleIntersection(localRay, this, outRaycastHit);
        default: 
            return false;
    }
}


// returns if a ray intersects a box
bool Collider::rayAABoxIntersection(const Ray& ray, const Collider* box, RaycastHit& outRaycastHit) {

    float tNearMax = -std::numeric_limits<float>::max();
    float tFarMin  =  std::numeric_limits<float>::max();

    // "slab" refers to the infinite plane of an axis.
    // the X slab is just the straight (1,0,0) axis that
    // extends from -infinity to +infinity.

    fm::vec3 entityScale = outRaycastHit.m_hitEntity->transform.getGlobalScale();
    // idfk dude if the scale in an axis is smaller than this
    // then it detects a hit all the time so like check this
    float minAxisScale = 0.01;

    // test intersection per axis
    for (int i = 0; i < 3; ++i) {

        // if the axis is smaller than the minimum allowed scale, 
        // just return to avoid some weird bugs
        if (entityScale[i] < minAxisScale) return false;

        float origin = ray.m_origin[i];
        float dir = ray.m_direction[i];
        
        float minVal = box->m_min[i];
        float maxVal = box->m_max[i];

        // avoid division by zero
        if (std::abs(dir) < 1e-8f) { // 1e-8f = 0.00000001 
            // dir is effectively 0

            // is ray origin outside this axis' slab?
            if (origin < minVal || origin > maxVal)
                return false; // yes, no intersection happened.
        
        } 
        else {
            // dir is not 0, continue

            // point where the ray entered. 
            float boxEntry = (minVal - origin) / dir;

            // point where the ray left.
            float boxExit  = (maxVal - origin) / dir;

            if (boxEntry > boxExit) std::swap(boxEntry, boxExit);

            // nearest hit point to the ray origin
            tNearMax = std::max(tNearMax, boxEntry);

            // furthest hit point to the origin
            tFarMin  = std::min(tFarMin, boxExit);

            // if near intersection is greater than far intersection, the box was missed.
            if (tNearMax > tFarMin)
                return false;
        }
    }

    RaycastHitInfo& outHitInfo = outRaycastHit.m_hitInfo;

    // check for box intersection
    if (tNearMax <= tFarMin && outHitInfo.m_distance > tNearMax) {
        // we hit!!
        // set some values and then we GET OUTTA HERE!!!

        fm::mat4 worldMatrix = outRaycastHit.m_hitEntity->transform.getGlobalMatrix();

        outHitInfo.m_hitPoint    = (ray.m_origin + ray.m_direction * tNearMax).transformed(worldMatrix);
        outHitInfo.m_farHitPoint = (ray.m_origin + ray.m_direction * tFarMin).transformed(worldMatrix);
        outHitInfo.m_distance    = tNearMax;
        outHitInfo.m_hasHit      = true;

        // GET OUTTA HERE!!!
        return true;
    }

    return false;
}


// returns whether a ray intersected a sphere
bool Collider::rayAASphereIntersection(const Ray& ray, const Collider* sphere, RaycastHit& outRaycastHit) {

    // get sphere center and length of origin to sphere center (negative)
    fm::vec3 sphereCenter = sphere->m_center + sphere->m_offset;
    fm::vec3 length = ray.m_origin - sphereCenter;

    // discriminants
    float rdr = ray.m_direction.dot(ray.m_direction); // red dead redemption (raydir dot raydir)
    float ldr = 2.0 * length.dot(ray.m_direction); // left dead redemption (length dot raydir)
    float ldl = length.dot(length) - (sphere->m_radius * sphere->m_radius); // left dead ledemption (length dot length)

    // if the discriminant is less than zero, the ray missed 
    float discriminant = (ldr * ldr) - (4.0f * rdr * ldl);
    if (discriminant < 0) return false;

    // discriminant square root
    float discSqrt = std::sqrt(discriminant);
    float rdr2 = 2.0f * rdr; // red dead redemption 2

    // maths i dont quite understand -faye
    // nearest hit point
    float near = (-ldr - discSqrt) / rdr2;
    // furthest hit point
    float far = (-ldr + discSqrt) / rdr2;

    // if near is negative, the ray missed.
    if (near < 0.0f) {
        near = far;
        if (near < 0.0f)
            return false;
    }


    RaycastHitInfo& outHitInfo = outRaycastHit.m_hitInfo;

    // get the world matrix
    fm::mat4 worldMatrix = outRaycastHit.m_hitEntity->transform.getGlobalMatrix();

    // set some hit info
    outHitInfo.m_hitPoint    = (ray.m_origin + ray.m_direction * near).transformed(worldMatrix);
    outHitInfo.m_farHitPoint = (ray.m_origin + ray.m_direction * far).transformed(worldMatrix);
    outHitInfo.m_distance    = near;
    outHitInfo.m_hasHit      = true;

    return true;
}

bool Collider::rayAACylinderIntersection(const Ray& ray, const Collider* cyl, RaycastHit& outRaycastHit) {
    float r = cyl->m_radius;
    float halfH = cyl->m_height * 0.5f;

    float tMin = std::numeric_limits<float>::max();
    bool hitFound = false;

    // 1. Test infinite cylinder body (XZ plane quadratic)
    float ox = ray.m_origin.x;
    float oz = ray.m_origin.z;
    float dx = ray.m_direction.x;
    float dz = ray.m_direction.z;

    float a = dx * dx + dz * dz;
    float b = 2.0f * (ox * dx + oz * dz);
    float c = ox * ox + oz * oz - r * r;

    if (std::abs(a) > 1e-8f) {
        float discriminant = b * b - 4.0f * a * c;
        if (discriminant >= 0.0f) {
            float discSqrt = std::sqrt(discriminant);
            float t0 = (-b - discSqrt) / (2.0f * a);
            float t1 = (-b + discSqrt) / (2.0f * a);

            if (t0 > t1) std::swap(t0, t1);

            // Check t0
            if (t0 > 0.0f) {
                float yHit = ray.m_origin.y + t0 * ray.m_direction.y;
                if (yHit >= -halfH && yHit <= halfH) {
                    tMin = std::min(tMin, t0);
                    hitFound = true;
                }
            }
            // Check t1 if t0 was missed or out of bounds
            if (t1 > 0.0f && !hitFound) {
                float yHit = ray.m_origin.y + t1 * ray.m_direction.y;
                if (yHit >= -halfH && yHit <= halfH) {
                    tMin = std::min(tMin, t1);
                    hitFound = true;
                }
            }
        }
    }

    // 2. Test flat end caps (y = -halfH and y = +halfH)
    float capYs[2] = { -halfH, halfH };
    for (float capY : capYs) {
        if (std::abs(ray.m_direction.y) > 1e-8f) {
            float tCap = (capY - ray.m_origin.y) / ray.m_direction.y;
            if (tCap > 0.0f && tCap < tMin) {
                fm::vec3 hitPoint = ray.m_origin + ray.m_direction * tCap;
                if (hitPoint.x * hitPoint.x + hitPoint.z * hitPoint.z <= r * r) {
                    tMin = tCap;
                    hitFound = true;
                }
            }
        }
    }

    if (!hitFound) return false;

    RaycastHitInfo& outHitInfo = outRaycastHit.m_hitInfo;
    fm::mat4 worldMatrix = outRaycastHit.m_hitEntity->transform.getGlobalMatrix();

    outHitInfo.m_hitPoint = (ray.m_origin + ray.m_direction * tMin).transformed(worldMatrix);
    outHitInfo.m_distance = tMin;
    outHitInfo.m_hasHit = true;
    return true;
}

bool Collider::rayAACapsuleIntersection(const Ray& ray, const Collider* cap, RaycastHit& outRaycastHit) {
    float r = cap->m_radius;
    float totalH = cap->m_height;
    float halfCylH = std::max(0.0f, (totalH * 0.5f) - r);

    float tMin = std::numeric_limits<float>::max();
    bool hitFound = false;

    // 1. Test central cylinder body
    {
        float ox = ray.m_origin.x;
        float oz = ray.m_origin.z;
        float dx = ray.m_direction.x;
        float dz = ray.m_direction.z;

        float a = dx * dx + dz * dz;
        float b = 2.0f * (ox * dx + oz * dz);
        float c = ox * ox + oz * oz - r * r;

        if (std::abs(a) > 1e-8f) {
            float disc = b * b - 4.0f * a * c;
            if (disc >= 0.0f) {
                float discSqrt = std::sqrt(disc);
                float t0 = (-b - discSqrt) / (2.0f * a);
                float t1 = (-b + discSqrt) / (2.0f * a);
                if (t0 > t1) std::swap(t0, t1);

                if (t0 > 0.0f) {
                    float y = ray.m_origin.y + t0 * ray.m_direction.y;
                    if (y >= -halfCylH && y <= halfCylH) { tMin = std::min(tMin, t0); hitFound = true; }
                }
                if (t1 > 0.0f && !hitFound) {
                    float y = ray.m_origin.y + t1 * ray.m_direction.y;
                    if (y >= -halfCylH && y <= halfCylH) { tMin = std::min(tMin, t1); hitFound = true; }
                }
            }
        }
    }

    // 2. Test top and bottom spherical caps
    fm::vec3 capCenters[2] = { fm::vec3(0.f, -halfCylH, 0.f), fm::vec3(0.f, halfCylH, 0.f) };
    for (const auto& center : capCenters) {
        fm::vec3 length = ray.m_origin - center;
        float rdr = ray.m_direction.dot(ray.m_direction);
        float ldr = 2.0f * length.dot(ray.m_direction);
        float ldl = length.dot(length) - (r * r);

        float disc = (ldr * ldr) - (4.0f * rdr * ldl);
        if (disc >= 0.0f) {
            float discSqrt = std::sqrt(disc);
            float near = (-ldr - discSqrt) / (2.0f * rdr);
            if (near > 0.0f && near < tMin) {
                tMin = near;
                hitFound = true;
            }
        }
    }

    if (!hitFound) return false;

    RaycastHitInfo& outHitInfo = outRaycastHit.m_hitInfo;
    fm::mat4 worldMatrix = outRaycastHit.m_hitEntity->transform.getGlobalMatrix();

    outHitInfo.m_hitPoint = (ray.m_origin + ray.m_direction * tMin).transformed(worldMatrix);
    outHitInfo.m_distance = tMin;
    outHitInfo.m_hasHit = true;
    return true;
}

// sets the colliders static boolean
void Collider::setStatic(bool s)
{
    if (m_isStatic == s) return;
    CollisionWorld::get().remove(this);
    m_isStatic = s;
    CollisionWorld::get().add(this);
    // if the collider is static, lock the transform
    // (transform can be manually unlocked, but this is
    // kind of just a warning against doing that
    entity.transform.m_lockTransform = m_isStatic;
}


// gets the bounding volumes bounds in world space
AABB Collider::getWorldAABB() {
    // gets the transform and global matrix
    Transform& t = entity.transform;
    fm::mat4 world = t.getGlobalMatrix();

    if (m_shape == ColliderShape::Capsule || m_shape == ColliderShape::Cylinder) {
        fm::vec3 a, b;
        getSegmentEnds(*this, a, b);
        float r = getWorldRadius();

        fm::vec3 lo = { std::min(a.x, b.x) - r, std::min(a.y, b.y) - r, std::min(a.z, b.z) - r };
        fm::vec3 hi = { std::max(a.x, b.x) + r, std::max(a.y, b.y) + r, std::max(a.z, b.z) + r };
        return { lo, hi };
    }

    // transform the spheres bounds based on the world space
    if (m_shape == ColliderShape::Sphere) {
        fm::vec3 c = (m_center + m_offset).transformed(world);

        // sets the space
        fm::vec3 s = t.getGlobalScale();
        float r = m_radius * std::max({ std::abs(s.x), std::abs(s.y), std::abs(s.z) });

        // new bounding volume
        return { c - fm::vec3(r, r, r), c + fm::vec3(r, r, r) };
    }

   
    // beeg box
    fm::vec3 lo( FLT_MAX,  FLT_MAX,  FLT_MAX);
    fm::vec3 hi(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    // checks the corner of the box and converts it to world space
    for (int i = 0; i < 8; ++i) {
        fm::vec3 corner((i & 1) ? m_max.x : m_min.x,
                        (i & 2) ? m_max.y : m_min.y,
                        (i & 4) ? m_max.z : m_min.z);

        fm::vec3 w = (corner + m_offset).transformed(world);

       
        lo = { std::min(lo.x, w.x), std::min(lo.y, w.y), std::min(lo.z, w.z) };
        hi = { std::max(hi.x, w.x), std::max(hi.y, w.y), std::max(hi.z, w.z) };
    }
    
    // returns bounding volume
    return { lo, hi };
}


// gets a colliders static variable
bool Collider::getStatic()
{
    return m_isStatic;
}


// rebuilds a bounding volume tree every step if it has changed
void CollisionWorld::step() {
    // rebuilds the static tree if a colliders static status has changed
    if (m_staticDirty) { m_staticTree.rebuild(m_static); m_staticDirty = false; }

    // rebuilds a dynamic bounding volume
    m_dynamicTree.rebuild(m_dynamic);

    // gets new colliders and adds them to the dynamic tree
    std::vector<Collider*> candidates;
    for (Collider* c : m_dynamic) {
        // clears vector and gets the collider in world space
        candidates.clear();
        AABB box = c->getWorldAABB();

        // check if the collider is already in a tree
        m_staticTree.query(box, candidates);
        m_dynamicTree.query(box, candidates);

        // iterates through new colliders
        for (Collider* o : candidates) {
            if (o == c) continue;

            // returns if theyre static
            if (!o->getStatic() && o->getColliderID() < c->getColliderID()) continue;

            // resolves collisions
            Collider::Contact contact;
            if (!c->IsCollidingWith(*o, contact)) continue;
            resolve(c, o, contact);
        }
    }
}

// casts a raycast in the world
bool CollisionWorld::raycast(const Physics::Ray& ray, Physics::RaycastHit& hit, LayerMask mask, float maxDist)
{
    refresh();
    // casts a raycast across the static tree
    bool a = m_staticTree.raycast(ray, hit, mask, maxDist);
    // creates a limit for the distance and a raycasthit variable
    float limit = a ? hit.m_hitInfo.m_distance : maxDist;
    Physics::RaycastHit h;
    // passes through the dynamic tree
    bool b = m_dynamicTree.raycast(ray, h, mask, limit);
    if (b) hit = h;
    return a || b;
}

// refresh a bounding volume tree
void CollisionWorld::refresh() {
    if (m_staticDirty) { m_staticTree.rebuild(m_static); m_staticDirty = false; }
    m_dynamicTree.rebuild(m_dynamic);
}

// adds a collider to the world and pushes it into a bounding volume tree
void CollisionWorld::add(Collider* c)
{
    if (!c) return; // returns to prevent nullptrs from going into the tree

    auto& list = c->getStatic() ? m_static : m_dynamic; // gets a list based on if the collider is static

    if (std::find(list.begin(), list.end(), c) != list.end()) return; // returns if the list is of size 1

    list.push_back(c); // adds the new collider
    if (c->getStatic()) m_staticDirty = true; // sets the colliders to newly static
}

// remove a collider from the world and bounding volume
void CollisionWorld::remove(Collider* c)
{
    if (!c) return; // prevents nullptr removals

    auto s = std::find(m_static.begin(), m_static.end(), c); // gets the collider from the static tree

    if (s != m_static.end()) // erase the collider from the tree
    {
        m_static.erase(s);
        m_staticDirty = true;
    }

    auto d = std::find(m_dynamic.begin(), m_static.end(), c); // gets the collider from the dynamic tree
    if (d != m_dynamic.end())
    {
        m_dynamic.erase(d); // removes the collider from the dynamic tree
    }
}