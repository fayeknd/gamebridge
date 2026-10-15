#include <transform.hpp>

using namespace Engine;

fm::mat4 Transform::getMatrix() {
    if (!m_dirty) return m_modelMat;

    m_modelMat = fm::model(m_position, m_scale, m_rotation, m_skew);

    m_dirty = false;
    return m_modelMat;

}

#pragma region getters

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

#pragma endregion


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

#pragma endregion