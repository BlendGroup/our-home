const float epsilon = (float)10e-6;

typedef struct _boid_t {
    float4 position;
    float4 velocity;
} boid_t;

float3 cohesion(__global const boid_t *flock, const size_t size, const int this_id, const float radius, const float max_force) {
    float3 force = (float3)(0.0f, 0.0f, 0.0f);
    const boid_t *current = &flock[this_id];
    int count = 0;
    for(int other_id = 0; other_id < size; other_id++) {
        if(other_id == this_id)
            continue;
        const boid_t *other = &flock[other_id];
        float distance = fast_distance(other->position, current->position);
        if((distance > epsilon) && (distance <= radius)) {
            force += other->position.xyz;
            count++;
        }
    }
    if(count > 0)
        force = force / (float)count;
    if(fast_length(force) > max_force)
        force = fast_normalize(force) * max_force;
    return force;
}

float3 alignment(__global const boid_t *flock, const size_t size, const int this_id, const float radius, const float max_force) {
    float3 force = (float3)(0.0f, 0.0f, 0.0f);
    const boid_t *current = &flock[this_id];
    int count = 0;
    for(int other_id = 0; other_id < size; other_id++) {
        if(other_id == this_id)
            continue;
        const boid_t *other = &flock[other_id];
        float distance = fast_distance(other->velocity, current->velocity);
        if((distance > epsilon) && (distance <= radius)) {
            force += other->velocity.xyz;
            count++;
        }
    }
    if(count > 0)
        force = force / (float)count;
    if(fast_length(force) > max_force)
        force = fast_normalize(force) * max_force;
    return force;
}

float3 separation(__global const boid_t *flock, const size_t size, const int this_id, const float radius, const float max_force) {
    float3 force = (float3)(0.0f, 0.0f, 0.0f);
    const boid_t *current = &flock[this_id];
    int count = 0;
    for(int other_id = 0; other_id < size; other_id++) {
        if(other_id == this_id)
            continue;
        const boid_t *other = &flock[other_id];
        float distance = fast_distance(other->position, current->position);
        if((distance > epsilon) && (distance <= radius)) {
            force += fast_normalize(current->position - other->position).xyz / distance;
            count++;
        }
    }
    if(count > 0)
        force = force / (float)count;
    if(fast_length(force) > 0.0f) {
        force = fast_normalize(force) * max_force;
        force -= current->velocity.xyz;
        if(fast_length(force) > max_force)
            force = fast_normalize(force) * max_force;
    }
    return force;
}

float3 attraction(__global const boid_t *flock, const int this_id, const float4 attractor_position, const float max_force) {
    float4 delta = attractor_position - flock[this_id].position;
    float4 force = delta / fast_length(delta);
    if(fast_length(force) > max_force)
        force = fast_normalize(force) * max_force;
    return force.xyz;
}

__kernel void flock_update (
    __global boid_t *const flock_buffer,
    const uint flock_size,
    const float cohesion_radius,
    const float alignment_radius,
    const float separation_radius,
    const float4 attractor_position,
    const float max_distance_from_attractor,
    const float max_speed,
    const float max_force
) {
    int this_id = get_global_id(0);
    if(this_id > flock_size)
        return;

    // calculate updated force on this boid (assume m = 1 => force = acceleration)
    float3 acceleration = (float3)(0.0f, 0.0f, 0.0f);
    acceleration += cohesion(flock_buffer, flock_size, this_id, cohesion_radius, max_force);
    acceleration += alignment(flock_buffer, flock_size, this_id, alignment_radius, max_force);
    acceleration += separation(flock_buffer, flock_size, this_id, separation_radius, max_force);
    acceleration += attraction(flock_buffer, this_id, attractor_position, max_force);

    boid_t *const current = &flock_buffer[this_id];

    // update force on this boid
    current->position += current->velocity;
    current->velocity += (float4)(acceleration, 0.0f);

    // restrict boid distance from attractor
    if(fast_distance(current->position, attractor_position) > max_distance_from_attractor)
        current->velocity = -current->velocity;
    
    // restrict boid speeds
    if(fast_length(current->velocity) > max_speed)
        current->velocity = fast_normalize(current->velocity) * max_speed;
}
