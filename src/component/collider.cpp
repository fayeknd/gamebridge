#include <component/collider.hpp>
#include <physics/physics.hpp>
#include <entity.hpp>
#include <debug.hpp>

using namespace Engine;
using namespace Physics;
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
    this->m_center = center;
    this->m_radius = radius;
}

Collider::Collider(ColliderShape shape)
{
    this->m_shape = shape;
}

fm::vec3 Collider::getWorldCenter() {
    return (m_center + m_offset).transformed(getEntity().transform.getGlobalMatrix());
}

float Collider::getWorldRadius() {
    fm::vec3 s = getEntity().transform.getGlobalScale();
    return m_radius * std::max({ std::abs(s.x), std::abs(s.y), std::abs(s.z) });
}

bool Collider::IsCollidingWith(Collider& collider, Contact& out)
{

    switch (this->m_shape) {
        case (ColliderShape::Box) : {

            switch (collider.m_shape) {

                case (ColliderShape::Sphere) : {
                    return CheckSphereBoxIntersection(collider, *this, out);
                }
                case (ColliderShape::Box) : {
                    return CheckBoxIntersection(*this, collider, out);
                }

            }
            break;
        }
        case (ColliderShape::Sphere) : {

            switch (collider.m_shape) {

                case (ColliderShape::Sphere) : {
                    return CheckSphereBoxIntersection(*this, collider, out);
                }
                case (ColliderShape::Box) : {
                    return CheckSphereBoxIntersection(*this, collider, out);
                }

            }
            break;
        }
    }
    return false;
}

bool Collider::IsCollidingWith(Collider& collider)
{
    Contact discard;
    return IsCollidingWith(collider, discard);
}

bool Collider::CheckBoxIntersection(Collider& a, Collider& b, Contact& out) {
    AABB A = a.getWorldAABB();
    AABB B = b.getWorldAABB();

    float ox = std::min(A.m_max.x, B.m_max.x) - std::max(A.m_min.x, B.m_min.x);
    float oy = std::min(A.m_max.y, B.m_max.y) - std::max(A.m_min.y, B.m_min.y);
    float oz = std::min(A.m_max.z, B.m_max.z) - std::max(A.m_min.z, B.m_min.z);
    if (ox <= 0 || oy <= 0 || oz <= 0) return false;

    fm::vec3 ca = (A.m_min + A.m_max) * 0.5f;
    fm::vec3 cb = (B.m_min + B.m_max) * 0.5f;

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
    return true;
}

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

