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
    return height * std::max({ std::abs(s.x), std::abs(s.y), std::abs(s.z) });
}

void Collider::getSegmentEnds(Collider& col, fm::vec3& outA, fm::vec3& outB)
{
    // gets the world matrix of the collider
    fm::mat4 world = col.getEntity().transform.getGlobalMatrix();
    // gets half the heigh for capsule hemispherical ends
    float halfH = std::max(0.0f, (col.height * 0.5f) - col.radius);

    // sets the collider height for half for a cylinder
    if (col.m_shape == ColliderShape::Cylinder) halfH = col.height * 0.5f;

    // returns the local half heights
    fm::vec3 localA(0.0f, -halfH, 0.0f);
    fm::vec3 localB(0.0f,  halfH, 0.0f);

    // transforms the local ends to world ends
    outA = (localA + col.offset).transformed(world);
    outB = (localB + col.offset).transformed(world);
}

void Collider::closestPointBetweenSegments(fm::vec3 p1, fm::vec3 q1, fm::vec3 p2, fm::vec3 q2, fm::vec3& c1, fm::vec3& c2)
{
    fm::vec3 d1 = q1 - p1;
    fm::vec3 d2 = q2 - p2;
    fm::vec3 r = p1 - p2;
    float a = d1.dot(d1);
    float e = d2.dot(d2);
    float f = d2.dot(r);

    float s, t;
    if (a <= 1e-6f && e <= 1e-6f) {
        c1 = p1; c2 = p2; return;
    }
    if (a <= 1e-6f) {
        s = 0.0f;
        t = std::clamp(f / e, 0.0f, 1.0f);
    } else {
        float c = d1.dot(r);
        if (e <= 1e-6f) {
            t = 0.0f;
            s = std::clamp(-c / a, 0.0f, 1.0f);
        } else {
            float b = d1.dot(d2);
            float denom = a * e - b * b;
            if (denom != 0.0f) {
                s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
            } else {
                s = 0.0f;
            }
            t = (b * s + f) / e;
            if (t < 0.0f) {
                t = 0.0f;
                s = std::clamp(-c / a, 0.0f, 1.0f);
            } else if (t > 1.0f) {
                t = 1.0f;
                s = std::clamp((b - c) / a, 0.0f, 1.0f);
            }
        }
    }
    c1 = p1 + d1 * s;
    c2 = p2 + d2 * t;
}

