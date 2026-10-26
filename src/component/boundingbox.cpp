#include "gl_context.hpp"
#include "mesh.hpp"
#include "physics/ray.hpp"
#include "shader.hpp"
#include <component/boundingbox.hpp>
#include <component/camera.hpp>
#include <entity.hpp>

using namespace Engine;
using namespace Physics;
using namespace Debug;

BoundingBoxID BoundingBox::getBoxID() {
    return m_boxID;
}

void BoundingBox::setColour(fm::vec4 col) {
    m_material->m_data.m_albedoFac = col;
    //m_program->useProgram();
    //m_program->setUniform("boxCol", col);
}

void BoundingBox::init() {

    // get the next box id
    m_boxID = s_boundingBoxIDPool.getNewID();

    // register ourselves with the bounding box list
    s_boundingBoxList.emplace(m_boxID, this);

    // create a static material to satisfy IRenderable.
    // this does not need to exist really. it just holds 
    // colour. 
    static std::shared_ptr<Material> boundingBoxMaterial = Material::create();
    boundingBoxMaterial->m_data.m_albedoFac = {1.0f, 0.9f, 0.8f, 1.0f};
    m_material = boundingBoxMaterial;
}

void BoundingBox::onDestroy() {

    // remove ourselves from the bounding box list
    s_boundingBoxList.erase(m_boxID);

    // release our id 
    s_boundingBoxIDPool.releaseID(m_boxID);
}

void BoundingBox::draw() {
    if (!DebugUI::get().m_drawDebugTools) return;
    this->collider->draw(entity.transform.getGlobalPosition(), entity.transform.getGlobalScale());
}

bool BoundingBox::checkRayIntersection(const Ray& ray, RaycastHit& outRaycastHit) {

    fm::mat4 worldMatrixInversed = outRaycastHit.m_hitEntity->transform.getGlobalMatrix().inversed();

    // construct our local ray 
    Ray localRay(
        ray.m_origin.transformed(worldMatrixInversed), // local origin
        ray.m_direction.transformed(worldMatrixInversed, true) // local direction
    );

    // get bounding box from entity. 
    BoundingBox& boundingBox = *outRaycastHit.m_hitBox;

    switch(boundingBox.collider->m_shape)
    {
        case ColliderShape::Box:
            return rayAABoxIntersection(localRay, boundingBox.collider.get(), outRaycastHit);
        case ColliderShape::Sphere:
            return rayAASphereIntersection(localRay, boundingBox.collider.get(), outRaycastHit);
        default: 
            return false;
    }
}

bool BoundingBox::rayAABoxIntersection(const Ray& ray, const Collider* box, RaycastHit& outRaycastHit) {

    if (outRaycastHit.m_hitEntity->getLayer()->m_ignoreRaycast) return false;

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

bool BoundingBox::rayAASphereIntersection(const Ray& ray, const Collider* sphere, RaycastHit& outRaycastHit) {

    if (outRaycastHit.m_hitEntity->getLayer()->m_ignoreRaycast) return false;

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

// set the box bounds
void BoundingBox::setBounds(fm::vec3 min, fm::vec3 max) {
    m_min = min;
    m_max = max;
}

std::unordered_map<BoundingBoxID, BoundingBox*> BoundingBox::getBoundingBoxList() {
    return s_boundingBoxList; // readonly
}
