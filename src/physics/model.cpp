#include <glm/gtx/norm.hpp>
#include "model.hpp"

float get_mass(Particle *p) {
    return p->radius * p->radius; // mass is proportional to radius squared
}

float calculate_volume(const std::vector<Particle *> &particles) {
    float current_volume = 0.0f;
    for (int i = 0; i < particles.size(); i++) {
        int j = (i + 1 == particles.size() ? 0 : i + 1);
        current_volume += cross(vec3(particles[i]->position, 0), vec3(particles[j]->position, 0)).z;
    }
    return current_volume;
}

ivec2 pos_to_grid_pos(vec2 pos) {
    return ivec2(floor(pos / World::grid_side)) + World::grid_size / 2;
}

bool check_grid_pos(ivec2 grid_pos) {
    return all(greaterThanEqual(grid_pos, ivec2(0))) && all(lessThan(grid_pos, ivec2(World::grid_size)));
}

void World::spawn_particle(vec2 position, float radius) {
    particles.push_back(Particle{radius, position});
}

void World::spawn_box(vec2 position, vec2 half_size, float angle) {
    boxes.push_back(Box{half_size, position, angle});
}

void World::spawn_joint(Particle *p1, Particle *p2, float stiffness, float damping) {
    joints.push_back(Joint{p1, p2, distance(p1->position, p2->position), stiffness, damping});
}

void World::spawn_inflated(const std::vector<Particle *> &particles_, float pressure) {
    volumes.push_back(InflatedBody{particles_, calculate_volume(particles_), pressure});
}

void World::update(float delta_time) {
    for (auto &row : grid) {
        for (auto &cell : row) {
            cell.clear();
        }
    }

    for (auto &p : particles) {
        ivec2 grid_pos = pos_to_grid_pos(p.position);
        if (!check_grid_pos(grid_pos)) continue;
        grid[grid_pos.x][grid_pos.y].push_back(&p);
    }

    for (auto &p : particles) {
        if (!p.alive) continue;

        ivec2 grid_pos = pos_to_grid_pos(p.position);
        if (!check_grid_pos(grid_pos)) continue;
        int delta = ceil((2 * p.radius) / grid_side);
        for (int dx = -delta; dx <= delta; dx++) {
            for (int dy = -delta; dy <= delta; dy++) {
                ivec2 next_grid_pos = grid_pos + ivec2(dx, dy);
                if (!check_grid_pos(next_grid_pos)) continue;
                for (auto &p2 : grid[next_grid_pos.x][next_grid_pos.y]) {
                    if (p.radius < p2->radius || p.radius == p2->radius && &p <= p2) {
                        continue;
                    }
                    solve(&p, p2, delta_time);
                }
            }
        }
    }

    // Particle vs Box
    for (auto &p : particles) {
        if (!p.alive) continue;
        for (auto &b : boxes) {
            solve(&b, &p, delta_time);
        }
    }
    // Joints
    for (auto &joint : joints) {
        solve(&joint, delta_time);
    }
    // Inflated bodies
    for (auto &volume : volumes) {
        solve(&volume, delta_time);
    }
    // Integrate
    for (auto &p : particles) {
        p.position += (p.velocity + p.velocity_pseudo) * delta_time;
        p.velocity += gravity * delta_time;
        p.velocity_pseudo = vec3(); // clear pseudo velocity

        if (p.position.y > 8) p.alive = false;
    }
}

void World::solve(Particle *p1, Particle *p2, float delta_time) {
    auto collision = find_collision(p1, p2);

    if (!collision.has_value())
        return;

    p1->velocity_pseudo -= collision->normal * collision->depth / delta_time * bias_factor;
    p2->velocity_pseudo += collision->normal * collision->depth / delta_time * bias_factor;

    float velocity_projected = dot(collision->normal, p1->velocity - p2->velocity);
    float effective_mass = (1.0f / (1.0f / get_mass(p1) + 1.0f / get_mass(p2)));
    float impulse = (1.0f + particle_bounciness) * velocity_projected * effective_mass;

    if (impulse < 0.0f)
        return;

    p1->velocity -= impulse * collision->normal / get_mass(p1);
    p2->velocity += impulse * collision->normal / get_mass(p2);
}

