#include "imgui/imgui.h"
#include <utils/debug.hpp>
#include <renderer.hpp>
#include <resources.hpp>
#include <component/camera.hpp>
#include <entity.hpp>
#include <input.hpp>

using namespace Engine::Debug;

void DebugUI::_createImGuiContext(GLFWwindow* wnd) {

    // https://github.com/ocornut/imgui/wiki/Getting-Started#example-if-you-are-using-glfw--openglwebgl

    // make sure the imgui context doesn't exist yet
    if (m_hasInit) return;

    // initialise ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // i don't kno
    m_io = &ImGui::GetIO();
    m_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    m_io->ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange; // cause it cant be fucking trusted.

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(wnd, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    // load the awesome font
    m_font_cfg.FontDataOwnedByAtlas = false; 

    // keep data alive
    static bytes font = ResourceManager::loadRaw("stuff=>fonts/OpenSans_Bold.ttf");

    m_font = m_io->Fonts->AddFontFromMemoryTTF(
        font.data(),       
        font.size(),
        16.0f,
        &m_font_cfg
    );
    IM_ASSERT(m_font != NULL);
    ImGui::PushFont(m_font);

    m_hasInit = true;
}

void DebugUI::_destroyImGuiContext() {
    
    // make sure a context DOES exist
    if (!m_hasInit) return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    m_hasInit = false;
}

void DebugUI::drawImGuiInterface_framebegin() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame(); 
    if (m_drawDebugger) drawDebugger();
}

