//
// Created by LiangKeJin on 2024/7/27.
//
#pragma once

#include <imgui.h>

/**
 * window size 和 gl size 可能不同
 * 比如在 macos 上, window size 是 1080x720, gl size 是 2160x1440
 * window size = io.DisplaySize
 */
class MainWindow {
public:
    // 初始化, 只调用一次
    static void onInit(ImVec2 windowSize);

    // 在 Imgui::render() 之前
    static void onPreRender(ImGuiIO& io, int glWidth, int glHeight);

    // 画 imgui
    static void onRenderImgui(ImGuiIO& io, int glWidth, int glHeight);

    // 在 Imgui::render() 之后
    static void onPostRender(ImGuiIO& io, int glWidth, int glHeight);

    // 窗口关闭
    static void onExit();
};
