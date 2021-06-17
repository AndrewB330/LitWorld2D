#pragma once

#include <optional>
#include <vector>
#include <deque>
#include <set>

#include "geometry.hpp"

struct Particle {
    float radius = 0.1f;
    vec2 position = vec2();
    vec2 velocity = vec2();
    vec2 velocity_pseudo = vec2();
    bool alive = true;
};

struct Box {
    vec2 half_size = vec2();
    vec2 position = vec2();
    float angle = 0.0f;
};

struct Collision {
    vec2 normal = vec2();
    float depth = 0.0f;
};

struct Joint {
    Particle *p1 = nullptr;
    Particle *p2 = nullptr;
    float length = 0.0f;
    float stiffness = 0.0f;
    float damping = 0.0f;
};

struct InflatedBody {
    std::vector<Particle *> particles;
    float volume = 1.0f;
    float pressure = 1.0f;
};

struct World {

    void update(float delta_time);

    // Spawn methods
    void spawn_particle(vec2 position, float radius);

    void spawn_box(vec2 position, vec2 half_size, float angle = 0.0f);

    void spawn_joint(Particle *p1, Particle *p2, float stiffness, float damping);

    void spawn_inflated(const std::vector<Particle *> &particles, float pressure);

    // Solve methods
    void solve(Particle *p1, Particle *p2, float delta_time);

    void solve(Box *b, Particle *p, float delta_time);

    void solve(Joint *joint, float delta_time);

    void solve(InflatedBody *volume, float delta_time);

    // Find collision methods
    static std::optional<Collision> find_collision(Particle *p1, Particle *p2);

    static std::optional<Collision> find_collision(Box *b, Particle *p);

    // Members
    std::deque<Particle> particles; // deque - to keep pointers valid even after push_back
    std::vector<Box> boxes;

    std::vector<InflatedBody> volumes;
    std::vector<Joint> joints;

    vec2 gravity = vec2(0, 2.0);

    // Friction does not work properly yet
    float box_friction = 0.0f;

    float box_bounciness = 0.0f;
    float particle_bounciness = 0.0f;

    // Simple data structure to speed up O(N^2) search of particle-particle collisions
    static constexpr int grid_size = 140;
    static constexpr float grid_side = 0.2f;
    std::vector<Particle*> grid[grid_size][grid_size];

    // How much pseudo velocity we will apply when bodies intersect [0; 1]
    float bias_factor = 0.2f;
};
