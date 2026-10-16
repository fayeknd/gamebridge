#pragma once
#include <fm/fmath.hpp>

namespace Engine {
    
    // inspired by my first (pretty terrible) transform system (https://github.com/fayeknd/Application/blob/b93812cc1c5f18045dbb9754be682b88c0b5b095/Application/transform.h)
    // this one is objectively more performant.
    
    // TODO : global / local positioning in a performant way that doesn't deconstruct a matrix like this implementation: https://github.com/fayeknd/peachmilk_editor/blob/main/src/render/transform.cpp#L58

    // comparable to Unity's transform system. 
    // uses a lot of getters/setters to avoid
    // having to recalculate the model matrix
    // every frame. this is a really easy
    // optimisation to make. 
    // read more about this below

    class Transform {
    protected:

        // marks if this transform has changed this frame
        // will only recalculate the model matrix if true.
        // by default is true to calculate the transform initially.
        bool m_dirty = true;

        // our actual 3d spatial transforms

        // position, initialised to (0,0,0).
        fm::vec3 m_position = {0};

        // rotation, initialised to (0,0,0).
        fm::vec3 m_rotation = {0};

        // scale,    initialised to (1,1,1).
        fm::vec3 m_scale    = {1};

        // skew,     initialised to (0,0,0).
        // as i'm sure you probably haven't used
        // skews before in a transform, they just
        // physically skew the object. 
        // no need, but it's implemented in
        // fmath, so may as well take advantage of it.
        // kinda cool can be used.
        fm::vec3 m_skew     = {0};

        // the transform matrix itself.
        fm::mat4 m_modelMat;

        // set a transform value. 
        // prevents uneccesary verbosity
        // in the setters. without this,
        // m_dirty = true would have to be
        // in every setter which looks ugly.
        template <typename T>
        // get the value to change, m_position.x for example, and the new value to change it to. 
        void _setValue(T& value, T& newValue) {
            // if the value is already the same, no point setting it or marking the transform dirty.
            if (value == newValue) return;

            // set the new value
            value = newValue;
            // mark transform dirty 
            m_dirty = true; 
        }

    public:

        // return the matrix. if dirty, also calculates the matrix.
        fm::mat4 getMatrix();

        // basic getters, they explain themselves
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


        // return if this transform is dirty.
        bool isDirty();


    };
}