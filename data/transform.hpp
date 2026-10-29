#pragma once
#include <fm/fmath.hpp>

namespace Engine {
    
    enum DepthType {
        Planar, // used for opaque sorting (clipping planes and such)
        SquaredRadial // used for transparent sorting
    };

    // forward declaration 
    class Entity;

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

        bool m_globalDirty = true;

        bool m_depthDirty = true;
        // our actual 3d spatial transforms

        // position, initialised to (0,0,0).
        fm::vec3 m_position          = {0};

        // rotation, initialised to (0,0,0).
        fm::vec3 m_rotation          = {0};

        // scale,    initialised to (1,1,1).
        fm::vec3 m_scale             = {1};

        // skew,     initialised to (0,0,0).
        // as i'm sure you probably haven't used
        // skews before in a transform, they just
        // physically skew the object. 
        // no need, but it's implemented in
        // fmath, so may as well take advantage of it.
        // kinda cool can be used.
        fm::vec3 m_skew              = {0};

        // global ones too
        fm::vec3 m_globalPosition    = {0};
        fm::vec3 m_globalRotation    = {0};
        fm::vec3 m_globalScale       = {0};
        fm::vec3 m_globalSkew        = {0};

        // the local directions 
        fm::vec3 m_up                = fm::vec3::yAxis();
        fm::vec3 m_right             = fm::vec3::xAxis();
        fm::vec3 m_front             = fm::vec3::zAxis();

        // the global directions
        fm::vec3 m_globalUp          = fm::vec3::yAxis();
        fm::vec3 m_globalRight       = fm::vec3::xAxis();
        fm::vec3 m_globalFront       = fm::vec3::zAxis();
        
        // the transform matrix itself.
        fm::mat4 m_modelMat;
        fm::mat4 m_globalModelMat;

        // how far is this object from the camera?
        // used for transparency sorting
        float m_depthFromCamera = 0;

        // small optimisation trick. if the last accessed
        // value was global, then the next accessed global
        // does not need to recompute anything.
        bool m_globalMode = false;

        void _setGlobalModeTrue();
        
        // set a transform value. 
        // prevents uneccesary verbosity
        // in the setters. without this,
        // m_dirty = true would have to be
        // in every setter which looks ugly.
        template <typename T>
        // get the value to change, m_position.x for example, and the new value to change it to. 
        void _setValue(T& value, T newValue) {
            // if the value is already the same, no point setting it or marking the transform dirty.
            if (value == newValue) return;

            // set the new value
            value = newValue;
            // mark transform dirty 
            m_dirty = true; 
            // setting a value, global will need to be updated now.
            m_globalMode = false;
            _entity_invalidateChildTransforms();
        }

        // set a global vector
        void _setGlobalVector(fm::vec3& globalv, fm::vec3& localv, fm::vec3 v);

        // pointer to an entity this transform might be on.
        Entity* m_entity;
        // call back to the entity and invalidate the child transforms
        void _entity_invalidateChildTransforms();
        
        // check if this transform's entity has a parent
        bool _hasParent();

        // get this entity's parent transform 
        fm::mat4 _getParentInv();

    public:

        // return how far this transform is from the camera.
        // this is not very accurate for transparency,
        // because we only really have access to the center
        // of an object. 
        float getDepth(fm::vec3 camPos, fm::vec3 camFront, DepthType type); 

        // return the matrix. if dirty, also calculates the matrix.
        fm::mat4 getMatrix(bool forceCompute = false);
        // return the global matrix.
        fm::mat4 getGlobalMatrix();

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

        fm::vec3 getUp();
        fm::vec3 getRight();
        fm::vec3 getFront();
        fm::vec3 getDown();
        fm::vec3 getLeft();
        fm::vec3 getBack();

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

        void addPosition(fm::vec3 v);
        void addRotation(fm::vec3 v);
        void addScale(fm::vec3 v);
        void addSkew(fm::vec3 v);

        void addPositionX(float v);
        void addPositionY(float v);
        void addPositionZ(float v);

        void addRotationX(float v);
        void addRotationY(float v);
        void addRotationZ(float v);

        void addScaleX(float v);
        void addScaleY(float v);
        void addScaleZ(float v);

        void addSkewX(float v);
        void addSkewY(float v);
        void addSkewZ(float v); 

        // getting global values is an expensive task and should not be done. please prevent using it. 
        fm::vec3 getGlobalPosition();
        fm::vec3 getGlobalRotation();
        fm::vec3 getGlobalScale();
        fm::vec3 getGlobalSkew();

        float getGlobalPositionX();
        float getGlobalPositionY();
        float getGlobalPositionZ(); 

        float getGlobalRotationX();
        float getGlobalRotationY();
        float getGlobalRotationZ();
        
        float getGlobalScaleX();
        float getGlobalScaleY();
        float getGlobalScaleZ();
        
        float getGlobalSkewX();
        float getGlobalSkewY();
        float getGlobalSkewZ(); 

        fm::vec3 getGlobalUp();
        fm::vec3 getGlobalRight();
        fm::vec3 getGlobalFront();
        fm::vec3 getGlobalDown();
        fm::vec3 getGlobalLeft();
        fm::vec3 getGlobalBack();

        void setGlobalPosition(fm::vec3 v);
        void setGlobalRotation(fm::vec3 v);
        void setGlobalScale(fm::vec3 v);
        void setGlobalSkew(fm::vec3 v);

        void setGlobalPositionX(float v);
        void setGlobalPositionY(float v);
        void setGlobalPositionZ(float v);

        void setGlobalRotationX(float v);
        void setGlobalRotationY(float v);
        void setGlobalRotationZ(float v);

        void setGlobalScaleX(float v);
        void setGlobalScaleY(float v);
        void setGlobalScaleZ(float v);

        void setGlobalSkewX(float v);
        void setGlobalSkewY(float v);
        void setGlobalSkewZ(float v); 

        void addGlobalPosition(fm::vec3 v);
        void addGlobalRotation(fm::vec3 v);
        void addGlobalScale(fm::vec3 v);
        void addGlobalSkew(fm::vec3 v);

        void addGlobalPositionX(float v);
        void addGlobalPositionY(float v);
        void addGlobalPositionZ(float v);

        void addGlobalRotationX(float v);
        void addGlobalRotationY(float v);
        void addGlobalRotationZ(float v);

        void addGlobalScaleX(float v);
        void addGlobalScaleY(float v);
        void addGlobalScaleZ(float v);

        void addGlobalSkewX(float v);
        void addGlobalSkewY(float v);
        void addGlobalSkewZ(float v); 



        // return if this transform is dirty.
        bool isDirty();
        bool isGlobalDirty();

        friend class Entity;
    };
}