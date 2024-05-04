#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <iostream>
#include <cstdlib>

constexpr int ScreenWidth = 1200, ScreenHeight = 900;
constexpr int Width = 600, Height = 450;
constexpr float Ratio = static_cast<float>(ScreenWidth) / Width;
constexpr int dx[] = {-1, 0, 1}, dy[] = {-1, 0, 1};
constexpr int PaintSize = 5;
constexpr bool InBound(int x, int y) {
    return x >= 0 && x < Width && y >= 0 && y < Height;
}
constexpr glm::vec3 MatWater[] = {
    {0.f, 0.6f, 1.f},
    {0.f, 0.7f, 1.f},
    {0.f, 0.8f, 1.f}
};
constexpr glm::vec3 MatSand[] = {
    {0.8f, 0.8f, 0.4f},
    {0.9f, 0.9f, 0.5f},
    {1.0f, 1.0f, 0.6f}
};
constexpr glm::vec3 MatWood[] = {
    {0.6f, 0.3f, 0.1f},
    {0.7f, 0.4f, 0.2f},
    {0.8f, 0.5f, 0.3f}
};
constexpr glm::vec3 MatEmpty = {0.f, 0.f, 0.f};
enum class Mat : short { Empty, Water, Sand, Wood };
bool IsMousePressed = false;
float g = 10.f;
float ElapsedTime = 0.f;

struct Particle{
    Mat Material;
    // float Life;
    glm::vec2 Velocity;
    glm::vec3 Color;
    bool Updated;

    Particle(Mat mat) : Material(mat) {
        Velocity = glm::vec2(rand() % 3 - 1, rand() % 8 - 2);
        // Velocity = glm::vec2(0, 0);
        int choice = rand() % 3;
        Updated = false;
        switch(mat)
        {
        case Mat::Water:
            Color = MatWater[choice];
            break;
        case Mat::Sand:
            Color = MatSand[choice];
            break;
        case Mat::Wood:
            Color = MatWood[choice];
            break;
        case Mat::Empty:
            Color = MatEmpty;
            break;
        }
    }
    Particle() : Particle(Mat::Empty) {}
}World[Width * Height];

struct Painter{
    Mat Material = Mat::Empty;
    int x, y;
}Painter;

Particle& GetParticle(int x, int y)
{
    return World[y * Width + x];
}

bool IsMat(int x, int y, Mat mat)
{
    return GetParticle(x, y).Material == mat;
}

bool completely_surrounded(int x, int y)
{
    for(int i = 0; i < 3; ++i)
        for(int j = 0; j < 3; ++j) if(dx[i] != 0 || dy[j] != 0) {
            if(InBound(x + dx[i], y + dy[j]) && IsMat(x + dx[i], y + dy[j], Mat::Empty))
                return false;
        }
    return true;
}

bool IsInLiquid(int x, int y)
{
    for(int i = 0; i < 3; ++i)
        for(int j = 0; j < 3; ++j) if(dx[i] != 0 || dy[j] != 0) {
            if(InBound(x + dx[i], y + dy[j]) && !IsMat(x + dx[i], y + dy[j], Mat::Water))
                return false;
        }
    return true;
}

void UpdateSand(int x, int y)
{
    float dt = 1.f / 60;
    Particle& p = GetParticle(x, y);
    int FallRate = 4;

    p.Velocity.y = std::clamp(p.Velocity.y + g * dt, -10.f, 10.f);
    //p.Updated = true;

    if(InBound(x, y + 1) && !IsMat(x, y + 1, Mat::Empty) && !IsMat(x, y + 1, Mat::Water))
        p.Velocity.y /= 2.f; // ???

    int vx = x + p.Velocity.x, vy = y + p.Velocity.y;
    Particle& np = GetParticle(vx, vy);
    if(
        InBound(vx, vy) && (
            IsMat(vx, vy, Mat::Empty) || (
                IsMat(vx, vy, Mat::Water) &&
                !np.Updated &&
                glm::length(np.Velocity) - glm::length(p.Velocity) > 10.f
            )
        )
    ) {
        if(IsMat(vx, vy, Mat::Water)) {
            for(int i = -10; i < 0; ++i)
                for(int j = -10; j < 10; ++j) {
                    if(InBound(vx + j, vy + i) && IsMat(vx + j, vy + i, Mat::Water)) {
                        std::swap(GetParticle(vx + i, vy + j), np);
                        goto ok;
                    }
                }
        }
ok:
        std::swap(p, np);
    } else if(InBound(x, y + 1) && (IsMat(x, y + 1, Mat::Empty) || IsMat(x, y + 1, Mat::Water))) {
        p.Velocity.y += g * dt;
        std::swap(p, GetParticle(x, y + 1));
    } else {
        for(int i = 0; i < 2; ++i) {
            int nx = x + (i ? 1 : -1), ny = y + 1;
            if(InBound(nx, ny) && (IsMat(nx, ny, Mat::Empty) || IsMat(nx, ny, Mat::Water))) {
                p.Velocity.y += g * dt;
                p.Velocity.x = IsInLiquid(x, y) ? 0 : (rand() % 2 ? 1.f : -1.f);
                // p.Velocity.x = 0;
                std::swap(GetParticle(nx, ny), p);
                return;
            }
        }
    } 
}

