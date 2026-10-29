#include <fm/types/mat4x4.hpp>
#include <transform.hpp>
#include <entity.hpp>
#include <component/camera.hpp>
#include <renderer.hpp>

using namespace Engine;

// get the matrix
// see src/component/renderable.cpp to see where this might be called
fm::mat4 Transform::getMatrix(bool forceRecompute) {
    // if the transform isn't dirty, return what's already been computed
    if (!m_dirty && !forceRecompute) return m_modelMat;

    // calculate the model matrix.
    // with our SUPER EASY fmath function that SOMEONE wrote :)
    m_modelMat = fm::model(m_position, m_scale, m_rotation, m_skew);

    // make just a rotation matrix
    fm::mat4 rot = fm::mat4().rotate(m_rotation);
    m_front   = fm::vec3::zAxis().transformed(rot); // get the local z direction
    m_right   = fm::vec3::xAxis().transformed(rot); // get the local x direction
    m_up      = fm::vec3::yAxis().transformed(rot); // get the local y direction

    // updated, set m_dirty to false.
    m_dirty = false;
    // we always want this to be true every frame.
    // the issue with depth calculations is they use
    // the camera positions, and we don't know
    // if or when those change, but we also
    // don't want to recalculate it EVERY time it's 
    // called, just once per frame maximum.
    // so we set this to true, and then if depth is
    // calculated, we don't need to calculate it again.
    m_depthDirty = true;
    // return the new model matrix
    return m_modelMat;

}

fm::mat4 Transform::getGlobalMatrix() {
    if (!m_globalDirty) return m_globalModelMat;
    m_globalDirty = false;

    fm::mat4 local = getMatrix(true);
    if (!m_entity) return local;

    auto p = m_entity->getParent();
    if (!p) return local;

    m_globalModelMat = p->transform.getGlobalMatrix() * local;
    fm::_mat4x4decomp<float> decomp = m_globalModelMat.decompose();
    m_globalPosition = decomp.position;
    m_globalRotation = decomp.rotation;
    m_globalScale    = decomp.scale;
    m_globalSkew     = decomp.skew;

    m_globalRight = -fm::vec3(m_globalModelMat.m_data[0][0], m_globalModelMat.m_data[0][1], m_globalModelMat.m_data[0][2]).normalized();
    m_globalUp    = -fm::vec3(m_globalModelMat.m_data[1][0], m_globalModelMat.m_data[1][1], m_globalModelMat.m_data[1][2]).normalized();
    m_globalFront =  fm::vec3(m_globalModelMat.m_data[2][0], m_globalModelMat.m_data[2][1], m_globalModelMat.m_data[2][2]).normalized();

    //if (fm::flipX) m_globalRight = -m_globalRight;
    //if (fm::flipY) m_globalUp    = -m_globalUp;
    //if (fm::flipZ) m_globalFront = -m_globalFront;

    return m_globalModelMat;
}

// get the object's "rough" depth from the camera
float Transform::getDepth(fm::vec3 camPos, fm::vec3 camFront, DepthType type) {
    
    // early return
    if (!m_depthDirty) return m_depthFromCamera;
    m_depthDirty = false;

    fm::vec3 pos = getGlobalPosition();
    fm::vec3 relative = pos - camPos;
    
    // get the relative distance from this to the camera
    switch (type) {
        case (DepthType::Planar) :       m_depthFromCamera = relative.dot(camFront); break; // opaque depth
        case (DepthType::SquaredRadial): m_depthFromCamera = relative.dot(relative); break; // transparent depth
    }
    return m_depthFromCamera; 
}

// check if the entity has a parent
bool Transform::_hasParent() {
    if (!m_entity) return false;

    auto p = m_entity->getParent();
    if (!p) return false;
    return true;
}

void Transform::_setGlobalVector(fm::vec3& globalv, fm::vec3& localv, fm::vec3 v) {
    if (!_hasParent()) return;
    if (globalv == v) return;

    // set the local value to the new value transformed 
    // by the parent inverse 
    _setValue(localv, v.transform(_getParentInv()));

    // mark transform dirty 
    m_globalDirty = true; 
    _entity_invalidateChildTransforms();
}

fm::mat4 Transform::_getParentInv() {
    return m_entity->getParent()->transform.getGlobalMatrix().inversed();
}

