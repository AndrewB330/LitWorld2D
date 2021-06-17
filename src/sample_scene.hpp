#pragma once
// WARNING: not the best code here :) Scene initialization, drawing and mouse/key controls

#include <cmath>

#include "application/window_renderer.hpp"
#include "application/window_listener.hpp"
#include "application/time_utils.hpp"

#include "physics/model.hpp"

#include <GL/glew.h>

class Scene : public WindowRenderer, public WindowListener {
public:
    Scene() = default;

    ~Scene() override = default;

    void SpawnInflatedBody(vec2 position, int n, float size, float radius) {
        size *= 0.7f;
        size_t start = world.particles.size();

        std::vector<Particle *> particles;
        for (int i = 0; i < n; i++) {
            float angle = (2.0f * (float) i * (float) M_PI / (float) n);
            vec2 v = vec2(cos(angle), sin(angle)) * size + position;
            world.spawn_particle(v, radius);
            if (i > 0) {
                auto p1 = &world.particles.back();
                auto p2 = &world.particles[i + start - 1];
                world.joints.push_back(Joint{p1, p2, distance(p1->position, p2->position), 6.0f, 0.2f});
            }
            particles.push_back(&world.particles.back());
        }

        {
            auto p1 = &world.particles.back();
            auto p2 = &world.particles[start];
            world.joints.push_back(Joint{p1, p2, distance(p1->position, p2->position), 6.0f, 0.2f});
        }

        world.volumes.push_back(InflatedBody{particles, 4.0, (float) (size * size * M_PI)});
    }

    void SpawnSoftBox(vec2 position, vec2 half_size, float angle = 0.0f, float radius = 0.05f) {
        float gap = radius * 2 * 1.2f;

        int size_x = half_size.x / gap;
        int size_y = half_size.y / gap;

        int start = world.particles.size();

        float stiffness = 4.5;

        for (int i = -size_x; i <= size_x; i++) {
            for (int j = -size_y; j <= size_y; j++) {
                vec2 noise = vec2(sin(j) * 0.01f, cos(j) * 0.01f);
                world.spawn_particle(rotate2d((vec2(i, j) + noise) * gap, angle) + position, radius);

                auto p1 = &world.particles.back();

                if (i > -size_x) {
                    int index = start + (i + size_x - 1) * (2 * size_y + 1) + (j + size_y);
                    auto p2 = &world.particles[index];
                    world.joints.push_back(Joint{p1, p2, gap, stiffness, 0.2f});
                }
                if (j > -size_y) {
                    int index = start + (i + size_x) * (2 * size_y + 1) + (j + size_y - 1);
                    auto p2 = &world.particles[index];
                    world.joints.push_back(Joint{p1, p2, gap, stiffness, 0.2f});
                }
                if (j > -size_y && i > -size_x) {
                    int index = start + (i + size_x - 1) * (2 * size_y + 1) + (j + size_y - 1);
                    auto p2 = &world.particles[index];
                    world.joints.push_back(Joint{p1, p2, gap * sqrtf(2), stiffness, 0.2f});
                }
                if (j < size_y && i > -size_x) {
                    int index = start + (i + size_x - 1) * (2 * size_y + 1) + (j + size_y + 1);
                    auto p2 = &world.particles[index];
                    world.joints.push_back(Joint{p1, p2, gap * sqrtf(2), stiffness, 0.2f});
                }
            }
        }
    }

    bool Init(SDL_Window *window, SDL_GLContext context) override {
        sdl_window = window;
        scale = 100.0f;

        InitWorld();

        timer.reset();
        return true;
    }

    void Redraw() override {
        SDL_GetWindowSize(sdl_window, &width, &height);
        glLoadIdentity();
        glOrtho(-width / 2, width / 2, height / 2, -height / 2, 0, 1);
        glViewport(0, 0, width, height);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        DrawGrid(1, {0.9, 0.9, 0.9});

        for (const auto &j : world.joints) {
            DrawLine(j.p1->position, j.p2->position, {0.25, 0.65, 0.25});
        }

        for (const auto &v : world.volumes) {
            std::vector<vec2> vertices;
            vec2 center = vec2();
            for (auto p: v.particles) {
                vertices.push_back(p->position);
                center += p->position;
            }
            center /= vertices.size();
            DrawPolygon(vertices, center, {0.75, 0.95, 0.75}, true, 0.8f);
        }

        for (const auto &p : world.particles) {
            DrawCircle(p.position, p.radius, {0.25, 0.85, 0.25}, true);
        }

        for (const auto &b : world.boxes) {
            DrawBox(b.position, b.half_size, b.angle, {0.4, 0.4, 0.4}, true);
        }
    }

