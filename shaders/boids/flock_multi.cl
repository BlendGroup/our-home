/* USE 4 DIFFERENT BUFFERS FOR DIFFERENT FORCES FOR NOW */
const float epsilon = (float)10e-6;

typedef struct _boid_t {
    float3 position;
    float3 velocity;
} boid_t;

// helper
void __attribute__((always_inline)) atomicAddf(volatile __global float *f, const float val) {
    union {
        uint u32;
        float f32;
    } prev, stored, next;
    prev.f32 = *f;
    do {
        next.f32 = (stored.f32 = prev.f32) + val;
        prev.u32 = atomic_cmpxchg((volatile __global uint *)f, stored.u32, next.u32);
    } while(prev.u32 != stored.u32);
}

__kernel void update_cohesion (
    __global boid_t *flock_buffer,
    __global float3 *force_buffer,
    const uint flock_size,
    const float cohesion_radius,
    const float max_force
) {
    size_t this_id = get_global_id(0);
    if(this_id > flock_size)
        return;
    
    float3 force = (float3)(0.0f, 0.0f, 0.0f);
    int count = 0;
    for(int other_id = 0; other_id < flock_size; other_id++) {
        if(other_id == this_id)
            continue;
        const boid_t *other = &flock_buffer[other_id];
        const boid_t *current = &flock_buffer[this_id];
        float distance = fast_distance(other->position, current->position);
        if((distance > epsilon) && (distance <= cohesion_radius)) {
            force += other->position;
            count++;
        }
    }
    if(count > 0)
        force = force / (float)count;
    if(fast_length(force) > max_force)
        force = fast_normalize(force) * max_force;
    
    // atomicAddf(&force_buffer[this_id][0], force[0]);
    // atomicAddf(&force_buffer[this_id][1], force[1]);
    // atomicAddf(&force_buffer[this_id][2], force[2]);
}

__kernel void update_alignment (
    __global boid_t *flock_buffer,
    __global float3 *force_buffer,
    const uint flock_size,
    const float alignment_radius,
    const float max_force
) {
    size_t this_id = get_global_id(0);
    if(this_id > flock_size)
        return;
    
    float3 force = (float3)(0.0f, 0.0f, 0.0f);
    int count = 0;
    for(int other_id = 0; other_id < flock_size; other_id++) {
        if(other_id == this_id)
            continue;
        const boid_t *other = &flock_buffer[other_id];
        const boid_t *current = &flock_buffer[this_id];
        float distance = fast_distance(other->velocity, current->velocity);
        if((distance > epsilon) && (distance <= alignment_radius)) {
            force += other->velocity;
            count++;
        }
    }
    if(count > 0)
        force = force / (float)count;
    if(fast_length(force) > max_force)
        force = fast_normalize(force) * max_force;
    
    // atomicAddf(&force_buffer[this_id][0], force[0]);
    // atomicAddf(&force_buffer[this_id][1], force[1]);
    // atomicAddf(&force_buffer[this_id][2], force[2]);
}

__kernel void update_separation (
    __global boid_t *flock_buffer,
    __global float3 *force_buffer,
    const uint flock_size,
    const float separation_radius,
    const float max_force
) {
    size_t this_id = get_global_id(0);
    if(this_id > flock_size)
        return;
    
    float3 force = (float3)(0.0f, 0.0f, 0.0f);
    int count = 0;
    for(int other_id = 0; other_id < flock_size; other_id++) {
        if(other_id == this_id)
            continue;
        const boid_t *other = &flock_buffer[other_id];
        const boid_t *current = &flock_buffer[this_id];
        float distance = fast_distance(other->position, current->position);
        if((distance > epsilon) && (distance <= separation_radius)) {
            force += fast_normalize(current->position - other->position) / distance;
            count++;
        }
    }
    if(count > 0)
        force = force / (float)count;
    if(fast_length(force) > 0.0f) {
        force = fast_normalize(force) * max_force;
        force -= flock_buffer[this_id].velocity;
        if(fast_length(force) > max_force)
            force = fast_normalize(force) * max_force;
    }
    
    // atomicAddf(&force_buffer[this_id][0], force[0]);
    // atomicAddf(&force_buffer[this_id][1], force[1]);
    // atomicAddf(&force_buffer[this_id][2], force[2]);
}

__kernel void update_attraction (
    __global boid_t *flock_buffer,
    __global float3 *force_buffer,
    const uint flock_size,
    const float3 attractor_position,
    const float max_force
) {
    size_t this_id = get_global_id(0);
    if(this_id > flock_size)
        return;

    float3 delta = attractor_position - flock_buffer[this_id].position;
    float3 force = delta / fast_length(delta);
    if(fast_length(force) > max_force)
        force = fast_normalize(force) * max_force;
    
    // atomicAddf(&force_buffer[this_id][0], force[0]);
    // atomicAddf(&force_buffer[this_id][1], force[1]);
    // atomicAddf(&force_buffer[this_id][2], force[2]);
}

__kernel void flock_update (
    __global boid_t *flock_buffer,
    __global float3 *force_buffer,
    const uint flock_size,
    const float3 attractor_position,
    const float max_distance_from_attractor,
    const float max_speed
) {
    size_t this_id = get_global_id(0);
    if(this_id > flock_size)
        return;
    boid_t *current = &flock_buffer[this_id];
    
    float3 acceleration = force_buffer[this_id];
    current->position += current->velocity;
    current->velocity += acceleration;
    
    if(fast_length(current->position - attractor_position) > max_distance_from_attractor)
        current->velocity = -current->velocity;
    if(fast_length(current->velocity) > max_speed)
        current->velocity = fast_normalize(current->velocity) * max_speed;
}
