#define epsilon 10e-6

typedef struct _boid_t {
	float4 position;
	float4 velocity;
} boid_t;

float3 cohesion(__global const boid_t *flock, const size_t size, const int this_id, const float radius, const float max_force) {
	float3 force = (float3)(0.0f, 0.0f, 0.0f);
	__global const boid_t *current = &flock[this_id];
	int count = 0;
	for(int other_id = 0; other_id < size; other_id++) {
		if(other_id == this_id)
			continue;
		__global const boid_t *other = &flock[other_id];
		float3 dx = other->position.xyz - current->position.xyz;
		if(dot(dx, dx) <= radius) {
			force += -dx;
			count++;
		}
	}
	if(count > 0)
		force = force / (float)count;
	if(length(force) > max_force)
		force = normalize(force) * max_force;
	return force;
}

float3 alignment(__global const boid_t *flock, const size_t size, const int this_id, const float radius, const float max_force) {
	float3 force = (float3)(0.0f, 0.0f, 0.0f);
	__global const boid_t *current = &flock[this_id];
	int count = 0;
	for(int other_id = 0; other_id < size; other_id++) {
		if(other_id == this_id)
			continue;
		__global const boid_t *other = &flock[other_id];
		float3 dv = other->velocity.xyz - current->velocity.xyz;
		float3 dx = other->position.xyz - current->position.xyz;
		if(dot(dx, dx) <= radius) {
			force += dv / (dot(dx, dx) + 10.0f);
			count++;
		}
	}
	if(count > 0)
		force = force / (float)count;
	if(length(force) > max_force)
		force = normalize(force) * max_force;
	return force;
}

float3 separation(__global const boid_t *flock, const size_t size, const int this_id, const float radius, const float max_force) {
	float3 force = (float3)(0.0f, 0.0f, 0.0f);
	__global const boid_t *current = &flock[this_id];
	int count = 0;
	for(int other_id = 0; other_id < size; other_id++) {
		if(other_id == this_id)
			continue;
		__global const boid_t *other = &flock[other_id];
		float3 dx = other->position.xyz - current->position.xyz;
		if(dot(dx, dx) <= radius) {
			force += -dx / dot(dx, dx);
			count++;
		}
	}
	if(count > 0)
		force = force / (float)count;
	force -= current->velocity.xyz;
	if(length(force) > max_force)
		force = normalize(force) * max_force;
	return force;
}

float3 attraction(__global const boid_t *flock, const int this_id, const float3 attractor_position, const float weight, const float max_force) {
	float3 force = weight * (attractor_position - flock[this_id].position.xyz);
	if(length(force) > max_force)
		force = normalize(force) * max_force;
	return force.xyz;
}

__kernel void flock_update (
	__global boid_t *const flock_buffer,
	const uint flock_size,
	const float cohesion_radius,
	const float alignment_radius,
	const float separation_radius,
	const float4 attractor_position,
	const float attraction_weight,
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
	acceleration += attraction(flock_buffer, this_id, attractor_position.xyz, attraction_weight, max_force);

	__global boid_t *const current = &flock_buffer[this_id];

	// update force on this boid
	current->position += current->velocity;
	current->velocity += (float4)(acceleration, 0.0f);
	
	// // restrict boid speeds
	if(length(current->velocity) > max_speed)
		current->velocity = (float4)(normalize(current->velocity.xyz), 0.0) * max_speed;
}
