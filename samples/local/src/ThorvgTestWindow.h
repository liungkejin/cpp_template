//
// Created by Kejin on 2026/3/10.
//

#pragma once

#include <imgui.h>

#include "IWindow.h"

class ThorvgTestWindow : public IWindow {
public:
    ThorvgTestWindow() : IWindow("ThorvgTestWindow") {
    }

protected:
    void onInit(ImGuiIO &io, ImVec2 windowSize) override;

    void onClose() override;

public:
    void onRenderImgui(ImGuiIO& io, ImVec2 windowSize) override;
};
