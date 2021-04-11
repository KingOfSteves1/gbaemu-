#include "videocontext.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl2.h>
#include <imgui/imgui_impl_sdl.h>
#include <shell/errors.h>
#include <shell/icon.h>

#include "base/bit.h"

VideoContext::~VideoContext()
{
    if (SDL_WasInit(SDL_INIT_VIDEO))
    {
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
}

void VideoContext::init()
{
    if (SDL_InitSubSystem(SDL_INIT_VIDEO))
        throw shell::Error("Cannot init video context: {}", SDL_GetError());

    if (!initWindow())   throw shell::Error("Cannot init window: {}", SDL_GetError());
    if (!initOpenGL())   throw shell::Error("Cannot init OpenGL");
    
    initImgui();
}

void VideoContext::raise()
{
    SDL_RaiseWindow(window);
}

void VideoContext::fullscreen()
{
    SDL_ShowCursor(SDL_ShowCursor(SDL_QUERY) ^ 0x1);
    SDL_SetWindowFullscreen(window, SDL_GetWindowFlags(window) ^ SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void VideoContext::title(const std::string& title)
{
    SDL_SetWindowTitle(window, title.c_str());
}

void VideoContext::renderPresent()
{
    constexpr GLfloat kTextureW     = kScreen.x;
    constexpr GLfloat kTextureH     = kScreen.y;
    constexpr GLfloat kTextureRatio = kTextureW / kTextureH;

    int w;
    int h;
    SDL_GetWindowSize(window, &w, &h);

    GLfloat window_w = w;
    GLfloat window_h = h;
    GLfloat offset_x = 0;
    GLfloat offset_y = 0;

    if (true)
    {
        GLfloat aspect_w = window_w;
        GLfloat aspect_h = window_h;

        if (kTextureRatio > (window_w  / window_h))
            aspect_h = aspect_w / kTextureRatio;
        else
            aspect_w = aspect_h * kTextureRatio;

        offset_x = (window_w - aspect_w) * 0.5f;
        offset_y = (window_h - aspect_h) * 0.5f;
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, main_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureW, kTextureH, 0, GL_BGRA, GL_UNSIGNED_BYTE, buffer.front().data());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-1, -1, 0);
    glScalef(2.0f / window_w, 2.0f / window_h, 1.0);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(           offset_x, window_h - offset_y);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(window_w - offset_x, window_h - offset_y);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(window_w - offset_x,            offset_y);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(           offset_x,            offset_y);
    glEnd();
}

void VideoContext::renderIcon()
{
    int w;
    int h;
    //SDL_RenderGetLogicalSize(renderer, &w, &h);
    //SDL_RenderSetLogicalSize(renderer, 18, 18);

    //for (const auto& pixel : shell::icon::kPixels)
    //{
    //    SDL_SetRenderDrawColor(
    //        renderer,
    //        pixel.r(),
    //        pixel.g(),
    //        pixel.b(),
    //        SDL_ALPHA_OPAQUE);

    //    SDL_RenderDrawPoint(
    //        renderer,
    //        pixel.x() + 1,
    //        pixel.y() + 1);
    //}

    //SDL_RenderSetLogicalSize(renderer, w, h);
}

void VideoContext::swapWindow()
{
    int w;
    int h;
    SDL_GetWindowSize(window, &w, &h);
    glViewport(0, 0, w, h);

    SDL_GL_SwapWindow(window);
}

shell::array<u32, kScreen.x>& VideoContext::scanline(uint line)
{
    return buffer[line];
}

bool VideoContext::initWindow()
{
    window = SDL_CreateWindow(
        "eggvance",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        2 * kScreen.x, 2 * kScreen.y,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (window)
        SDL_SetWindowMinimumSize(window, kScreen.x, kScreen.y);;

    return window;
}

bool VideoContext::initOpenGL()
{
    context = SDL_GL_CreateContext(window);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        return false;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &main_texture);
    glBindTexture(GL_TEXTURE_2D, main_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    SDL_GL_SetSwapInterval(0);

    return true;
}

void VideoContext::initImgui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL2_Init();
}
