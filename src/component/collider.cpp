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

// checks if a sphere collider is colliding with a box collider, wraps the vec3 version below
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
    // (pick up if the centers are exactly on top of each other)
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

// gets a colliders static variable
bool Collider::getStatic()
{
    return m_isStatic;
}