void DebugUI::drawDebugger() {

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.2f, 0.2f, 0.4f)); 
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always);

    ImVec2 lastPos(0.0f, 0.0f);
    ImVec2 lastSize(0.0f, 0.0f);
    float spacing = 10.0f;
    float nextY;

    ImGuiWindowFlags flags = (  
          ImGuiWindowFlags_NoTitleBar 
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings
    );
    if (ImGui::Begin("_perf_wnd", NULL, flags)) {

        ImGui::Text("Performance");
        ImGui::Separator();

        ImGui::Text("%.1f FPS\n%.1f Target FPS\n%.5f ms/frame", 
            static_cast<double>(Renderer::get().getFPS()),
            static_cast<double>(Renderer::get().getTargetFPS()), 
            Time::deltaTime());

        lastPos = ImGui::GetWindowPos();
        lastSize = ImGui::GetWindowSize();
        nextY = lastPos.y + lastSize.y + spacing;
        ImGui::End();
    }

    ImGui::SetNextWindowPos(ImVec2(lastPos.x, nextY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(150, 0));
    if (ImGui::Begin("info_wnd", NULL, flags)) {

        auto cam = Camera::activeCamera();
        if (cam) {
            ImGui::Text("Active Camera");
            ImGui::Separator();

            ImGui::PushItemWidth(-1.0f); 

            ImGui::Text("Position");
            ImGui::Text("X:%.2f Y:%.2f Z:%.2f", 
                cam->getEntity().transform.getGlobalPositionX(),
                cam->getEntity().transform.getGlobalPositionY(),
                cam->getEntity().transform.getGlobalPositionZ() 
            );
            ImGui::Text("Rotation");
            ImGui::Text("X:%.2f Y:%.2f Z:%.2f", 
                cam->getEntity().transform.getGlobalRotationX(),
                cam->getEntity().transform.getGlobalRotationY(),
                cam->getEntity().transform.getGlobalRotationZ() 
            );
            ImGui::Separator();
            ImGui::Text("Front");
            ImGui::Text("X:%.2f Y:%.2f Z:%.2f", 
                cam->getEntity().transform.getGlobalFront().x,
                cam->getEntity().transform.getGlobalFront().y,
                cam->getEntity().transform.getGlobalFront().z 
            );
            ImGui::Text("Right");
            ImGui::Text("X:%.2f Y:%.2f Z:%.2f", 
                cam->getEntity().transform.getGlobalRight().x,
                cam->getEntity().transform.getGlobalRight().y,
                cam->getEntity().transform.getGlobalRight().z 
            );
            ImGui::Text("Up");
            ImGui::Text("X:%.2f Y:%.2f Z:%.2f", 
                cam->getEntity().transform.getGlobalUp().x,
                cam->getEntity().transform.getGlobalUp().y,
                cam->getEntity().transform.getGlobalUp().z 
            );
            ImGui::Separator();

            float fov = cam->getFOV();
            float nc = cam->getNearClip();
            float fc = cam->getFarClip();
            float ortho = cam->getOrthoSize();
            int proj = cam->getProjectionMode();
            std::string projFormat;
            switch (proj) {
                case (1):  projFormat = "Orthographic";      break;
                case (0):  projFormat = "Perspective";       break;
            }
            if (proj == 1) {
                ImGui::Text("Ortho Size");
                if (ImGui::SliderFloat("##Ortho Size", &ortho, 0.1, 10)) {
                    cam->setOrthoSize(ortho);
                }
            }
            else {
                ImGui::Text("FOV");
                if (ImGui::SliderFloat("##FOV", &fov, 30, 120)) {
                    cam->setFOV(fov);
                }
            }
            
            ImGui::Text("Near Clip");
            if (ImGui::SliderFloat("##Near Clip", &nc, 0.01, 1000)) {
                cam->setNearClip(nc);
            }
            ImGui::Text("Far Clip");
            if (ImGui::SliderFloat("##Far Clip", &fc, 0.01, 1000)) {
                cam->setFarClip(fc);
            }
            ImGui::Text("Projection");
            if (ImGui::SliderInt("##Projection", &proj, 0, 1, projFormat.c_str())) {
                cam->setProjectionMode(static_cast<ProjectionMode>(proj));
            }
        }

        lastPos = ImGui::GetWindowPos();
        lastSize = ImGui::GetWindowSize();
        nextY = lastPos.y + lastSize.y + spacing;
        ImGui::End();
    }



    ImGui::SetNextWindowPos(ImVec2(lastPos.x, nextY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(130, 0));
    if (ImGui::Begin("_render_wnd", NULL, flags)) {
        
        ImGui::Text("Draw Options");
        ImGui::Separator();

        float fps = Renderer::get().getTargetFPS();
        int vsm = Renderer::get().getVSyncMode();
        std::string vsmFormat;

        switch (vsm) {
            case (0):  vsmFormat = "Off";      break;
            case (1):  vsmFormat = "On";       break;
            default:   vsmFormat = "Adaptive"; break;
        }

        ImGui::PushItemWidth(-1.0f); 

        ImGui::Checkbox("Draw Debug", &m_drawDebugTools);
        ImGui::Text("Target FPS");
        if (ImGui::SliderFloat("##Target FPS", &fps, 1, 240, "%.01f")) {
            Renderer::get().setTargetFPS(fps);
        }
        ImGui::Text("VSync Mode");
        if (ImGui::SliderInt("##Vsync mode", &vsm, -1, 1, vsmFormat.c_str())) {
            Renderer::get().setVSyncMode(static_cast<VSyncMode>(vsm));
        }

        ImGui::End();
    }
    ImGui::PopStyleColor(2);
}

void DebugUI::drawImGuiInterface_frameend() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DebugLine::_genLine() {
    m_program = ShaderProgram::getShaderViaKey(shader_lineShader);
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    
    // position
    glVertexAttribPointer(shaderVertexAttrib_Position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)offsetof(Vertex, position));
    glEnableVertexAttribArray(shaderVertexAttrib_Position);

    //colour
    glVertexAttribPointer(shaderVertexAttrib_Colour,   4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)offsetof(Vertex, colour));
    glEnableVertexAttribArray(shaderVertexAttrib_Colour);

    glBindVertexArray(0);
}

void DebugLine::_deleteLine() {
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

DebugLine::DebugLine() {
    _genLine();
}

DebugLine::~DebugLine() {
    _deleteLine();
}

// Accepts any number of points using an initializer list: DrawLines(mvp, color, {a, b, c})
void DebugLine::draw(std::vector<Engine::Vertex> points) {
    if (!m_program || !DebugUI::get().m_drawDebugTools) return;
    m_renderData.useRenderData();


    // don't bother with anything smaller than 2 points
    if (points.size() < 2) return;

    // ViewProjection matrix
    fm::mat4 vpMat = fm::mat4::identity();
    // if main camera exists
    if (Camera::activeCamera()) {
        // vpMat is the main camera projection * view
        vpMat = Camera::activeCamera()->getProjectionMat() * Camera::activeCamera()->getViewMat();
    }
    m_program->setUniform(shader_ViewProjectionMatrix, vpMat);

    // bind the vertex array
    glBindVertexArray(m_vao);
    // bind the vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // upload the points dynamically.
    // this is a debug tool, so it's not performant.
    // we can just upload the data every frame.
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Vertex), points.data(), GL_STREAM_DRAW);
    glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(points.size()));

    glBindVertexArray(0);
}