bool Collider::CheckSphereBoxIntersection(Collider& a, Collider& b, Contact& out)
{
    fm::vec3 c = a.getWorldCenter();
    float r    = a.getWorldRadius();
    AABB box   = b.getWorldAABB();

    fm::vec3 closest(std::clamp(c.x, box.m_min.x, box.m_max.x),
                     std::clamp(c.y, box.m_min.y, box.m_max.y),
                     std::clamp(c.z, box.m_min.z, box.m_max.z));

    fm::vec3 closestPoint;

    fm::vec3 d = c - closest;           
    float distSq = d.dot(d);
    if (distSq > r * r) return false;

    if (distSq > 1e-12f) {
        float dist = std::sqrt(distSq);
        out.normal = d * (-1.f / dist);
        out.penetration  = r - dist;
        return true;
    }

    
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

void CollisionWorld::resolve(Collider* a, Collider* b, const Collider::Contact& c) {
    Physics::Rigidbody* ra = a->m_rb;
    Physics::Rigidbody* rb = b->m_rb;

    float ia = ra ? ra->getInverseMass() : 0.f;
    float ib = rb ? rb->getInverseMass() : 0.f;
    float total = ia + ib;
    if (total == 0.f) return;                       

    // 1. push apart objects
    if (ra) a->getEntity().transform.addPosition(c.normal * (-c.penetration * ia / total));
    if (rb) b->getEntity().transform.addPosition(c.normal * ( c.penetration * ib / total));

    // 2. bouncyyyyy i love trampolining
    fm::vec3 va = ra ? ra->getVelocity() : fm::vec3::zero();
    fm::vec3 vb = rb ? rb->getVelocity() : fm::vec3::zero();
    float vn = (vb - va).dot(c.normal);             
    if (vn > 0.f) return;                           

    float e = std::min(ra ? ra->m_bounciness : 1.f, rb ? rb->m_bounciness : 1.f);
    float restThreshold = 9.8f * (1.0f / PhysicsImpl::tps) * 2.0f;
    if (std::abs(vn) < restThreshold) e = 0.f;               // kill tiny bounces so resting objects settle

    float j = -(1.f + e) * vn / total;              // impulse magnitude
    if (ra) ra->addForce(va - c.normal * (j * ia), Rigidbody::ForceMode::VelocityChange);
    if (rb) rb->addForce(vb + c.normal * (j * ib), Rigidbody::ForceMode::VelocityChange);
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

ColliderID Collider::getColliderID() {
    return m_ColliderID;
}

void Collider::setColour(fm::vec4 col) {
    m_material->m_data.m_albedoFac = col;
    //m_program->useProgram();
    //m_program->setUniform("boxCol", col);
}

void Collider::init() {

    // get the next box id
    m_ColliderID = s_colliderIDPool.getNewID();

    // create a static material to satisfy IRenderable.
    // this does not need to exist really. it just holds 
    // colour. 
    static std::shared_ptr<Material> boundingBoxMaterial = Material::create();
    boundingBoxMaterial->m_data.m_albedoFac = {1.0f, 0.9f, 0.8f, 1.0f};
    m_material = boundingBoxMaterial;

    m_rb = getEntity().getComponent<Physics::Rigidbody>();
    m_isStatic = (m_rb == nullptr);
    CollisionWorld::get().add(this);
}

void Collider::onDestroy() {
    // release our id 
    s_colliderIDPool.releaseID(m_ColliderID);
    CollisionWorld::get().remove(this);
}

bool Collider::checkRayIntersection(const Ray& ray, RaycastHit& outRaycastHit) {

    outRaycastHit.m_hitEntity = &getEntity();
    outRaycastHit.m_hitBox = this;

    fm::mat4 worldMatrixInversed = outRaycastHit.m_hitEntity->transform.getGlobalMatrix().inversed();

    // construct our local ray 
    Ray localRay(
        ray.m_origin.transformed(worldMatrixInversed), // local origin
        ray.m_direction.transformed(worldMatrixInversed, true) // local direction
    );

    // get bounding box from entity. 
    Collider& boundingBox = *outRaycastHit.m_hitBox;

    switch(m_shape)
    {
        case ColliderShape::Box:
            return rayAABoxIntersection(localRay, this, outRaycastHit);
        case ColliderShape::Sphere:
            return rayAASphereIntersection(localRay, this, outRaycastHit);
        default: 
            return false;
    }
}

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

void Collider::setStatic(bool s)
{
    if (m_isStatic == s) return;
    CollisionWorld::get().remove(this);
    m_isStatic = s;
    CollisionWorld::get().add(this);
}

AABB Collider::getWorldAABB() {
    Transform& t = entity.transform;
    fm::mat4 world = t.getGlobalMatrix();

    if (m_shape == ColliderShape::Sphere) {
        fm::vec3 c = (m_center + m_offset).transformed(world);

        
        fm::vec3 s = t.getGlobalScale();
        float r = m_radius * std::max({ std::abs(s.x), std::abs(s.y), std::abs(s.z) });

        return { c - fm::vec3(r, r, r), c + fm::vec3(r, r, r) };
    }

   
    fm::vec3 lo( FLT_MAX,  FLT_MAX,  FLT_MAX);
    fm::vec3 hi(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (int i = 0; i < 8; ++i) {
        fm::vec3 corner((i & 1) ? m_max.x : m_min.x,
                        (i & 2) ? m_max.y : m_min.y,
                        (i & 4) ? m_max.z : m_min.z);

        fm::vec3 w = (corner + m_offset).transformed(world);

       
        lo = { std::min(lo.x, w.x), std::min(lo.y, w.y), std::min(lo.z, w.z) };
        hi = { std::max(hi.x, w.x), std::max(hi.y, w.y), std::max(hi.z, w.z) };
    }
    
    return { lo, hi };
}


void CollisionWorld::step() {
    if (m_staticDirty) { m_staticTree.rebuild(m_static); m_staticDirty = false; }
    m_dynamicTree.rebuild(m_dynamic);

    std::vector<Collider*> candidates;
    for (Collider* c : m_dynamic) {
        candidates.clear();
        AABB box = c->getWorldAABB();
        m_staticTree.query(box, candidates);
        m_dynamicTree.query(box, candidates);

        for (Collider* o : candidates) {
            if (o == c) continue;

            if (!o->m_isStatic && o->getColliderID() < c->getColliderID()) continue;

            Collider::Contact contact;
            if (!c->IsCollidingWith(*o, contact)) continue;
            resolve(c, o, contact);
        }
    }
}

bool CollisionWorld::raycast(const Physics::Ray& ray, Physics::RaycastHit& hit, LayerMask mask, float maxDist)
{
    refresh();
    //DBG.logInfo("static: {} dynamic: {}", m_static.size(), m_dynamic.size());
    bool a = m_staticTree.raycast(ray, hit, mask, maxDist);
    float limit = a ? hit.m_hitInfo.m_distance : maxDist;
    Physics::RaycastHit h;
    bool b = m_dynamicTree.raycast(ray, h, mask, limit);
    if (b) hit = h;
    return a || b;
}

void CollisionWorld::refresh() {
    if (m_staticDirty) { m_staticTree.rebuild(m_static); m_staticDirty = false; }
    m_dynamicTree.rebuild(m_dynamic);
}

void CollisionWorld::add(Collider* c)
{
    if (!c) return;

    auto& list = c->m_isStatic ? m_static : m_dynamic;

    if (std::find(list.begin(), list.end(), c) != list.end()) return;

    list.push_back(c);
    if (c->m_isStatic) m_staticDirty = true;
}

void CollisionWorld::remove(Collider* c)
{
    if (!c) return;

    auto s = std::find(m_static.begin(), m_static.end(), c);

    if (s != m_static.end())
    {
        m_static.erase(s);
        m_staticDirty = true;
    }

    auto d = std::find(m_dynamic.begin(), m_static.end(), c);
    if (d != m_dynamic.end())
    {
        m_dynamic.erase(d);
    }
}