// checks if a collider is colliding with another collider and outputs a contact point
bool Collider::IsCollidingWith(Collider& collider, Contact& out)
{

    switch (this->m_shape) {
        case (ColliderShape::Box) : {

            switch (collider.m_shape) {

                case (ColliderShape::Sphere) : {
                    return CheckSphereBoxIntersection(*this, collider, out);
                }
                case (ColliderShape::Box) : {
                    return CheckBoxIntersection(*this, collider, out);
                }
                case ColliderShape::Capsule : {
                    return CheckBoxCapsuleIntersection(*this, collider, out);
                }
                case ColliderShape::Cylinder : {
                    return CheckBoxCylinderIntersection(*this, collider, out);
                }
            }
            break;
        }
        case (ColliderShape::Sphere) : {

            switch (collider.m_shape) {

                case (ColliderShape::Sphere) : {
                    return CheckSphereBoxIntersection(collider, *this, out);
                }
                case (ColliderShape::Box) : {
                    return CheckSphereBoxIntersection(*this, collider, out);
                }
                case ColliderShape::Capsule : {
                    return CheckSphereCapsuleIntersection(collider, *this, out);
                }
                case ColliderShape::Cylinder : {
                    return CheckSphereCylinderIntersection(*this, collider, out);
                }

            }
            break;
        }
        case (ColliderShape::Capsule) : {

            switch (collider.m_shape) {

                case (ColliderShape::Sphere) : {
                    return CheckSphereCapsuleIntersection(*this, collider, out);
                }
                case (ColliderShape::Box) : {
                    return CheckBoxCapsuleIntersection(collider, *this, out);
                }
                case ColliderShape::Capsule : {
                    return CheckCapsuleIntersection(*this, collider, out);
                }
                case ColliderShape::Cylinder : {
                    return CheckCylinderCapsuleIntersection(collider, *this, out);
                }

            }
            break;
        }
        case (ColliderShape::Cylinder) : {

            switch (collider.m_shape) {

                case (ColliderShape::Sphere) : {
                    return CheckSphereCylinderIntersection(*this, collider, out);
                }
                case (ColliderShape::Box) : {
                    return CheckBoxCylinderIntersection(collider, *this, out);
                }
                case ColliderShape::Capsule : {
                    return CheckCylinderIntersection(*this, collider, out);
                }
                case ColliderShape::Cylinder : {
                    return CheckCylinderCapsuleIntersection(*this, collider, out);
                }

            }
            break;
        }
    }
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

bool Collider::CheckSphereBoxIntersection(Collider& a, Collider& b, Contact& out) {
    return CheckSphereBoxIntersection(a.getWorldCenter(), a.getWorldRadius(), b, out);
}

bool Collider::CheckSphereCapsuleIntersection(Collider& capsule, Collider& sphere, Contact& out)
{
    fm::vec3 pointA, pointB;
    getSegmentEnds(capsule, pointA, pointB);

    fm::vec3 sphereCenter = sphere.getWorldCenter();
    fm::vec3 closest = fm::vec3::closestPoint(pointA, pointB, sphereCenter);

    float rc = capsule.getWorldRadius();
    float rs = sphere.getWorldRadius();
    float sumRad = rc + rs;

    fm::vec3 d = sphereCenter - closest;
    float distSq = d.dot(d);

    if (distSq > sumRad * sumRad) return false;

    float dist = std::sqrt(distSq);
    out.penetration = sumRad - dist;
    out.normal = dist > 1e-6f ? d * (1.0f / dist) : fm::vec3(0.f, 1.f, 0.f);
    return true;
}

bool Collider::CheckBoxCapsuleIntersection(Collider& box, Collider& capsule, Contact& out) {
    fm::vec3 a, b;
    getSegmentEnds(capsule, a, b);

    // Find closest point on capsule segment to box center (or use box bounds)
    AABB bBox = box.getWorldAABB();

    fm::vec3 segMid = (a + b) + fm::vec3(0.5f);
    fm::vec3 boxClosestToMid(std::clamp(segMid.x, bBox.m_min.x, bBox.m_max.x),
                             std::clamp(segMid.y, bBox.m_min.y, bBox.m_max.y),
                             std::clamp(segMid.z, bBox.m_min.z, bBox.m_max.z));

    fm::vec3 closestOnSeg = fm::vec3::closestPoint(a, b, boxClosestToMid);

    // Temporarily treat the closest segment point as a sphere center
    return CheckSphereBoxIntersection(closestOnSeg, capsule.getWorldRadius(), box, out);
}

bool Collider::CheckCapsuleIntersection(Collider& capA, Collider& capB, Contact& out) {
    fm::vec3 a1, b1, a2, b2;
    getSegmentEnds(capA, a1, b1);
    getSegmentEnds(capB, a2, b2);

    fm::vec3 c1, c2;
    closestPointBetweenSegments(a1, b1, a2, b2, c1, c2);

    fm::vec3 d = c2 - c1;
    float distSq = d.dot(d);
    float rSum = capA.getWorldRadius() + capB.getWorldRadius();

    if (distSq > rSum * rSum) return false;

    float dist = std::sqrt(std::max(0.0f, distSq));
    out.penetration = rSum - dist;
    out.normal = dist > 1e-6f ? d * (1.0f / dist) : fm::vec3(0.f, 1.f, 0.f);
    return true;
}

bool Collider::CheckCylinderCapsuleIntersection(Collider& cyl, Collider& cap, Contact& out) {
    // Treat cylinder core as a segment with radial bounds + flat cap checks
    fm::vec3 a, b;
    getSegmentEnds(cyl, a, b);
    fm::vec3 ca, cb;
    getSegmentEnds(cap, ca, cb);

    fm::vec3 c1, c2;
    closestPointBetweenSegments(a, b, ca, cb, c1, c2);

    fm::vec3 d = c2 - c1;
    float distSq = d.dot(d);
    float rSum = cyl.getWorldRadius() + cap.getWorldRadius();

    if (distSq > rSum * rSum) return false;

    float dist = std::sqrt(std::max(0.0f, distSq));
    out.penetration = rSum - dist;
    out.normal = dist > 1e-6f ? d * (-1.0f / dist) : fm::vec3(0.f, 1.f, 0.f);
    return true;
}

bool Collider::CheckBoxCylinderIntersection(Collider& box, Collider& cyl, Contact& out) {
    fm::vec3 a, b;
    getSegmentEnds(cyl, a, b);
    float r = cyl.getWorldRadius();

    // 1. Get exact world-space height from segment endpoints (accounts for transform scale!)
    fm::vec3 ab = b - a;
    float worldHeightSq = ab.dot(ab);
    if (worldHeightSq < 1e-6f) return false;
    
    float worldHeight = std::sqrt(worldHeightSq);
    fm::vec3 axisDir = ab * (1.0f / worldHeight);

    AABB bBox = box.getWorldAABB();
    fm::vec3 boxCenter = (bBox.m_min + bBox.m_max) * 0.5f;

    // 2. Find the closest point on the cylinder's central axis segment to the box center
    fm::vec3 closestOnAxis = fm::vec3::closestPoint(a, b, boxCenter);

    // 3. Find the closest point on the box to that axis point
    fm::vec3 boxClosest(std::clamp(closestOnAxis.x, bBox.m_min.x, bBox.m_max.x),
                        std::clamp(closestOnAxis.y, bBox.m_min.y, bBox.m_max.y),
                        std::clamp(closestOnAxis.z, bBox.m_min.z, bBox.m_max.z));

    // Vector from bottom endpoint 'a' to boxClosest
    fm::vec3 ac = boxClosest - a;
    float projection = ac.dot(axisDir);

    // 4. Test collision based on whether it hits the side wall or the flat end caps
    if (projection >= 0.0f && projection <= worldHeight) {
        // --- SIDE WALL COLLISION ---
        fm::vec3 axisPoint = a + axisDir * projection;
        fm::vec3 d = boxClosest - axisPoint;
        float distSq = d.dot(d);

        if (distSq > r * r) return false;

        float dist = std::sqrt(std::max(0.0f, distSq));
        out.penetration = r - dist;
        out.normal = dist > 1e-6f ? d * (-1.0f / dist) : fm::vec3(0.f, 1.f, 0.f);
        return true;
    } 
    else {
        // --- FLAT END CAP COLLISION ---
        fm::vec3 capCenter = (projection < 0.0f) ? a : b;
        fm::vec3 d = boxClosest - capCenter;
        float distSq = d.dot(d);

        if (distSq > r * r) return false;

        float axialDist = (projection < 0.0f) ? -projection : (projection - worldHeight);
        out.penetration = r - std::abs(axialDist);
        out.normal = (projection < 0.0f) ? axisDir * -1.0f : axisDir;

        if (out.normal.dot(out.normal) < 0.5f) {
            out.normal = fm::vec3(0.f, 1.f, 0.f); // fallback
        }

        if (out.penetration <= 0.0f) return false;
        return true;
    }
}

bool Collider::CheckSphereCylinderIntersection(Collider& sphere, Collider& cyl, Contact& out) {
    fm::vec3 a, b;
    getSegmentEnds(cyl, a, b);
    fm::vec3 sCenter = sphere.getWorldCenter();

    fm::vec3 closestOnAxis = fm::vec3::closestPoint(a, b, sCenter);
    fm::vec3 d = sCenter - closestOnAxis;
    float distSq = d.dot(d);
    float rSum = sphere.getWorldRadius() + cyl.getWorldRadius();

    if (distSq > rSum * rSum) return false;

    float dist = std::sqrt(std::max(0.0f, distSq));
    out.penetration = rSum - dist;
    out.normal = dist > 1e-6f ? d * (-1.0f / dist) : fm::vec3(0.f, 1.f, 0.f);
    return true;
}

bool Collider::CheckCylinderIntersection(Collider& cylA, Collider& cylB, Contact& out) {
    fm::vec3 a1, b1, a2, b2;
    getSegmentEnds(cylA, a1, b1);
    getSegmentEnds(cylB, a2, b2);

    fm::vec3 c1, c2;
    closestPointBetweenSegments(a1, b1, a2, b2, c1, c2);

    fm::vec3 d = c2 - c1;
    float distSq = d.dot(d);
    float rSum = cylA.getWorldRadius() + cylB.getWorldRadius();

    if (distSq > rSum * rSum) return false;

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
    float correctionMag = std::max(0.0f, c.penetration - slop) / total * percent;
    
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
    float restThreshold = 9.8f * (1.0f / PhysicsImpl::tps) * 2.0f;
    if (std::abs(vn) < restThreshold) e = 0.f;               // kill tiny bounces so resting objects settle

    // adds a velocity change to the rigidbody
    float j = -(1.f + e) * vn / total;
    if (ra) ra->addForce(-c.normal * (j * ia), Rigidbody::ForceMode::VelocityChange);
    if (rb) rb->addForce( c.normal * (j * ib), Rigidbody::ForceMode::VelocityChange);
}

void Collider::draw() {

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
            _drawCylinder(radius, height, entity.getRealMatrix(), line);
            break;
        }
        case (ColliderShape::Capsule) : {
            _drawCapsule(radius, height, entity.getRealMatrix(), line);
            break;
        }
    }
}