void World::solve(Box *b, Particle *p, float delta_time) {
    auto collision = find_collision(b, p);

    if (!collision.has_value())
        return;

    p->velocity_pseudo += collision->normal * collision->depth / delta_time * bias_factor;

    if (dot(p->velocity, collision->normal) > 0.0f)
        return;

    vec2 tangent = tangent2d(collision->normal);

    p->velocity -= (1.0f + box_bounciness) * dot(p->velocity, collision->normal) * collision->normal;
    p->velocity -= box_friction * dot(p->velocity, tangent) * tangent;
}

void World::solve(Joint *joint, float delta_time) {
    auto p1 = joint->p1, p2 = joint->p2;

    float current_length = length(p1->position - p2->position);
    vec2 direction = normalize(p2->position - p1->position);

    float length_difference = clamp(joint->length - current_length, -0.8f, 0.8f);
    float velocity_projection = clamp(dot(direction, p1->velocity - p2->velocity), -0.8f, 0.8f);

    float force = length_difference * joint->stiffness + velocity_projection * joint->damping;

    p1->velocity -= force * delta_time * direction / get_mass(p1);
    p2->velocity += force * delta_time * direction / get_mass(p2);
}

void World::solve(InflatedBody *v, float delta_time) {
    vec2 total_velocity = vec2();
    for (auto p: v->particles) total_velocity += p->velocity;
    total_velocity /= v->particles.size();

    float current_volume = calculate_volume(v->particles);
    float current_pressure = v->pressure * (v->volume / current_volume); // pressure * volume ~= const
    float pressure_difference = current_pressure - 1.0f; // minus one atmosphere

    for (size_t i = 0; i < v->particles.size(); i++) {
        auto p1 = v->particles[i];
        auto p2 = v->particles[(i + 1) % v->particles.size()];

        vec2 normal = tangent2d(normalize(p1->position - p2->position));
        float velocity_projected = dot(normal, (p1->velocity + p2->velocity) * 0.5f - total_velocity);

        float current_length = length(p1->position - p2->position);
        float force = current_length * pressure_difference - velocity_projected * 0.005f;

        p1->velocity += force * delta_time * normal / get_mass(p1);
        p2->velocity += force * delta_time * normal / get_mass(p2);
    }
}

std::optional<Collision> World::find_collision(Particle *p1, Particle *p2) {
    float dist_sqr = distance2(p1->position, p2->position);
    float sum_radius = p1->radius + p2->radius;

    // fast check
    if (dist_sqr > sum_radius * sum_radius || dist_sqr < 0.00001f)
        return std::nullopt;

    float dist = sqrtf(dist_sqr);

    Collision collision;
    collision.depth = sum_radius - dist;
    collision.normal = (p2->position - p1->position) / dist;
    return collision;
}

std::optional<Collision> World::find_collision(Box *b, Particle *p) {
    float dist_sqr = distance2(b->position, p->position);
    float radius_sum = length(b->half_size) + p->radius;

    // fast check
    if (dist_sqr > radius_sum * radius_sum)
        return std::nullopt;

    vec2 particle_in_box_space = rotate2d(p->position - b->position, -b->angle);
    vec2 nearest_in_box_space = glm::clamp(particle_in_box_space, -b->half_size, b->half_size);

    float dist = distance(particle_in_box_space, nearest_in_box_space);
    if (dist > p->radius)
        return std::nullopt;

    vec2 nearest = rotate2d(nearest_in_box_space, b->angle) + b->position;

    Collision collision;
    collision.depth = p->radius - dist;
    collision.normal = (p->position - nearest) / dist;
    return collision;
}
