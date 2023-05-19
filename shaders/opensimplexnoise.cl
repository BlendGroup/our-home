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

kernel void fbm2(global float2* input, global float* output, const uint numPoints, const int octaves) {
	int index = get_global_id(0);
	if(index < numPoints) {
		float2 st = input[index];
		st /= 1.5f;
		float value = 0.;
		float amplitude = 1.;
		float freq = 0.8;
		
		for (int i = 0; i < octaves; i++)
		{
			value = max(value, value + (.25f - fabs(snoise2(st * freq) - .3f) * amplitude));
			amplitude *= .37;			
			freq *= 2.05;
			st = st.yx;
		}
		output[index] = value*2.0-2.0;
	}
}

// kernel void turbulencefbm2(global short* perm, global double2* permGrad2, global latticepoint2D_t* lookup2d, global float2* input, global float* output, const uint numPoints, const int octaves) {
// 	int index = get_global_id(0);
// 	if(index < numPoints) {
// 		double2 st = convert_double2(input[index]);
// 		float value = 0.0;
// 		float amplitude = 0.5;

// 		for (int i = 0; i < octaves; i++) {
// 			value += amplitude * noise2_base(perm, permGrad2, lookup2d, st);
// 			st *= 2.0;
// 			amplitude *= 0.5;
// 		}
// 		output[index] = value;
// 	}
// }

// kernel void noise3(global short* perm, global double* permGrad3, global latticepoint3D_t* lookup3d, global float* input, global float* output, const uint numPoints) {
// 	int index = get_global_id(0);
// 	if(index < numPoints) {
// 		double3 xyz = (double3)(input[index * 3 + 0], input[index * 3 + 1], input[index * 3 + 2]);
// 		output[index] = noise3_base(perm, permGrad3, lookup3d, xyz);
// 	}
// }