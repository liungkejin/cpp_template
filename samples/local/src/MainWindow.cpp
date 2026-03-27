//
// Created by LiangKeJin on 2024/7/27.
//

#include "MainWindow.h"

#include <string>
#include <vector>

#include "imgui_impl_opengl3_loader.h"
#include "IWindow.h"


static bool g_show_demo_window = false;
static std::vector<std::pair<std::string, std::shared_ptr<IWindow>>> g_all_windows;

template <typename T>
static void checkAddOrRemoveWindow(bool show, const std::string& name, ImGuiIO& io, ImVec2 windowSize) {
    if (show) {
        for (const auto& [fst, snd] : g_all_windows) {
            if (fst == name) {
                return;
            }
        }
        auto window = std::make_shared<T>();
        window->init(io, windowSize);
        g_all_windows.push_back({name, window});
    } else {
        for (auto it = g_all_windows.begin(); it != g_all_windows.end(); ++it) {
            if (it->first == name) {
                it->second->close();
                it = g_all_windows.erase(it);
                break;
            }
        }
    }
}

void MainWindow::onInit(ImVec2 windowSize) {
    // std::string version = GLUtil::glVersion();
    // _INFO("onInit, window size(%.1fx%.1f), GL version: %s", windowSize.x, windowSize.y, version);
}

void MainWindow::onPreRender(ImGuiIO& io, int glWidth, int glHeight) {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
}

void MainWindow::onRenderImgui(ImGuiIO &io, int glWidth, int glHeight) {

    auto windowWidth = io.DisplaySize.x;
    auto windowHeight = io.DisplaySize.y;

    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 6.0f;
    style.FrameBorderSize = 1.0f;
    if (g_show_demo_window) {
        ImGui::ShowDemoWindow(&g_show_demo_window);
    }

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Main", 0, flags);
    ImGui::SetWindowPos(ImVec2(0, 0));
    int tabWidth = 150;
    ImGui::SetWindowSize(ImVec2(tabWidth, windowHeight));

    ImGui::Text("应用帧率 (%.1f FPS)", io.Framerate);
    ImGui::Checkbox("显示 demo window", &g_show_demo_window);

    ImGui::End();

    for (const auto& [fst, snd] : g_all_windows) {
        snd->renderImgui(io, ImVec2(tabWidth, 0), ImVec2(windowWidth-tabWidth, windowHeight));
    }
}

void MainWindow::onPostRender(ImGuiIO& io, int glWidth, int glHeight) {
    //
}

void MainWindow::onExit() {
    g_all_windows.clear();
}
