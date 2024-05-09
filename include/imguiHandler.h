#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include <implot.h>

#include "application.h"

namespace ImGuiHandler
{
    void ControlTab(bool& showDemoWindow, Application& application);
    void VisualizeTab(bool& showDemoWindow, Application& application);

    void Init(const char* glslVersion, GLFWwindow* window)
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();
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

        ImGui::Begin("Main");
        if (ImGui::BeginTabBar("MainTab"))
        {
            if (ImGui::BeginTabItem("Controls"))
            {
                ControlTab(showDemoWindow, application);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Visualize"))
            {
                VisualizeTab(showDemoWindow, application);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        // FIXME: ImGui::End() called in main.cpp
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

        // To invert the image
        //ImGui::Image((void*)(unsigned int)(*texture), getRegion, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::Image((void*)(unsigned int)(*texture), getRegion);
        ImGui::PopStyleVar(3);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ControlTab(bool& showDemoWindow, Application& application)
    {
        if (showDemoWindow)
            ImPlot::ShowDemoWindow(&showDemoWindow);

        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::Checkbox("Demo Window", &showDemoWindow);
        ImGui::NewLine();
        ImGui::NewLine();

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Building Algorithms"))
        {
            if ((!application.m_MazeBuilder || !application.m_MazeBuilder->m_Completed) &&
                !application.IsButtonPressed(Application::BUILDER_RECURSIVE_BACKTRACK) &&
                !application.IsButtonPressed(Application::BUILDER_KRUSKAL) &&
                !application.IsButtonPressed(Application::BUILDER_PRIMS))
            {
                application.m_ButtonStates |= (ImGui::Button("Recursive Backtrack") ? Application::BUILDER_RECURSIVE_BACKTRACK : 0x00);
                application.m_ButtonStates |= (ImGui::Button("Kruskal") ? Application::BUILDER_KRUSKAL : 0x00);
                application.m_ButtonStates |= (ImGui::Button("Prims") ? Application::BUILDER_PRIMS : 0x00);
                application.m_ButtonStates |= (ImGui::Button("Wilson") ? Application::BUILDER_WILSON : 0x00);
            }
            else if ((application.m_MazeBuilder && application.m_MazeBuilder->m_Completed) ||
                application.IsButtonPressed(Application::BUILDER_RECURSIVE_BACKTRACK) ||
                application.IsButtonPressed(Application::BUILDER_KRUSKAL) ||
                application.IsButtonPressed(Application::BUILDER_PRIMS) ||
                application.IsButtonPressed(Application::BUILDER_WILSON))
            {
                ImGui::BeginDisabled();
                ImGui::Button("Recursive Backtrack");
                ImGui::Button("Kruskal");
                ImGui::Button("Prims");
                ImGui::Button("Wilson");
                ImGui::EndDisabled();
            }
            ImGui::TreePop();
        }
        ImGui::NewLine();

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Solving Algorithms"))
        {
            if (application.m_MazeBuilder && application.m_MazeBuilder->m_Completed
                && !application.IsButtonPressed(Application::SOLVER_DFS) && !application.IsButtonPressed(Application::SOLVER_BFS)
                && !application.IsButtonPressed(Application::SOLVER_DIJKSTRA) && !application.IsButtonPressed(Application::SOLVER_ASTAR))
            {
                application.m_ButtonStates |= (ImGui::Button("Depth First Search") ? Application::SOLVER_DFS : 0);
                application.m_ButtonStates |= (ImGui::Button("Breadth First Search") ? Application::SOLVER_BFS : 0);
                application.m_ButtonStates |= (ImGui::Button("Dijkstra") ? Application::SOLVER_DIJKSTRA : 0);
                application.m_ButtonStates |= (ImGui::Button("A star") ? Application::SOLVER_ASTAR : 0);
            }
            else if (!application.m_MazeBuilder || !application.m_MazeBuilder->m_Completed ||
                application.IsButtonPressed(Application::BUILDER_RECURSIVE_BACKTRACK) || application.IsButtonPressed(Application::BUILDER_KRUSKAL) ||
                application.IsButtonPressed(Application::SOLVER_DIJKSTRA) || application.IsButtonPressed(Application::SOLVER_ASTAR))
            {
                ImGui::BeginDisabled();
                ImGui::Button("Depth first search");
                ImGui::Button("Breadth first search");
                ImGui::Button("Dijkstra");
                ImGui::Button("A star");
                ImGui::EndDisabled();
            }
            ImGui::TreePop();
        }
        ImGui::NewLine();

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Main"))
        {
            uint16_t lower1 = 1, higher1 = 200, higher2 = 100;
            uint32_t lower3 = 0, higher3 = application.m_Maze->m_HalfCellHeight, higher4 = application.m_Maze->m_MazeArea - 1;

            uint16_t cellWidth = application.m_Maze->m_HalfCellHeight;
            uint16_t wallWidth = application.m_Maze->m_WallThickness;

            ImGui::SliderInt("Delay (ms)", &application.m_Delay, 0, 200);
            ImGui::SliderScalar("Cell Width", ImGuiDataType_U16, &application.m_Maze->m_HalfCellHeight, &lower1, &higher1);
            ImGui::SliderScalar("Wall Width", ImGuiDataType_U16, &application.m_Maze->m_WallThickness, &lower1, &higher2);
            ImGui::SliderScalar("Path Width", ImGuiDataType_U16, &application.m_Maze->m_LineThickness, &lower1, &higher3);
            ImGui::SliderScalar("Start Cell", ImGuiDataType_U32, &application.m_Route.first, &lower3, &higher4);
            ImGui::SliderScalar("End Cell", ImGuiDataType_U32, &application.m_Route.second, &lower3, &higher4);

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

        //ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Colors"))
        {
            ImGui::ColorEdit4("Maze", application.m_Maze->m_ColorMaze);
            ImGui::ColorEdit4("Start", application.m_Maze->m_ColorStart);
            ImGui::ColorEdit4("End", application.m_Maze->m_ColorEnd);
            ImGui::ColorEdit4("Path", application.m_ColorPath);
            ImGui::ColorEdit4("Search", application.m_Maze->m_ColorSearched);
            ImGui::ColorEdit4("Search Top", application.m_Maze->m_ColorSearchTop);
            ImGui::ColorEdit4("Background", application.m_Maze->m_ColorBackground);
            ImGui::TreePop();
        }

        //ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Animation"))
        {
            ImGui::Checkbox("Path Animation", &application.m_PathAnimation);
            ImGui::SliderFloat("Color Cycle", &application.m_PathSpeed, 0, 0.2);
            ImGui::TreePop();
        }
    }

    void VisualizeTab(bool& showDemoWindow, Application& application)
    {

        static int minDisplayValue = 0;
        static int maxDisplayValue = application.m_Maze->m_RandUpperLimit;
        static int previousValue = application.m_Maze->m_RandUpperLimit;

        static bool displayValue = false;
        static char fmtData[4]{};

        static ImPlotColormap map = ImPlotColormap_Viridis;
        if (ImPlot::ColormapButton(ImPlot::GetColormapName(map), ImVec2(225, 0), map)) {
            map = (map + 1) % ImPlot::GetColormapCount();
            // We bust the color cache of our plots so that item colors will
            // resample the new colormap in the event that they have already
            // been created. See documentation in implot.h.
            //BustColorCache("##Heatmap1");
        }

        ImGui::SameLine();
        ImGui::LabelText("##Colormap Index", "%s", "Change Colormap");
        ImGui::SetNextItemWidth(225);
        ImGui::SliderInt("Min Display", &minDisplayValue, 0, 500);
        ImGui::SetNextItemWidth(225);
        ImGui::SliderInt("Max Display", &maxDisplayValue, 1, 500);

        if (minDisplayValue >= maxDisplayValue)
            minDisplayValue = maxDisplayValue-1;

        ImGui::Checkbox("Display Values", &displayValue);

        if (displayValue)
        {
            fmtData[0] = '%';
            fmtData[1] = 'i';
            fmtData[2] = '\0';
        }
        else
        {
            fmtData[0] = '\0';
        }

        static ImPlotHeatmapFlags hm_flags = ImPlotHeatmapFlags_ColMajor;
        
        ImPlot::PushColormap(map);

        if (ImPlot::BeginPlot("##Heatmap1", ImVec2(225, 225), ImPlotFlags_NoLegend | ImPlotFlags_NoMouseText))
        {
            ImPlot::PlotHeatmap("heat", application.m_Maze->m_CellWeights.data(), application.m_Maze->m_CellsAcrossHeight, application.m_Maze->m_CellsAcrossWidth, minDisplayValue, maxDisplayValue, fmtData, ImPlotPoint(0, 0), ImPlotPoint(application.m_Maze->m_CellsAcrossWidth, application.m_Maze->m_CellsAcrossHeight), hm_flags);
            ImPlot::EndPlot();
        }
        ImGui::SameLine();
        ImPlot::ColormapScale("##HeatScale", minDisplayValue, maxDisplayValue, ImVec2(60, 225));
        ImPlot::PopColormap();

        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.8f, 0.8f));
        if (ImGui::Button("Reassign Weights"))
        {
            application.m_Maze->SetCellWeights();
        }
        ImGui::PopStyleColor(3);
        ImGui::SetNextItemWidth(225);
        ImGui::SliderInt("Max Weight Value", &application.m_Maze->m_RandUpperLimit, 1, 500);
        if (previousValue != application.m_Maze->m_RandUpperLimit)
        {
            previousValue = application.m_Maze->m_RandUpperLimit;
            application.m_Maze->SetCellWeights();
        }
    }
}