// call back to the entity and invalidate it's children
void Transform::_entity_invalidateChildTransforms() {
    if (!m_entity) return; // no entity assigned, return
    m_entity->m_transformDirty = true; // also let the entity know the transform was dirty
    m_entity->_invalidateChildTransforms();
}

void Transform::_setGlobalModeTrue() {
    if (m_globalMode) return;
    m_globalMode = true;
    m_globalDirty = true;
    getGlobalMatrix();
}

// a bunch of getters.
#pragma region getters

bool Transform::isDirty() {
    return m_dirty;
}
bool Transform::isGlobalDirty() {
    return m_globalDirty;
}

fm::vec3 Transform::getPosition() {
    return m_position;
}
fm::vec3 Transform::getRotation() {
    return m_rotation;
}
fm::vec3 Transform::getScale() {
    return m_scale;
}
fm::vec3 Transform::getSkew() {
    return m_skew;
}

fm::vec3 Transform::getUp() {
    return -m_up;
}
fm::vec3 Transform::getRight() {
    return m_right;
}
fm::vec3 Transform::getFront() {
    return m_front;
}
fm::vec3 Transform::getDown() {
    return m_up;
}
fm::vec3 Transform::getLeft() {
    return -m_right;
}
fm::vec3 Transform::getBack() {
    return -m_front;
}



float Transform::getPositionX() {
    return m_position.x;
}
float Transform::getPositionY() {
    return m_position.y;
}
float Transform::getPositionZ() {
    return m_position.z;
}


float Transform::getRotationX() {
    return m_rotation.x;
}
float Transform::getRotationY() {
    return m_rotation.y;
}
float Transform::getRotationZ() {
    return m_rotation.z;
}


float Transform::getScaleX() {
    return m_scale.x;
}
float Transform::getScaleY() {
    return m_scale.y;
}
float Transform::getScaleZ() {
    return m_scale.z;
}


float Transform::getSkewX() {
    return m_skew.x;
}
float Transform::getSkewY() {
    return m_skew.y;
}
float Transform::getSkewZ() {
    return m_skew.z;
}

fm::vec3 Transform::getGlobalPosition() {
    _setGlobalModeTrue();
    return m_globalPosition;
}
fm::vec3 Transform::getGlobalRotation() {
    _setGlobalModeTrue();
    return m_globalRotation;
}
fm::vec3 Transform::getGlobalScale() {
    _setGlobalModeTrue();
    return m_globalScale;
}
fm::vec3 Transform::getGlobalSkew() {
    _setGlobalModeTrue();
    return m_globalSkew;
}

fm::vec3 Transform::getGlobalUp() {
    _setGlobalModeTrue();
    return m_globalUp;
}
fm::vec3 Transform::getGlobalRight() {
    _setGlobalModeTrue();
    return m_globalRight;
}
fm::vec3 Transform::getGlobalFront() {
    _setGlobalModeTrue();
    return m_globalFront;
}
fm::vec3 Transform::getGlobalDown() {
    _setGlobalModeTrue();
    return -m_globalUp;
}
fm::vec3 Transform::getGlobalLeft() {
    _setGlobalModeTrue();
    return -m_globalRight;
}
fm::vec3 Transform::getGlobalBack() {
    _setGlobalModeTrue();
    return -m_globalFront;
}


float Transform::getGlobalPositionX() {
    _setGlobalModeTrue();
    return m_globalPosition.x;
}
float Transform::getGlobalPositionY() {
    _setGlobalModeTrue();
    return m_globalPosition.y;
}
float Transform::getGlobalPositionZ() {
    _setGlobalModeTrue();
    return m_globalPosition.z;
}


float Transform::getGlobalRotationX() {
    _setGlobalModeTrue();
    return m_globalRotation.x;
}
float Transform::getGlobalRotationY() {
    _setGlobalModeTrue();
    return m_globalRotation.y;
}
float Transform::getGlobalRotationZ() {
    _setGlobalModeTrue();
    return m_globalRotation.z;
}


float Transform::getGlobalScaleX() {
    _setGlobalModeTrue();
    return m_globalScale.x;
}
float Transform::getGlobalScaleY() {
    _setGlobalModeTrue();
    return m_globalScale.y;
}
float Transform::getGlobalScaleZ() {
    _setGlobalModeTrue();
    return m_globalScale.z;
}


