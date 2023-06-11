float3 mod289_3(float3 x) {
	return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}

float2 mod289_2(float2 x) {
	return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}

float3 permute(float3 x) {
	return mod289_3(((x * 34.0f) + 10.0f) * x);
}

float snoise2(float2 v) {
	const float4 C = (float4)(0.211324865405187f,  // (3.0-sqrt(3.0))/6.0
						0.366025403784439f,  // 0.5*(sqrt(3.0)-1.0)
						-0.577350269189626f,  // -1.0 + 2.0 * C.x
						0.024390243902439f); // 1.0 / 41.0
	float2 i = floor(v + dot(v, C.yy));
	float2 x0 = v - i + dot(i, C.xx);

	float2 i1;
	i1 = (x0.x > x0.y) ? (float2)(1.0, 0.0) : (float2)(0.0, 1.0);
	float4 x12 = x0.xyxy + C.xxzz;
	x12.xy -= i1;

	i = mod289_2(i);
	float3 p = permute(permute(i.y + (float3)(0.0f, i1.y, 1.0f)) + i.x + (float3)(0.0f, i1.x, 1.0f));

	float3 m = max(0.5f - (float3)(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0f);
	m = m * m ;
	m = m * m ;

	float3 in = p * C.www;
	float3 x = 2.0f * min(in - floor(in), 1.0f) - 1.0f;
	float3 h = fabs(x) - 0.5f;
	float3 hx = fabs(x);
	float3 ox = floor(x + 0.5f);
	float3 a0 = x - ox;

	m *= 1.79284291400159f - 0.85373472095314f * (a0 * a0 + h * h);

	float3 g;
	g.x  = a0.x  * x0.x  + h.x  * x0.y;
	g.yz = a0.yz * x12.xz + h.yz * x12.yw;
	float ans = 130.0f * dot(m, g);
	return ans;
}

kernel void noise2(global float2* input, global float* output, const uint numPoints, const float amplitude) {
	int index = get_global_id(0);
	if(index < numPoints) {
		float2 xy = input[index];
		output[index] = snoise2(xy) * amplitude;
	}
}

kernel void fbm2(global float2* input, global float* output, const uint numPoints, const int octaves, const float totalamplitude) {
	int index = get_global_id(0);
	if(index < numPoints) {
		// Initial values
		float2 st = input[index];
		float value = 0.0;
		float amplitude = 0.5f;
		float frequency = 0.0f;
		for (int i = 0; i < octaves; i++) {
			float f = snoise2(st);
			value += amplitude * f;
			st *= 2.0f;
			amplitude *= 0.5f;
		}
		output[index] = value * totalamplitude;
	}
}

kernel void turbulencefbm2(global float2* input, global float* output, const uint numPoints, const int octaves, const float offset) {
	int index = get_global_id(0);
	if(index < numPoints) {
		// Initial values
		float2 st = input[index];
		float value = 0.0;
		float amplitude = 0.5f;
		float frequency = 0.0f;
		for (int i = 0; i < octaves; i++) {
			float f = fabs(snoise2(st));
			f = offset - f;
			f = f * f;
			value += amplitude * f;
			st *= 2.0f;
			amplitude *= 0.5f;
		}
		output[index] = value;
	}
}

kernel void combinetex(read_only image2d_t heightMap1, read_only image2d_t heightMap2, write_only image2d_t heightMapOp, float offset) {
	int2 coord = (int2)(get_global_id(0), get_global_id(1));
	float h1 = read_imagef(heightMap1, coord).r;
	float h2 = read_imagef(heightMap2, coord).r;
	float t = (float)get_global_id(1) / (float)get_image_height(heightMapOp);
	t += offset;
	t = clamp(t, 0.0f, 1.0f);
	write_imagef(heightMapOp, coord, (float4)(mix(h1, h2, t)));
}
