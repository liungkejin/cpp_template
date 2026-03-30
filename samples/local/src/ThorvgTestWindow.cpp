//
// Created by LiangKeJin on 2026/3/10.
//
#include "ThorvgTestWindow.h"

#include <cmrc/cmrc.hpp>
#include <thorvg-1/thorvg.h>

#include "znative/utils/ThreadUtils.h"
#include "znative/gles/Framebuffer.h"
#include "znative/utils/TimeUtils.h"

struct ZImage {
    int width = 0;
    int height = 0;
    std::shared_ptr<void> data = nullptr;
};

CMRC_DECLARE(local);

ZImage decodeImage() {
    auto fs = cmrc::local::get_filesystem();
    auto file = fs.open("assets/images/lyf.jpg");
    tvg::Picture *p = tvg::Picture::gen();
    if (p->load(file.begin(), file.size(), "jpg", nullptr) != tvg::Result::Success) {
        tvg::Paint::rel(p);
        return {};
    }
    float w, h;
    p->size(&w, &h);
    if (w < 1 || h < 1) {
        tvg::Paint::rel(p);
        return {};
    }

    ZImage image;
    image.width = (int)w;
    image.height = (int)h;
    image.data = std::shared_ptr<uint8_t>(new uint8_t[image.width*image.height*4]);

    auto tempCanvas = tvg::SwCanvas::gen();
    tempCanvas->target((uint32_t *)image.data.get(), image.width, image.width, image.height,
                       tvg::ColorSpace::ABGR8888);
    tempCanvas->add(p); // ownership transferred
    tempCanvas->update();
    tempCanvas->draw(true);
    tempCanvas->sync();
    delete tempCanvas;

    return image;
}

static ZImage test_image;

static ZImage sw_target;
static znative::ImageTexture sw_texture;

static znative::Framebuffer gl_target;

void ThorvgTestWindow::onInit(ImGuiIO &io, ImVec2 windowSize) {
    tvg::Initializer::init(4);

    test_image = decodeImage();
    if (test_image.data) {
        sw_target = {};
        sw_target.width = test_image.width;
        sw_target.height = test_image.height;
        sw_target.data = std::shared_ptr<uint8_t>(new uint8_t[test_image.width*test_image.height*4]);

        gl_target.create(test_image.width, test_image.height);
    } else {
        _ERROR("load test image failed!!!!");
    }
}

void ThorvgTestWindow::onClose() {
    test_image = {};
    sw_target = {};
    gl_target.release();

    tvg::Initializer::term();
}

