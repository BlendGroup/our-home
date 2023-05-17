#define PMASK 2047

typedef struct {
	int xsv, ysv;
	double dx, dy;
} latticepoint2D_t;

typedef struct {
	double dxr, dyr, dzr;
	int xrv, yrv, zrv;
	int nextOnFailure;
	int nextOnSuccess;
} latticepoint3D_t;

int2 fast_floor2(double2 s){
	int xi = (int)s.x;
	int yi = (int)s.y;
	xi = s.x < xi ? xi - 1 : xi;
	yi = s.y < yi ? yi - 1 : yi;
	return (int2)(xi, yi);
}

int3 fast_floor3(double3 s){
	int xi = (int)s.x;
	int yi = (int)s.y;
	int zi = (int)s.z;
	xi = s.x < xi ? xi - 1 : xi;
	yi = s.y < yi ? yi - 1 : yi;
	zi = s.z < zi ? zi - 1 : zi;
	return (int3)(xi, yi, zi);
}

double noise2_base(global short* perm, global double2* permGrad2, global latticepoint2D_t* lookup2, double2 xy) {
	double s1 = 0.366025403784439 * (xy.x + xy.y);
	double2 s = xy + s1;
	double value = 0;
	int2 sb = fast_floor2(s);
	double2 si = s - convert_double2(sb);

	int a = (int)(si.x + si.y);
	int index = (a << 2) | (int)(si.x - si.y / 2 + 1 - a / 2.0) << 3 | (int)(si.y - si.x / 2 + 1 - a / 2.0) << 4;

	double ssi = (si.x + si.y) * -0.211324865405187;
	double2 id = si + ssi;

	for (int i = 0; i < 4; i++){
		global latticepoint2D_t *c = &(lookup2[index + i]);

		double dx = id.x + c->dx;
		double dy = id.y + c->dy;
		double attn = 2.0 / 3.0 - dx * dx - dy * dy;
		if (attn <= 0)
			continue;

		int2 pm = (int2)((sb.x + c->xsv) & PMASK, (sb.y + c->ysv) & PMASK);
		double2 grad = permGrad2[perm[pm.x] ^ pm.y];
		double extrapolation = grad.x * dx + grad.y * dy;

		attn *= attn;
		value += attn * attn * extrapolation;
	}

	return value;
}

kernel void noise2(global short* perm, global double2* permGrad2, global latticepoint2D_t* lookup2d, global float2* input, global float* output, const uint numPoints, const float amplitude) {
	int index = get_global_id(0);
	if(index < numPoints) {
		double2 xy = convert_double2(input[index]);
		output[index] = noise2_base(perm, permGrad2, lookup2d, xy) * amplitude;
	}
}

kernel void fbm2(global short* perm, global double2* permGrad2, global latticepoint2D_t* lookup2d, global float2* input, global float* output, const uint numPoints, const int octaves) {
	int index = get_global_id(0);
	if(index < numPoints) {
		double2 st = convert_double2(input[index]);
		float value = 0.0;
		float amplitude = 0.5;

		for (int i = 0; i < octaves; i++) {
			double n = fabs(noise2_base(perm, permGrad2, lookup2d, st));
			value += amplitude * n;
			st *= 2.0;
			amplitude *= 0.5;
		}
		output[index] = 1.0 - value;
	}
}

kernel void turbulencefbm2(global short* perm, global double2* permGrad2, global latticepoint2D_t* lookup2d, global float2* input, global float* output, const uint numPoints, const int octaves) {
	int index = get_global_id(0);
	if(index < numPoints) {
		double2 st = convert_double2(input[index]);
		float value = 0.0;
		float amplitude = 0.5;

		for (int i = 0; i < octaves; i++) {
			value += amplitude * noise2_base(perm, permGrad2, lookup2d, st);
			st *= 2.0;
			amplitude *= 0.5;
		}
		output[index] = value;
	}
}

double noise3_base(global short* perm, global double* permGrad3, global latticepoint3D_t* lookup3, double3 xyz) {
	double r1 = 0.6666666666667 * (xyz.x + xyz.y + xyz.z);
	double3 r = r1 - xyz;
	double value = 0;
	int3 rb = fast_floor3(r);
	double3 ri = r - convert_double3(rb);

	int3 ht = (int3)((int)(ri.x + 0.5), (int)(ri.y + 0.5), (int)(ri.z + 0.5));
	int index = (ht.x << 0) | (ht.y << 1) | (ht.z << 2);

	while(index >= 0) {
		__global latticepoint3D_t *c = &(lookup3[index]);
		double dxr = ri.x + c->dxr;
		double dyr = ri.y + c->dyr;
		double dzr = ri.z + c->dzr;
		double attn = 0.75 - dxr * dxr - dyr * dyr - dzr * dzr;
		if(attn < 0) {
			index = c->nextOnFailure;
		} else {
			int pxm = (rb.x + c->xrv) & PMASK;
			int pym = (rb.y + c->yrv) & PMASK;
			int pzm = (rb.z + c->zrv) & PMASK;
			int permi = perm[perm[pxm] ^ pym] ^ pzm;
			double3 grad = (double3)(permGrad3[permi * 3 + 0], permGrad3[permi * 3 + 1], permGrad3[permi * 3 + 2]);
			double extrapolation = grad.x * dxr + grad.y * dyr + grad.z * dzr;
			attn *= attn;
			value += attn * attn * extrapolation;
			index = c->nextOnSuccess;
		}
	}
	return value;
}

kernel void noise3(global short* perm, global double* permGrad3, global latticepoint3D_t* lookup3d, global float* input, global float* output, const uint numPoints) {
	int index = get_global_id(0);
	if(index < numPoints) {
		double3 xyz = (double3)(input[index * 3 + 0], input[index * 3 + 1], input[index * 3 + 2]);
		output[index] = noise3_base(perm, permGrad3, lookup3d, xyz);
	}
}