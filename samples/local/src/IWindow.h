//
// Created by wutacam on 2025/1/14.
//

#pragma once

#include <imgui.h>
#include <string>

class IWindow {
public:
    explicit IWindow(const std::string &name, bool subWindow = true) : m_name(name), m_sub_window(subWindow) {
    }

    virtual ~IWindow() {
    }

public:
    const std::string &name() const {
        return m_name;
    }

    void init(ImGuiIO &io, ImVec2 windowSize) {
        onInit(io, m_sub_window ? windowSize : io.DisplaySize);
    }

    void renderImgui(ImGuiIO &io, ImVec2 windowPos, ImVec2 windowSize) {
        if (m_sub_window) {
            ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
            ImGui::Begin(m_name.c_str(), 0, flags);
            ImGui::SetWindowPos(windowPos);
            ImGui::SetWindowSize(windowSize);
            onRenderImgui(io, windowSize);
        } else {
            onRenderImgui(io, io.DisplaySize);
        }
        if (m_sub_window) {
            ImGui::End();
        }
    }

    void close() {
        onClose();
    }

protected:
    virtual void onInit(ImGuiIO &io, ImVec2 windowSize) {}

    virtual void onRenderImgui(ImGuiIO &io, ImVec2 windowSize) = 0;

    virtual void onClose() {}

private:
    const std::string m_name;
    const bool m_sub_window;
};