    void InitWorld() {
        world.spawn_box(vec2(0, 3), vec2(4, 0.2), 0);
        world.spawn_box(vec2(+5, 2.5), vec2(0.2, 0.5), 0.1f);
        world.spawn_box(vec2(-5, 2.5), vec2(0.2, 0.5), -0.1f);

        world.spawn_box(vec2(-2.5, 0), vec2(2, 0.2), 0.1f);
        world.spawn_box(vec2(0, 1), vec2(0.3, 0.2), 0.1f);
        world.spawn_box(vec2(2.5, 0), vec2(1, 0.2), -0.3f);
    }

    bool ProcessEvent(const SDL_Event &event) override {
        static int type = 0;
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            vec2 pos = ScreenToWorld(event.motion.x, event.motion.y);
            static int t = 0; t++; // just a number sequence, to generate some "random" numbers with sin, cos
            if (type == 0) {
                world.spawn_particle(pos, 0.2f);
            }
            if (type == 1) {
                SpawnSoftBox(pos, vec2(sin(t), cos(t)) * 0.3f + 0.5f, cosf((float)t), 0.05f);
            }
            if (type == 2) {
                float size = sinf((float)t) * 0.3f + 0.5f;
                SpawnInflatedBody(pos, (int) (size * 45 + 1), size, 0.05f);
            }
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_1) {
            type = 0;
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_2) {
            type = 1;
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_3) {
            type = 2;
        }
        return false;
    }

    void StartFrameEvent() override {
        auto delta_time = (float) timer.get_elapsed_seconds();
        timer.reset();

        for (int i = 0; i < k_physics_iterations_per_frame; i++) {
            world.update(delta_time / k_physics_iterations_per_frame + 1e-6f);
        }
    }

private:
    using Color = vec3;

    void glVertex(vec2 v) {
        v -= screen_center;
        v *= scale;
        glVertex2f(v.x, v.y);
    }

    vec2 ScreenToWorld(int x, int y) {
        return vec2(x - width / 2, y - height / 2) / scale + screen_center;
    }

    void DrawRectangle(const vec2 &a, const vec2 &b, Color color, bool fill = false) {
        glColor3f(color.r, color.g, color.b);
        if (fill) {
            glBegin(GL_POLYGON);
        } else {
            glBegin(GL_LINE_LOOP);
        }
        glVertex(vec2(a.x, a.y));
        glVertex(vec2(a.x, b.y));
        glVertex(vec2(b.x, b.y));
        glVertex(vec2(b.x, a.y));
        glEnd();
    }

    void DrawBox(const vec2 &pos, const vec2 &half_size, float angle, Color color, bool fill = false) {
        glColor3f(color.r, color.g, color.b);
        if (fill) {
            glBegin(GL_POLYGON);
        } else {
            glBegin(GL_LINE_LOOP);
        }
        glVertex(rotate2d(half_size * vec2(1, 1), angle) + pos);
        glVertex(rotate2d(half_size * vec2(1, -1), angle) + pos);
        glVertex(rotate2d(half_size * vec2(-1, -1), angle) + pos);
        glVertex(rotate2d(half_size * vec2(-1, 1), angle) + pos);
        glEnd();
    }

    void DrawCircle(const vec2 &center, float radius, Color color, bool fill = false) {
        glColor3f(color.r, color.g, color.b);
        if (fill) {
            glBegin(GL_POLYGON);
        } else {
            glBegin(GL_LINE_LOOP);
        }
        for (int i = 0; i < 32; i++) {
            float angle = static_cast<float>((i) * M_PI) * 2.0f / 32;
            glVertex(center + radius * vec2(cosf(angle), sinf(angle)));
        }
        glEnd();
    }

    void
    DrawPolygon(const std::vector<vec2> &vertices, vec2 center, Color color, bool fill = false, float alpha = 1.0f) {
        glColor4f(color.r, color.g, color.b, alpha);
        if (fill) {
            glBegin(GL_TRIANGLE_FAN);
            glVertex(center);
        } else {
            glBegin(GL_LINE_LOOP);
        }
        for (size_t i = 0; i < vertices.size(); i++) {
            glVertex(vertices[i]);
        }
        glVertex(vertices[0]);
        glEnd();
    }

    void DrawLine(const vec2 &a, const vec2 &b, Color color) {
        glColor3f(color.r, color.g, color.b);
        glBegin(GL_LINES);
        glVertex(a);
        glVertex(b);
        glEnd();
    }

    void DrawGrid(float step, Color color) {
        vec2 left_up = ScreenToWorld(0, 0);
        vec2 right_down = ScreenToWorld(width, height);
        for (int i = floor(left_up.x / step); i < ceil(right_down.x / step); i++) {
            DrawLine(vec2(i * step, left_up.y), vec2(i * step, right_down.y), color);
        }
        for (int i = floor(left_up.y / step); i < ceil(right_down.y / step); i++) {
            DrawLine(vec2(left_up.x, i * step), vec2(right_down.x, i * step), color);
        }
    }

    SDL_Window *sdl_window{};
    lit::common::timer timer;
    World world;

    int width = 512;
    int height = 512;

    const int k_physics_iterations_per_frame = 10;

    float scale = 1.0;
    vec2 screen_center = vec2();
};