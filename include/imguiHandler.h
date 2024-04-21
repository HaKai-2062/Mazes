#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui_internal.h>


namespace ImGuiHandler
{
    void Init(const char* glslVersion, GLFWwindow* window)
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
        //io.ConfigViewportsNoAutoMerge = true;
        //io.ConfigViewportsNoTaskBarIcon = true;

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glslVersion);

        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        // Dockspace setup
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;
    }

    void BeginFrame(ImGuiID& dockSpaceID, bool& showDemoWindow, Application& application)
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        dockSpaceID = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_HiddenTabBar);
        
        if (showDemoWindow)
            ImGui::ShowDemoWindow(&showDemoWindow);
        
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Begin("Controls");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::Checkbox("Demo Window", &showDemoWindow);
        ImGui::NewLine();
        ImGui::NewLine();
        
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Building Algorithms"))
        {
            if ((!application.m_MazeBuilder || !application.m_MazeBuilder->m_Completed) &&
                !application.IsButtonPressed(Application::BUILDER_RECURSIVE_BACKTRACK) && !application.IsButtonPressed(Application::BUILDER_KRUSKAL))
            {
                application.m_ButtonStates |= (ImGui::Button("Recursive Backtrack") ? Application::BUILDER_RECURSIVE_BACKTRACK : 0x00);
                application.m_ButtonStates |= (ImGui::Button("Kruskal") ? Application::BUILDER_KRUSKAL : 0x00);
            }
            else if ((application.m_MazeBuilder && application.m_MazeBuilder->m_Completed) ||
                application.IsButtonPressed(Application::BUILDER_RECURSIVE_BACKTRACK) || application.IsButtonPressed(Application::BUILDER_KRUSKAL))
            {
                ImGui::BeginDisabled();
                ImGui::Button("Recursive Backtrack");
                ImGui::Button("Kruskal");
                ImGui::EndDisabled();
            }
            ImGui::TreePop();
        }
        ImGui::NewLine();

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Solving Algorithms"))
        {
            if (application.m_MazeBuilder && application.m_MazeBuilder->m_Completed && !application.IsButtonPressed(Application::SOLVER_DFS) && !application.IsButtonPressed(Application::SOLVER_BFS))
            {
                application.m_ButtonStates |= (ImGui::Button("Depth First Search") ? Application::SOLVER_DFS : 0);
                application.m_ButtonStates |= (ImGui::Button("Breadth First Search") ? Application::SOLVER_BFS : 0);
            }
            else if (!application.m_MazeBuilder || !application.m_MazeBuilder->m_Completed ||
                application.IsButtonPressed(Application::BUILDER_RECURSIVE_BACKTRACK) || application.IsButtonPressed(Application::BUILDER_KRUSKAL))
            {
                ImGui::BeginDisabled();
                ImGui::Button("Depth first search");
                ImGui::Button("Breadth first search");
                ImGui::EndDisabled();
            }
            ImGui::TreePop();
        }
        ImGui::NewLine();

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Main"))
        {
            uint16_t lower1 = 1, higher1 = 200, higher2 = 100;
            uint32_t lower3 = 0, higher3 = application.m_Maze->m_MazeArea-1;

            uint16_t cellWidth = application.m_Maze->m_HalfCellHeight;
            uint16_t wallWidth = application.m_Maze->m_WallThickness;

            ImGui::SliderInt("Delay (ms)", &application.m_Delay, 0, 200);
            ImGui::SliderScalar("Cell Half Width (px)", ImGuiDataType_U16, &application.m_Maze->m_HalfCellHeight, &lower1, &higher1);
            ImGui::SliderScalar("Wall Width (px)", ImGuiDataType_U16, &application.m_Maze->m_WallThickness, &lower1, &higher2);
            ImGui::SliderScalar("Start Cell", ImGuiDataType_U32, &application.m_Route.first, &lower3, &higher3);
            ImGui::SliderScalar("End Cell", ImGuiDataType_U32, &application.m_Route.second, &lower3, &higher3);

            if (cellWidth != application.m_Maze->m_HalfCellHeight || wallWidth != application.m_Maze->m_WallThickness)
            {
                cellWidth = application.m_Maze->m_HalfCellHeight;
                wallWidth = application.m_Maze->m_WallThickness;
                application.DeleteMaze();
                application.m_Maze = new Maze(*application.m_Width, *application.m_Height, cellWidth, wallWidth);
            }

            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Resetting"))
        {
            if (!application.m_MazeSolver || !application.m_MazeSolver->m_Completed)
            {
                ImGui::BeginDisabled();
                ImGui::Button("Reset Path");
                ImGui::EndDisabled();
            }
            else
            {
                application.m_ButtonStates |= (ImGui::Button("Reset Path") ? Application::PATH : 0x00);
            }
            application.m_ButtonStates |= (ImGui::Button("Reset All") ? Application::MAZE : 0x00);
            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Colors"))
        {
            ImGui::ColorEdit4("Maze", application.m_Maze->m_ColorMaze);
            ImGui::ColorEdit4("Path", application.m_ColorPath);
            ImGui::ColorEdit4("Search", application.m_Maze->m_ColorSearched);
            ImGui::ColorEdit4("Stack Top", application.m_Maze->m_ColorStackTop);
            ImGui::ColorEdit4("Background", application.m_Maze->m_ColorBackground);
            ImGui::NewLine();
            ImGui::TextWrapped("The following must be unchecked to make the custom Path color appear");
            ImGui::Checkbox("Animation 1", &application.m_ColorPathAnimation1);

            ImGui::TreePop();
        }
    }

    void EndFrame(ImGuiID dockSpaceID, unsigned int* texture, ImVec2& getRegion, bool& imguiWindowResized)
    {
        ImGui::SetNextWindowDockID(dockSpaceID, ImGuiCond_FirstUseEver);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0.f, 0.f));
        ImGui::Begin("SceneWindow", false, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar);
        if (getRegion.x != ImGui::GetContentRegionAvail().x || getRegion.y != ImGui::GetContentRegionAvail().y)
            imguiWindowResized = true;
        getRegion = ImGui::GetContentRegionAvail();

        ImGui::Image((void*)(unsigned int)(*texture), getRegion);
        ImGui::PopStyleVar(3);
        ImGui::End();

        //ImGui::Begin("test");
        //ImGui::Text("%.1f, %.1f", getRegion.x, getRegion.y);
        //ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}