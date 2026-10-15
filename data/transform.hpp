#pragma once
#include <fm/fmath.hpp>

namespace Engine {
    
    class Transform {
    protected:

        bool m_dirty = true;

        fm::vec3 m_position = {0};
        fm::vec3 m_rotation = {0};
        fm::vec3 m_scale    = {1};
        fm::vec3 m_skew     = {0};

        fm::mat4 m_modelMat;


        template <typename T>
        void _setValue(T& value, T& newValue) {
            if (value == newValue) return;

            value = newValue;
            m_dirty = true; 
        }

    public:

        fm::mat4 getMatrix();

        fm::vec3 getPosition();
        fm::vec3 getRotation();
        fm::vec3 getScale();
        fm::vec3 getSkew();

        float getPositionX();
        float getPositionY();
        float getPositionZ(); 

        float getRotationX();
        float getRotationY();
        float getRotationZ();
        
        float getScaleX();
        float getScaleY();
        float getScaleZ();
        
        float getSkewX();
        float getSkewY();
        float getSkewZ(); 

        void setPosition(fm::vec3 v);
        void setRotation(fm::vec3 v);
        void setScale(fm::vec3 v);
        void setSkew(fm::vec3 v);

        void setPositionX(float v);
        void setPositionY(float v);
        void setPositionZ(float v);

        void setRotationX(float v);
        void setRotationY(float v);
        void setRotationZ(float v);

        void setScaleX(float v);
        void setScaleY(float v);
        void setScaleZ(float v);

        void setSkewX(float v);
        void setSkewY(float v);
        void setSkewZ(float v); 




    };
}