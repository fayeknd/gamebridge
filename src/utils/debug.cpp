#include "imgui/imgui.h"
#include <utils/debug.hpp>
#include <renderer.hpp>
#include <resources.hpp>
#include <component/camera.hpp>
#include <entity.hpp>
#include <input.hpp>
#include <entity.hpp>
#include <meta>

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
    drawPropertyEditor();
}

void DebugUI::drawPropertyEditor() {
    ImGui::ShowDemoWindow();
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


/*
using namespace Engine;

struct ExampleAppPropertyEditor
{
    ImGuiTextFilter     Filter;
    Entity*             SelectedNode = NULL;
    bool                UseClipper = false;

    void Draw(Entity* root_node)
    {
        // Left side: draw tree
        // - Currently using a table to benefit from RowBg feature
        // - Our tree node are all of equal height, facilitating the use of a clipper.
        if (ImGui::BeginChild("##tree", ImVec2(300, 0), ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
        {
            ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
            ImGui::Checkbox("Use Clipper", &UseClipper);
            ImGui::SameLine();
            ImGui::Text("(%d root nodes)", static_cast<int>(root_node->getChildCount()));
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
            if (ImGui::InputTextWithHint("##Filter", "incl -excl", Filter.InputBuf, IM_COUNTOF(Filter.InputBuf), ImGuiInputTextFlags_EscapeClearsAll))
                Filter.Build();
            ImGui::PopItemFlag();

            if (ImGui::BeginTable("##list", 1, ImGuiTableFlags_RowBg))
            {
                if (UseClipper)
                    DrawClippedTree(root_node);
                else
                    DrawTree(root_node);
                ImGui::EndTable();
            }
        }
        ImGui::EndChild();

        // Right side: draw properties
        ImGui::SameLine();

        ImGui::BeginGroup(); // Lock X position
        if (Entity* node = SelectedNode)
        {
            ImGui::Text("%s", node->m_name.c_str());
            ImGui::TextDisabled("UID: 0x%08X", node->getID());
            ImGui::Separator();
            if (ImGui::BeginTable("##properties", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
            {
                // Push object ID after we entered the table, so table is shared for all objects
                ImGui::PushID((int)node->getID());
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 2.0f); // Default twice larger
                {
                    // In a typical application, the structure description would be derived from a data-driven system.
                    // - We try to mimic this with our ExampleMemberInfo structure and the ExampleTreeNodeMemberInfos[] array.
                    // - Limits and some details are hard-coded to simplify the demo.
                    for (const ExampleMemberInfo& field_desc : ExampleTreeNodeMemberInfos)
                    {
                        ImGui::TableNextRow();
                        ImGui::PushID(field_desc.Name);
                        ImGui::TableNextColumn();
                        ImGui::AlignTextToFramePadding();
                        ImGui::TextUnformatted(field_desc.Name);
                        ImGui::TableNextColumn();
                        void* field_ptr = (void*)(((unsigned char*)node) + field_desc.Offset);
                        switch (field_desc.DataType)
                        {
                        case ImGuiDataType_Bool:
                        {
                            IM_ASSERT(field_desc.DataCount == 1);
                            ImGui::Checkbox("##Editor", (bool*)field_ptr);
                            break;
                        }
                        case ImGuiDataType_S32:
                        {
                            int v_min = INT_MIN, v_max = INT_MAX;
                            ImGui::SetNextItemWidth(-FLT_MIN);
                            ImGui::DragScalarN("##Editor", field_desc.DataType, field_ptr, field_desc.DataCount, 1.0f, &v_min, &v_max);
                            break;
                        }
                        case ImGuiDataType_Float:
                        {
                            float v_min = 0.0f, v_max = 1.0f;
                            ImGui::SetNextItemWidth(-FLT_MIN);
                            ImGui::SliderScalarN("##Editor", field_desc.DataType, field_ptr, field_desc.DataCount, &v_min, &v_max);
                            break;
                        }
                        case ImGuiDataType_String:
                        {
                            ImGui::InputText("##Editor", reinterpret_cast<char*>(field_ptr), 28);
                            break;
                        }
                        }
                        ImGui::PopID();
                    }
                }
                ImGui::PopID();
                ImGui::EndTable();
            }
        }
        ImGui::EndGroup();
    }

    template <typename T>
    void drawComponentInspectorValues(T& t) {
        constexpr std::meta::info
    }

    // Custom search filter
    // - Here we apply on root node only.
    // - This does a case insensitive stristr which is pretty heavy. In a real large-scale app you would likely store a filtered list which in turns would be trivial to linearize.
    inline bool IsNodePassingFilter(ExampleTreeNode* node)
    {
        return node->Parent->Parent != NULL || Filter.PassFilter(node->Name);
    }

    // Basic version, recursive. This is how you would generally draw a tree.
    // - Simple but going to be noticeably costly if you have a large amount of nodes as DrawTreeNode() is called for all of them.
    // - On my desktop PC (2020), for 10K nodes in an optimized build this takes ~1.2 ms
    // - Unlike arrays or grids which are very easy to clip, trees are currently more difficult to clip.
    void DrawTree(ExampleTreeNode* node)
    {
        for (ExampleTreeNode* child : node->Childs)
            if (IsNodePassingFilter(child) && DrawTreeNode(child))
            {
                DrawTree(child);
                ImGui::TreePop();
            }
    }

    // More advanced version. Use a alternative clipping technique: fast-forwarding through non-visible chunks.
    // - On my desktop PC (2020), for 10K nodes in an optimized build this takes ~0.1 ms
    //   (in ExampleTree_CreateDemoTree(), change 'int ROOT_ITEMS_COUNT = 10000' to try with this amount of root nodes).
    // - 1. Use clipper with indeterminate count (items_count = INT_MAX): we need to call SeekCursorForItem() at the end once we know the count.
    // - 2. Use SetNextItemStorageID() to specify ID used for open/close storage, making it easy to call TreeNodeGetOpen() on any arbitrary node.
    // - 3. Linearize tree during traversal: our tree data structure makes it easy to access sibling and parents.
    // - Unlike clipping for a regular array or grid which may be done using random access limited to visible areas,
    //   this technique requires traversing most accessible nodes. This could be made more optimal with extra work,
    //   but this is a decent simplicity<>speed trade-off.
    // See https://github.com/ocornut/imgui/issues/3823 for discussions about this.
    void DrawClippedTree(ExampleTreeNode* root_node)
    {
        ExampleTreeNode* node = root_node->Childs[0]; // First node
        ImGuiListClipper clipper;
        clipper.Begin(INT_MAX);
        while (clipper.Step())
            while (clipper.UserIndex < clipper.DisplayEnd && node != NULL)
                node = DrawClippedTreeNodeAndAdvanceToNext(&clipper, node);

        // Keep going to count nodes and submit final count so we have a reliable scrollbar.
        // - One could consider caching this value and only refreshing it occasionally e.g. window is focused and an action occurs.
        // - Incorrect but cheap approximation would be to use 'clipper_current_idx = IM_MAX(clipper_current_idx, root_node->Childs.Size)' instead.
        // - If either of those is implemented, the general cost will approach zero when scrolling is at the top of the tree.
        while (node != NULL)
            node = DrawClippedTreeNodeAndAdvanceToNext(&clipper, node);
        //clipper.UserIndex = IM_MAX(clipper.UserIndex, root_node->Childs.Size); // <-- Cheap approximation instead of while() loop above.
        clipper.SeekCursorForItem(clipper.UserIndex);
    }

    ExampleTreeNode* DrawClippedTreeNodeAndAdvanceToNext(ImGuiListClipper* clipper, ExampleTreeNode* node)
    {
        if (IsNodePassingFilter(node))
        {
            // Draw node if within visible range
            bool is_open = false;
            if (clipper->UserIndex >= clipper->DisplayStart && clipper->UserIndex < clipper->DisplayEnd)
            {
                is_open = DrawTreeNode(node);
            }
            else
            {
                is_open = (node->Childs.Size > 0 && ImGui::TreeNodeGetOpen((ImGuiID)node->UID));
                if (is_open)
                    ImGui::TreePush(node->Name);
            }
            clipper->UserIndex++;

            // Next node: recurse into childs
            if (is_open)
                return node->Childs[0];
        }

        // Next node: next sibling, otherwise move back to parent
        while (node != NULL)
        {
            if (node->IndexInParent + 1 < node->Parent->Childs.Size)
                return node->Parent->Childs[node->IndexInParent + 1];
            node = node->Parent;
            if (node->Parent == NULL)
                break;
            ImGui::TreePop();
        }
        return NULL;
    }

    // To support node with same name we incorporate node->UID into the item ID.
    // (this would more naturally be done using PushID(node->UID) + TreeNodeEx(node->Name, tree_flags),
    //   but it would require in DrawClippedTreeNodeAndAdvanceToNext() to add PushID() before TreePush(), and PopID() after TreePop(),
    //   so instead we use TreeNodeEx(node->UID, tree_flags, "%s", node->Name) here)
    bool DrawTreeNode(ExampleTreeNode* node)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_None;
        tree_flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick; // Standard opening mode as we are likely to want to add selection afterwards
        tree_flags |= ImGuiTreeNodeFlags_NavLeftJumpsToParent;  // Left arrow support
        tree_flags |= ImGuiTreeNodeFlags_SpanFullWidth;         // Span full width for easier mouse reach
        tree_flags |= ImGuiTreeNodeFlags_DrawLinesToNodes;      // Always draw hierarchy outlines
        if (node == SelectedNode)
            tree_flags |= ImGuiTreeNodeFlags_Selected;          // Draw selection highlight
        if (node->Childs.Size == 0)
            tree_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen; // Use _NoTreePushOnOpen + set is_open=false to avoid unnecessarily push/pop on leaves.
        if (node->DataMyBool == false)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        ImGui::SetNextItemStorageID((ImGuiID)node->UID);        // Use node->UID as storage id
        bool is_open = ImGui::TreeNodeEx((void*)(intptr_t)node->UID, tree_flags, "%s", node->Name);
        if (node->Childs.Size == 0)
            is_open = false;
        if (node->DataMyBool == false)
            ImGui::PopStyleColor();
        if (ImGui::IsItemFocused())
            SelectedNode = node;
        return is_open;
    }
};
*/