void Collider::_drawBox(fm::vec3 min, fm::vec3 max, fm::mat4 model, Debug::DebugLine& line) {

    std::vector<Point> points;

    points.push_back({min, m_drawCol});
    points.push_back({fm::vec3(max.x, min.y, min.z), m_drawCol});
    points.push_back({fm::vec3(max.x, max.y, min.z), m_drawCol});
    points.push_back({fm::vec3(min.x, max.y, min.z), m_drawCol});

    points.push_back({fm::vec3(min.x, min.y, max.z), m_drawCol});
    points.push_back({fm::vec3(max.x, min.y, max.z), m_drawCol});
    points.push_back({max, m_drawCol});
    points.push_back({fm::vec3(min.x, max.y, max.z), m_drawCol});


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
    m_material->m_data.m_albedoFac = col;
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
    static std::shared_ptr<Material> boundingBoxMaterial = Material::create();
    boundingBoxMaterial->m_data.m_albedoFac = {1.0f, 0.9f, 0.8f, 1.0f};
    m_material = boundingBoxMaterial;

    // gets the rigidbody from the entity and sets the static variable based on that then creates the world collider
    m_rb = getEntity().getComponent<Physics::Rigidbody>();
    m_isStatic = (m_rb == nullptr);
    CollisionWorld::get().add(this);
}

// DESTROYS EVERYTHING MWHAHAHA and destroys the collider
void Collider::onDestroy() {
    // release our id 
    s_colliderIDPool.releaseID(m_ColliderID);

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
    float r = cyl->radius;
    float halfH = cyl->height * 0.5f;

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
    float r = cap->radius;
    float totalH = cap->height;
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