void UpdateWater(int x, int y)
{
    float dt = 1.f / 60;
    Particle& p = GetParticle(x, y);
    int FallRate = 2;
    int SpreadRate = 8;

    p.Velocity.y = std::clamp(p.Velocity.y + g * dt, -10.f, 10.f);
    p.Updated = true;

    if(InBound(x, y + 1) && !IsMat(x, y + 1, Mat::Empty))
        p.Velocity.y /= 2.f; // ???

    int ran = rand() % 2;
    int r = ran ? SpreadRate : -SpreadRate, l = -r;
    int u = FallRate, vx = p.Velocity.x, vy = p.Velocity.y;
    int nx[] = {x + vx, x, x + r, x + l};
    int ny[] = {y + vy, y + u, y + u, y + u};

    for(int i = 0; i < 4; ++i)
        if(InBound(nx[i], ny[i]) && IsMat(nx[i], ny[i], Mat::Empty)) {
            std::swap(GetParticle(nx[i], ny[i]), p);
            return;
        }
    if(IsInLiquid(x, y)) {
        int ri = rand() % 3, rj = rand() % 3;
        if(InBound(x + dx[ri], y + dy[rj]) && rand() % 10 == 0)
            std::swap(GetParticle(x + dx[ri], y + dy[rj]), p);
        return;
    }
    if(completely_surrounded(x, y)) return;
    for(int i = 0; i < FallRate; ++i)
        for(int j = SpreadRate; j > 0; --j) {
            if(int nx = x - j, ny = y + i; InBound(nx, ny) && IsMat(nx, ny, Mat::Empty)) {
                std::swap(GetParticle(nx, ny), p);
                return;
            }
            if(int nx = x + j, ny = y + i; InBound(nx, ny) && IsMat(nx, ny, Mat::Empty)) {
                std::swap(GetParticle(nx, ny), p);
                return;
            }
        }
}

void Update(bool even)
{
    ElapsedTime = glfwGetTime();
    for(int j = Height - 1; j >= 0; --j)
        for(int i = even ? 0 : Width - 1; i >= 0 && i < Width; i += even ? 1 : -1) {
            switch(GetParticle(i, j).Material) {
            case Mat::Water:
                UpdateWater(i, j);
                break;
            case Mat::Sand:
                UpdateSand(i, j);
                break;
            case Mat::Wood:
            case Mat::Empty:
                break;
            }
        }
}

static void Render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_POINTS);
    for(int i = 0; i < Width; ++i)
        for(int j = 0; j < Height; ++j) {
            Particle& p = GetParticle(i, j);
            glColor3f(p.Color.r, p.Color.g, p.Color.b);
            glVertex2i(i, j);
        }
    glEnd();
}

void KeyInteract(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS) {
        switch(key)
        {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_F1:
            Painter.Material = Mat::Water;
            break;
        case GLFW_KEY_F2:
            Painter.Material = Mat::Sand;
            break;
        case GLFW_KEY_F3:
            Painter.Material = Mat::Wood;
            break;
        case GLFW_KEY_F4:
            Painter.Material = Mat::Empty;
            break;
        }
    }
}

void MouseInteract(GLFWwindow *window, int button, int action, int mods)
{
    if(action == GLFW_PRESS) IsMousePressed = true;
    else if(action == GLFW_RELEASE) IsMousePressed = false;
}

void ProcessPaint(GLFWwindow *window)
{
    if(!IsMousePressed) return;
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    Painter.x = x / Ratio;
    Painter.y = y / Ratio;
    for(int i = -PaintSize; i <= PaintSize; ++i)
        for(int j = -PaintSize; j <= PaintSize; ++j)
            if(int nx = Painter.x + i, ny = Painter.y + j;
                InBound(nx, ny) && 
                (IsMat(nx, ny, Mat::Empty) || Painter.Material == Mat::Empty))
            {
                GetParticle(nx, ny) = Particle(Painter.Material);
            }
}

int main()
{
    if (!glfwInit()) {
        const char *errmsg;
        glfwGetError(&errmsg);
        if(!errmsg) errmsg = "Unknown error";
        std::cerr << "Error initializing GLFW: " << errmsg << std::endl;
        return 1;
    }
    glfwWindowHint(GLFW_OPENGL_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(ScreenWidth, ScreenHeight, "PixelSim", NULL, NULL);
    if (!window) {
        const char *errmsg;
        glfwGetError(&errmsg);
        if(!errmsg) errmsg = "Unknown error";
        std::cerr << "Error creating window: " << errmsg << std::endl;
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwTerminate();
        std::cerr << "GLAD failed to load GL functions" << std::endl;
        return 1;
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, Width, Height, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPointSize(Ratio);

    glfwSetKeyCallback(window, KeyInteract);
    glfwSetMouseButtonCallback(window, MouseInteract);

    bool even = false;
    srand(time(NULL));
    while (!glfwWindowShouldClose(window))
    {
        ProcessPaint(window);
        Update(even ^= 1);
        Render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}