float Transform::getGlobalSkewX() {
    _setGlobalModeTrue();
    return m_globalSkew.x;
}
float Transform::getGlobalSkewY() {
    _setGlobalModeTrue();
    return m_globalSkew.y;
}
float Transform::getGlobalSkewZ() {
    _setGlobalModeTrue();
    return m_globalSkew.z;
}

#pragma endregion

// a bunch of setters using the _setValue(T& a, T& b) function.
#pragma region setters

void Transform::setPosition(fm::vec3 v) {
    _setValue(m_position, v);
}
void Transform::setRotation(fm::vec3 v) {
    _setValue(m_rotation, v);
}
void Transform::setScale(fm::vec3 v) {
    _setValue(m_scale, v);
}
void Transform::setSkew(fm::vec3 v) {
    _setValue(m_skew, v);
}


void Transform::setPositionX(float v) {
    _setValue(m_position.x, v);
}
void Transform::setPositionY(float v) {
    _setValue(m_position.y, v);
}
void Transform::setPositionZ(float v) {
    _setValue(m_position.z, v);
}


void Transform::setRotationX(float v) {
    _setValue(m_rotation.x, v);
}
void Transform::setRotationY(float v) {
    _setValue(m_rotation.y, v);
}
void Transform::setRotationZ(float v) {
    _setValue(m_rotation.z, v);
}


void Transform::setScaleX(float v) {
    _setValue(m_scale.x, v);
}
void Transform::setScaleY(float v) {
    _setValue(m_scale.y, v);
}
void Transform::setScaleZ(float v) {
    _setValue(m_scale.z, v);
}


void Transform::setSkewX(float v) {
    _setValue(m_skew.x, v);
}
void Transform::setSkewY(float v) {
    _setValue(m_skew.y, v);
}
void Transform::setSkewZ(float v) {
    _setValue(m_skew.z, v);
}

void Transform::addPosition(fm::vec3 v) {
    _setValue(m_position, (m_position + v));
}
void Transform::addRotation(fm::vec3 v) {
    _setValue(m_rotation, (m_rotation + v));
}
void Transform::addScale(fm::vec3 v) {
    _setValue(m_scale, (m_scale + v));
}
void Transform::addSkew(fm::vec3 v) {
    _setValue(m_skew, (m_skew + v));
}

void Transform::addPositionX(float v) {
    _setValue(m_position.x, (m_position.x + v));
}
void Transform::addPositionY(float v) {
    _setValue(m_position.y, (m_position.y + v));
}
void Transform::addPositionZ(float v) {
    _setValue(m_position.z, (m_position.z + v));
}

void Transform::addRotationX(float v) {
    _setValue(m_rotation.x, (m_rotation.x + v));
}
void Transform::addRotationY(float v) {
    _setValue(m_rotation.y, (m_rotation.y + v));
}
void Transform::addRotationZ(float v) {
    _setValue(m_rotation.z, (m_rotation.z + v));
}

void Transform::addScaleX(float v) {
    _setValue(m_scale.x, (m_scale.x + v));
}
void Transform::addScaleY(float v) {
    _setValue(m_scale.y, (m_scale.y + v));
}
void Transform::addScaleZ(float v) {
    _setValue(m_scale.z, (m_scale.z + v));
}

void Transform::addSkewX(float v) {
    _setValue(m_skew.x, (m_skew.x + v));
}
void Transform::addSkewY(float v) {
    _setValue(m_skew.y, (m_skew.y + v));
}
void Transform::addSkewZ(float v) {
    _setValue(m_skew.z, (m_skew.z + v));
}


void Transform::setGlobalPosition(fm::vec3 v) {
    _setGlobalVector(m_globalPosition, m_position, v);
}
void Transform::setGlobalRotation(fm::vec3 v) {
    _setGlobalVector(m_globalRotation, m_rotation, v);
}
void Transform::setGlobalScale(fm::vec3 v) {
    _setGlobalVector(m_globalScale, m_scale, v);
}
void Transform::setGlobalSkew(fm::vec3 v) {
    _setGlobalVector(m_globalSkew, m_skew, v);
}


void Transform::setGlobalPositionX(float v) {
    _setGlobalVector(m_globalPosition, m_position, {v, m_globalPosition.y, m_globalPosition.z});
}
void Transform::setGlobalPositionY(float v) {
    _setGlobalVector(m_globalPosition, m_position, {m_globalPosition.x, v, m_globalPosition.z});
}
void Transform::setGlobalPositionZ(float v) {
    _setGlobalVector(m_globalPosition, m_position, {m_globalPosition.x, m_globalPosition.y, v});
}


