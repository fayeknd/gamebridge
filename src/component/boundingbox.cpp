#include "gl_context.hpp"
#include <component/boundingbox.hpp>


using namespace Engine;

// set the box bounds
void BoundingBox::setBounds(fm::vec3 min, fm::vec3 max) {
    m_min = min;
    m_max = max;
}