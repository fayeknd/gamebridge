#include <component/collider.hpp>
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
    this->center = center;
    this->radius = radius;
}

Collider::Collider(ColliderShape shape)
{
    this->m_shape = shape;
}

bool Collider::IsCollidingWith(Collider& collider)
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
    closestPoint.y = std::max(boxMin.y, std::min(sphereCenter.y, boxMax.y));
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

void Collider::draw() {

    static DebugLine line;
    fm::vec3 globalPos = entity.transform.getGlobalPosition();
    fm::vec3 entityScale = entity.transform.getScale();
    
    fm::mat4 model = entity.getRealMatrix();

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

    // register ourselves with the bounding box list
    s_colliderList.emplace(m_ColliderID, this);

    // create a static material to satisfy IRenderable.
    // this does not need to exist really. it just holds 
    // colour. 
    static std::shared_ptr<Material> boundingBoxMaterial = Material::create();
    boundingBoxMaterial->m_data.m_albedoFac = {1.0f, 0.9f, 0.8f, 1.0f};
    m_material = boundingBoxMaterial;
}

void Collider::onDestroy() {

    // remove ourselves from the bounding box list
    s_colliderList.erase(m_ColliderID);

    // release our id 
    s_colliderIDPool.releaseID(m_ColliderID);
}

bool Collider::checkRayIntersection(const Ray& ray, RaycastHit& outRaycastHit) {

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
    fm::vec3 sphereCenter = sphere->center + sphere->offset;
    fm::vec3 length = ray.m_origin - sphereCenter;

    // discriminants
    float rdr = ray.m_direction.dot(ray.m_direction); // red dead redemption (raydir dot raydir)
    float ldr = 2.0 * length.dot(ray.m_direction); // left dead redemption (length dot raydir)
    float ldl = length.dot(length) - (sphere->radius * sphere->radius); // left dead ledemption (length dot length)

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


std::unordered_map<ColliderID, Collider*> Collider::getColliderList() {
    return s_colliderList; // readonly
}