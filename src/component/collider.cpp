#include "gl_context.hpp"
#include "mesh.hpp"
#include "ray.hpp"
#include "shader.hpp"
#include <component/collider.hpp>
#include <component/camera.hpp>
#include <debug.hpp>

using namespace Engine;

BoundingBoxID BoundingBox::getBoxID() {
    return m_boxID;
}

void BoundingBox::setColour(fm::vec4 col) {
    m_program->useProgram();
    m_program->setUniform(colliderUniform_boxCol, col);
}

void BoundingBox::init() {

    // get the next box id
    m_boxID = s_boundingBoxIDPool.getNewID();

    // register ourselves with the bounding box list
    s_boundingBoxList.emplace(m_boxID, this);

    // create some debug drawing stuff
    m_boxMesh = Mesh::primitive(Primitive::Cube);
    m_program = ShaderProgram::createProgramFromPath("stuff=>shaders/collider.glsl");
    // set the box draw colour
    setColour(m_boxDebugCol);
}

void BoundingBox::onDestroy() {

    // remove ourselves from the bounding box list
    s_boundingBoxList.erase(m_boxID);

    // release our id 
    s_boundingBoxIDPool.releaseID(m_boxID);
}

void BoundingBox::draw() {
    if (!m_boxMesh || !m_program || !entity.isEnabled() || !m_renderBounds || !Debug::DebugUI::get().m_drawHitboxes) return;
    m_renderData.useRenderData();

    // bind the mesh
    m_boxMesh->bindMesh();

    // use the program 
    m_program->useProgram();

    fm::vec3 boxCenter = m_min.lerp(m_max, 0.5f);

    // set basic uniforms
    m_program->setUniform(shader_ModelMatrix, entity.transform.getGlobalMatrix() * fm::model(
        boxCenter,
        (m_max.abs() + m_min.abs()),
        {0}, {0}
    ));
    // ViewProjection matrix
    fm::mat4 vpMat = fm::mat4::identity();
    // if main camera exists
    if (Camera::activeCamera()) {
        // vpMat is the main camera projection * view
        vpMat = Camera::activeCamera()->getProjectionMat() * Camera::activeCamera()->getViewMat();
    }
    m_program->setUniform(shader_ViewProjectionMatrix, vpMat);

    // draw the mesh
    glDrawElements(m_boxMesh->getTopology(), m_boxMesh->getIndexCount(), GL_UNSIGNED_SHORT, 0);
    glLineWidth(5);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(m_boxMesh->getTopology(), m_boxMesh->getIndexCount(), GL_UNSIGNED_SHORT, 0);
    m_renderData.useRenderData();
}

bool BoundingBox::checkBoxIntersection(BoundingBox& a, BoundingBox& b) {
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

bool BoundingBox::checkRayIntersection(const Ray& ray, RaycastHit& outRaycastHit) {

    fm::mat4 worldMatrixInversed = outRaycastHit.m_hitEntity->transform.getGlobalMatrix().inversed();

    // construct our local ray 
    Ray localRay(
        ray.m_origin.transformed(worldMatrixInversed), // local origin
        ray.m_direction.transformed(worldMatrixInversed, true) // local direction
    );

    // get bounding box from entity. 
    BoundingBox& boundingBox = *outRaycastHit.m_hitBox;

    // return a collision. we don't need to return hasHit,
    // because if hasHit is true, this function will have set it.
    return rayAABoxIntersection(localRay, boundingBox, outRaycastHit);
}

bool BoundingBox::rayAABoxIntersection(const Ray& ray, const BoundingBox& box, RaycastHit& outRaycastHit) {

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
        
        float minVal = box.m_min[i];
        float maxVal = box.m_max[i];

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

// set the box bounds
void BoundingBox::setBounds(fm::vec3 min, fm::vec3 max) {
    m_min = min;
    m_max = max;
}

std::unordered_map<BoundingBoxID, BoundingBox*> BoundingBox::getBoundingBoxList() {
    return s_boundingBoxList; // readonly
}