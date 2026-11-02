#pragma once
#include <gl_context.hpp>
#include <material.hpp>
#include <component/component.hpp>

// this probably didn't have to be its own file lol
// but whatev

namespace Engine {

    // this determines 
    struct RenderCommand {
        uint64_t m_sortKey;
        IComponent* m_component;
        void (*drawCallback)(IComponent*);
    };
}