void ThorvgTestWindow::onRenderImgui(ImGuiIO &io, ImVec2 windowSize) {
    static bool useGlCanvas = false;
    ImGui::Checkbox("GlCanvas", &useGlCanvas);
    static bool glNoCache = false;
    static int glMsaaSamples = 0;
    if (useGlCanvas) {
        ImGui::Checkbox("Disable Shader Cache", &glNoCache);
        if (glNoCache) {
            tvg::Initializer::init(4);
        }

        static int glMsaaSamplesIndex = 0;
        static const char *MsaaSamplesList[3] = {
            "0",
            "2",
            "4"
        };
        ImGui::SetNextItemWidth(100);
        if (ImGui::Combo("MSAA Samples", &glMsaaSamplesIndex, MsaaSamplesList, 3)) {
            glMsaaSamples = std::atoi(MsaaSamplesList[glMsaaSamplesIndex]);
        }
    }

    auto startMs = znative::TimeUtils::nowMs();
    tvg::Canvas *canvas = nullptr;
    if (useGlCanvas) {
        tvg::GlCanvas* glcanvas = tvg::GlCanvas::gen();
        glcanvas->target(nullptr, nullptr, nullptr,
            gl_target.id(), gl_target.texWidth(), gl_target.texHeight(), tvg::ColorSpace::ABGR8888S, glMsaaSamples);
        canvas = glcanvas;
    } else {
        tvg::SwCanvas* swcanvas = tvg::SwCanvas::gen();
        swcanvas->target((uint32_t *)sw_target.data.get(), sw_target.width, sw_target.width, sw_target.height, tvg::ColorSpace::ABGR8888);
        canvas = swcanvas;
    }

    tvg::Picture *p = tvg::Picture::gen();
    p->load((uint32_t *)test_image.data.get(), test_image.width, test_image.height, tvg::ColorSpace::ABGR8888);
    canvas->add(p);
    // Generate a shape
    {
        auto rect = tvg::Shape::gen();
        // Append a rounded rectangle to the shape (x, y, w, h, rx, ry)
        rect->appendRect(50, 50, 200, 200, 20, 20);
        // Set the shape's color to (r, g, b)
        rect->fill(100, 100, 100);
        // Add the shape to the canvas
        canvas->add(rect);
    }

    {
        // Generate a shape
        auto path = tvg::Shape::gen();
        // Set the sequential path coordinates
        path->moveTo(199, 34);
        path->lineTo(253, 143);
        path->lineTo(374, 160);
        path->lineTo(287, 244);
        path->lineTo(307, 365);
        path->lineTo(199, 309);
        path->lineTo(97, 365);
        path->lineTo(112, 245);
        path->lineTo(26, 161);
        path->lineTo(146, 143);
        // Complete the path
        path->close();
        // Set the shape's color to (r, g, b)
        path->fill(150, 150, 255);
        // Add the shape to the canvas
        canvas->add(path);
    }
    {
        // Generate a shape
        auto circle = tvg::Shape::gen();
        // Append a circle to the shape (cx, cy, rx, ry)
        circle->appendCircle(400, 400, 100, 100);

        // Generate a radial gradient
        auto fill = tvg::RadialGradient::gen();
        // Set the radial gradient geometry info (cx, cy, radius, fx, fy, fr)
        fill->radial(400, 400, 150, 400, 400, 0);

        // Gradient colors
        tvg::Fill::ColorStop colorStops[2];
        // 1st color values (offset, r, g, b, a)
        colorStops[0] = {0.0, 255, 255, 255, 255};
        // 2nd color values (offset, r, g, b, a)
        colorStops[1] = {1.0, 0, 0, 0, 255};
        // Set the gradient colors info
        fill->colorStops(colorStops, 2);

        // Set the shape fill
        circle->fill(fill);

        // Add the shape to the canvas
        canvas->add(circle);
    }

    {
        // Generate a shape
        auto rect = tvg::Shape::gen();
        // Append a round rectangle to the shape (x, y, w, h, rx, ry)
        rect->appendRect(50, 50, 200, 200, 20, 20);
        // Set the shape's color to (r, g, b)
        rect->fill(100, 100, 100);

        // Set the stroke's width
        rect->strokeWidth(4);
        // Set the stroke's color to (r, g, b)
        rect->strokeFill(50, 50, 50);
        // Set the stroke's join style
        rect->strokeJoin(tvg::StrokeJoin::Round);
        // Set the stroke's cap style
        rect->strokeCap(tvg::StrokeCap::Round);

        // Set the stroke's dash pattern (line, gap)
        float pattern[2] = {7, 10};
        rect->strokeDash(pattern, 2);

        // Add the shape to the canvas
        canvas->add(rect);
    }

    {
        // Generate a Scene
        auto scene = tvg::Scene::gen();

        // Generate a round rectangle
        auto rect = tvg::Shape::gen();
        rect->appendRect(-235, -250, 400, 400, 50, 50);
        rect->fill(0, 255, 0);
        // Add the rectangle to the scene
        scene->add(rect);

        // Generate a circle
        auto circle = tvg::Shape::gen();
        circle->appendCircle(-165, -150, 200, 200);
        // Set the shape's color to (r, g, b, a)

        circle->fill(255, 255, 0, 127);
        // Add the circle to the scene
        scene->add(circle);

        // Generate an ellipse
        auto ellipse = tvg::Shape::gen();
        ellipse->appendCircle(265, 250, 150, 100);
        ellipse->fill(0, 255, 255);
        // Add the eliipse to the scene
        scene->add(ellipse);

        // Transform the scene
        scene->translate(350, 350);
        scene->scale(0.5);
        scene->rotate(45);

        // Add the scene to the canvas
        canvas->add(scene);
    }

    canvas->update();
    canvas->draw(true);
    canvas->sync();

    _INFO("cost ms: %lld", (znative::TimeUtils::nowMs() - startMs));

    delete canvas;
    if (glNoCache) {
        auto res = tvg::Initializer::term();
        _INFO("termrm res: %d", (int) res);
    }


    std::shared_ptr<znative::Texture> texture;
    if (useGlCanvas) {
        texture = gl_target.texture();
    } else {
        texture = sw_texture.update((uint8_t *)sw_target.data.get(), sw_target.width, sw_target.height);
    }

    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
        // window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
        // window_flags |= ImGuiWindowFlags_MenuBar;
        // ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
        ImGui::BeginChild("ChildR", ImVec2(0, 0), ImGuiChildFlags_None, window_flags);
        auto avsize = ImGui::GetContentRegionAvail();
        int tw = texture->width();
        int th = texture->height();
        float displayw, displayh;
        if (avsize.x * th > avsize.y * tw) {
            displayh = avsize.y;
            displayw = displayh * (float) tw / th;
        } else {
            displayw = avsize.x;
            displayh = displayw * (float) th / tw;
        }
        if (useGlCanvas) {
            ImGui::Image(texture->id(), ImVec2(displayw, displayh), ImVec2(0, 1.0f), ImVec2(1.0f, 0));
        } else {
            ImGui::Image(texture->id(), ImVec2(displayw, displayh));
        }
        ImGui::EndChild();
    }
}