void Transform::setGlobalRotationX(float v) {
    _setGlobalVector(m_globalRotation, m_rotation, {v, m_globalRotation.y, m_globalRotation.z});
}
void Transform::setGlobalRotationY(float v) {
    _setGlobalVector(m_globalRotation, m_rotation, {m_globalRotation.x, v, m_globalRotation.z});
}
void Transform::setGlobalRotationZ(float v) {
    _setGlobalVector(m_globalRotation, m_rotation, {m_globalRotation.x, m_globalRotation.y, v});
}


void Transform::setGlobalScaleX(float v) {
    _setGlobalVector(m_globalScale, m_scale, {v, m_globalScale.y, m_globalScale.z});
}
void Transform::setGlobalScaleY(float v) {
    _setGlobalVector(m_globalScale, m_scale, {m_globalScale.x, v, m_globalScale.z});
}
void Transform::setGlobalScaleZ(float v) {
    _setGlobalVector(m_globalScale, m_scale, {m_globalScale.y, m_globalScale.y, v});
}


void Transform::setGlobalSkewX(float v) {
    _setGlobalVector(m_globalSkew, m_skew, {v, m_globalSkew.y, m_globalSkew.z});
}
void Transform::setGlobalSkewY(float v) {
    _setGlobalVector(m_globalSkew, m_skew, {m_globalSkew.x, v, m_globalSkew.z});
}
void Transform::setGlobalSkewZ(float v) {
    _setGlobalVector(m_globalSkew, m_skew, {m_globalSkew.x, m_globalSkew.y, v});
}

void Transform::addGlobalPosition(fm::vec3 v) {
    _setGlobalVector(m_globalPosition, m_position, m_globalPosition + v);
}
void Transform::addGlobalRotation(fm::vec3 v) {
    _setGlobalVector(m_globalRotation, m_rotation, m_globalRotation + v);
}
void Transform::addGlobalScale(fm::vec3 v) {
    _setGlobalVector(m_globalScale, m_scale, m_globalScale + v);
}
void Transform::addGlobalSkew(fm::vec3 v) {
    _setGlobalVector(m_globalSkew, m_skew, m_globalSkew + v);
}

void Transform::addGlobalPositionX(float v) {
    _setGlobalVector(m_globalPosition, m_position, m_globalPosition + fm::vec3(v, 0, 0));
}
void Transform::addGlobalPositionY(float v) {
    _setGlobalVector(m_globalPosition, m_position, m_globalPosition + fm::vec3(0, v, 0));
}
void Transform::addGlobalPositionZ(float v) {
    _setGlobalVector(m_globalPosition, m_position, m_globalPosition + fm::vec3(0, 0, v));
}

void Transform::addGlobalRotationX(float v) {
    _setGlobalVector(m_globalRotation, m_rotation, m_globalRotation + fm::vec3(v, 0, 0));
}
void Transform::addGlobalRotationY(float v) {
    _setGlobalVector(m_globalRotation, m_rotation, m_globalRotation + fm::vec3(0, v, 0));
}
void Transform::addGlobalRotationZ(float v) {
    _setGlobalVector(m_globalRotation, m_rotation, m_globalRotation + fm::vec3(0, 0, v));
}

void Transform::addGlobalScaleX(float v) {
    _setGlobalVector(m_globalScale, m_scale, m_globalScale + fm::vec3(v, 0, 0));
}
void Transform::addGlobalScaleY(float v) {
    _setGlobalVector(m_globalScale, m_scale, m_globalScale + fm::vec3(0, v, 0));
}
void Transform::addGlobalScaleZ(float v) {
    _setGlobalVector(m_globalScale, m_scale, m_globalScale + fm::vec3(0, 0, v));
}

void Transform::addGlobalSkewX(float v) {
    _setGlobalVector(m_globalSkew, m_skew, m_globalSkew + fm::vec3(v, 0, 0));
}
void Transform::addGlobalSkewY(float v) {
    _setGlobalVector(m_globalSkew, m_skew, m_globalSkew + fm::vec3(0, v, 0));
}
void Transform::addGlobalSkewZ(float v) {
    _setGlobalVector(m_globalSkew, m_skew, m_globalSkew + fm::vec3(0, 0, v));
}

#pragma endregion