// #include<opensimplexnoise.h>
// #include<vmath.h>
// #include<clhelper.h>
// #include<iostream>
// #include<vector>
// #include<global.h>
// #include<unordered_map>
// #include<errorlog.h>

// #define PSIZE 2048
// #define NUM_LATTICE 8
// #define NUM_POINTS_PER_2D_LATICE 4
// #define NUM_POINTS_PER_3D_LATICE 14

// using namespace vmath;
// using namespace std;

// #define PROGRAM_NAME "opensimplexnoise.cl"
// #define ERROR_INVALID_NOISE "Invalid Noise Type"

// unordered_map<noisetype, string> noisekernelnamelookup = {
// 	{Noise2D, "noise2"},
// 	{Noise3D, "noise3"}
// };

// unordered_map<noisetype, string> fbmkernelnamelookup = {
// 	{FBM2D, "fbm2"},
// 	{FBM3D, "fbm3"}
// };

// typedef struct latticepoint2D_t {
// 	int xsv, ysv;
// 	double dx, dy;
// } latticepoint2D_t;

// latticepoint2D_t latticepoint2D(int xsv, int ysv) {
// 	latticepoint2D_t lp2D;
// 	lp2D.xsv = xsv;
// 	lp2D.ysv = ysv;
// 	double ssv = (xsv + ysv) * -0.211324865405187;
// 	lp2D.dx = -xsv - ssv;
// 	lp2D.dy = -ysv - ssv;
// 	return lp2D;
// }

// typedef struct latticepoint3D_t {
// 	double dxr, dyr, dzr;
// 	int xrv, yrv, zrv;
// 	int nextOnFailure;
// 	int nextOnSuccess;
// } latticepoint3D_t;

// latticepoint3D_t latticepoint3D(int xrv, int yrv, int zrv, int lattice) {
// 	latticepoint3D_t lp3D;
// 	lp3D.dxr = -xrv + lattice * 0.5;
// 	lp3D.dyr = -yrv + lattice * 0.5;
// 	lp3D.dzr = -zrv + lattice * 0.5;
// 	lp3D.xrv = xrv + lattice * 1024;
// 	lp3D.yrv = yrv + lattice * 1024;
// 	lp3D.zrv = zrv + lattice * 1024;
// 	return lp3D;
// }

// opensimplexnoise::opensimplexnoise() {
// 	try {
// 		string programName;
// 		programglobal::oclContext->compilePrograms({"shaders/opensimplexnoise.cl"});
// 	} catch(string errString) {
// 		throwErr(errString);
// 	}
// }

// cl_mem create2DUniformInput(ivec2 dim, ivec2 offset, float timeInterval) {
// 	cl_mem inputGrid;
// 	const float freq = 1.0f / timeInterval; 
// 	size_t num_points = dim[0] * dim[1];
// 	vec2 *grid = (vec2*)malloc(num_points * sizeof(vec2));
// 	int i = 0;
// 	for (int y = 0; y < dim[1]; y++){
// 		for (int x = 0; x < dim[0]; x++) {
// 			grid[i++] = vec2((x + offset[0]) * freq, (y + offset[1]) * freq);
// 		}
// 	}
// 	CLErr(inputGrid = clCreateBuffer(programglobal::oclContext->getContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(vec2) * num_points, grid, &clhelpererr));
// 	return inputGrid;
// }

// cl_mem create3DUniformInput(ivec3 dim, ivec3 offset, float timeInterval) {
// 	cl_mem inputGrid;
// 	const float freq = 1.0f / timeInterval; 
// 	size_t num_points = dim[0] * dim[1] * dim[2];
// 	vec3 *grid = (vec3*)malloc(num_points * sizeof(vec3));
// 	int i = 0;
// 	for(int z = 0; z < dim[2]; z++) {
// 		for (int y = 0; y < dim[1]; y++){
// 			for (int x = 0; x < dim[0]; x++){
// 				grid[i++] = vec3((x + offset[0]) * freq, (y + offset[1]) * freq, (z + offset[2]) * freq);
// 			}
// 		}
// 	}
// 	CLErr(inputGrid = clCreateBuffer(programglobal::oclContext->getContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(vec3) * num_points, grid, &clhelpererr));
// 	return inputGrid;
// }

// cl_mem createUniformInput(noisetype type, const int* dim, const int* offset, float timeInterval) {
// 	if(type == Noise2D || type == FBM2D) {
// 		return create2DUniformInput(ivec2(dim[0], dim[1]), ivec2(offset[0], offset[1]), timeInterval);
// 	} else if(type == Noise3D) {
// 		return create3DUniformInput(ivec3(dim[0], dim[1], dim[2]), ivec3(offset[0], offset[1], offset[2]), timeInterval);
// 	} else {
// 		throwErr(ERROR_INVALID_NOISE);
// 		return NULL;
// 	}
// }

// void create2DNoiseTexture(cl_kernel kernel, cl_mem inputGrid, clglmem outputNoise, ivec2 dim, float amplitude, long seed) {
// 	try {
// 		static dvec2 gradients2d[PSIZE];
// 		static latticepoint2D_t lookup2d[NUM_LATTICE * NUM_POINTS_PER_2D_LATICE];
// 		static short perm[PSIZE];
// 		static dvec2 permGrad2d[PSIZE];

// 		//Fill Gradients Array -- TODO find out why this array and N2
// 		const double N2 = 0.05481866495625118;

// 		static const dvec2 arr[24] = {
// 			dvec2(0.130526192220052, 0.99144486137381) / N2,
// 			dvec2(0.38268343236509, 0.923879532511287) / N2,
// 			dvec2(0.608761429008721, 0.793353340291235) / N2,
// 			dvec2(0.793353340291235, 0.608761429008721) / N2,
// 			dvec2(0.923879532511287, 0.38268343236509) / N2,
// 			dvec2(0.99144486137381, 0.130526192220051) / N2,
// 			dvec2(0.99144486137381, -0.130526192220051) / N2,
// 			dvec2(0.923879532511287, -0.38268343236509) / N2,
// 			dvec2(0.793353340291235, -0.60876142900872) / N2,
// 			dvec2(0.608761429008721, -0.793353340291235) / N2,
// 			dvec2(0.38268343236509, -0.923879532511287) / N2,
// 			dvec2(0.130526192220052, -0.99144486137381) / N2,
// 			dvec2(-0.130526192220052, -0.99144486137381) / N2,
// 			dvec2(-0.38268343236509, -0.923879532511287) / N2,
// 			dvec2(-0.608761429008721, -0.793353340291235) / N2,
// 			dvec2(-0.793353340291235, -0.608761429008721) / N2,
// 			dvec2(-0.923879532511287, -0.38268343236509) / N2,
// 			dvec2(-0.99144486137381, -0.130526192220052) / N2,
// 			dvec2(-0.99144486137381, 0.130526192220051) / N2,
// 			dvec2(-0.923879532511287, 0.38268343236509) / N2,
// 			dvec2(-0.793353340291235, 0.608761429008721) / N2,
// 			dvec2(-0.608761429008721, 0.793353340291235) / N2,
// 			dvec2(-0.38268343236509, 0.923879532511287) / N2,
// 			dvec2(-0.130526192220052, 0.99144486137381) / N2
// 		};
// 		for(int i = 0; i < PSIZE; i++) {
// 			gradients2d[i] = arr[i % 24];
// 		}

// 		//Fill Lookup Array -- TODO find out why

// 		for(int i = 0; i < NUM_LATTICE; i++) {
// 			int i1, i2, j1, j2;
// 			if((i & 1) == 0) {//Once every 2 Times
// 				if((i & 2) == 0) { //Twice every 4 times
// 					i1 = -1;
// 					j1 = 0;
// 				} else { //Twice every 4 times
// 					i1 = 1;
// 					j1 = 0;
// 				}
// 				if((i & 4) == 0) { //Quadrice every 8 times
// 					i2 = 0;
// 					j2 = -1;
// 				} else { //Quadrice every 8 times
// 					i2 = 0;
// 					j2 = 1;
// 				}
// 			} else {//Once every 2 Times
// 				if((i & 2) == 0) { //Twice every 4 times
// 					i1 = 0;
// 					j1 = 1;
// 				} else { //Twice every 4 times
// 					i1 = 2;
// 					j1 = 1;
// 				}
// 				if((i & 4) == 0) { //Quadrice every 8 times
// 					i2 = 1;
// 					j2 = 0;
// 				} else { //Quadrice every 8 times
// 					i2 = 1;
// 					j2 = 2;
// 				}
// 			}
// 			lookup2d[i * NUM_POINTS_PER_2D_LATICE + 0] = latticepoint2D(0, 0);
// 			lookup2d[i * NUM_POINTS_PER_2D_LATICE + 1] = latticepoint2D(1, 1);
// 			lookup2d[i * NUM_POINTS_PER_2D_LATICE + 2] = latticepoint2D(i1, j1);
// 			lookup2d[i * NUM_POINTS_PER_2D_LATICE + 3] = latticepoint2D(i2, j2);
// 		}
		
// 		//Fill Perm Array -- TODO find why

// 		short source[PSIZE];
// 		for(int i = 0; i < PSIZE; i++) {
// 			source[i] = i;
// 		}
// 		for(int i = PSIZE - 1; i >= 0; i--) {
// 			//Whyyyy
// 			seed = seed * 6364136223846793005L + 1442695040888963407L;
// 			int r = (int)((seed + 31) % (i + 1));
// 			if (r < 0) {
// 				r += (i + 1);
// 			}
// 			perm[i] = source[r];
// 			permGrad2d[i] = gradients2d[perm[i]];
// 			source[r] = source[i];
// 		}

// 		CLErr(cl_mem clPerm = clCreateBuffer(programglobal::oclContext->getContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(perm), perm, &clhelpererr));
// 		CLErr(cl_mem clPermGrad2d = clCreateBuffer(programglobal::oclContext->getContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(permGrad2d), permGrad2d, &clhelpererr));
// 		CLErr(cl_mem clLookup2d = clCreateBuffer(programglobal::oclContext->getContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(lookup2d), lookup2d, &clhelpererr));
// 		cl_uint pointCount = dim[0] * dim[1];
// 		size_t globalWorkSize = pointCount;

// 		programglobal::oclContext->setKernelParameters(kernel, { param(0, clPerm), param(1, clPermGrad2d), param(2, clLookup2d), param(3, inputGrid), param(4, outputNoise.cl), param(5, pointCount), param(6, amplitude) });
// 		programglobal::oclContext->runCLKernel(kernel, 1, &globalWorkSize, NULL, { outputNoise });

// 		clReleaseMemObject(clPerm);
// 		clReleaseMemObject(clPermGrad2d);
// 		clReleaseMemObject(clLookup2d);
// 	} catch(string errorString) {
// 		throwErr(errorString);
// 	}
// }

// void create2DFBMTexture(cl_kernel kernel, cl_mem inputGrid, clglmem outputNoise, ivec2 dim, int octaves, long seed) {
// 	try {
// 		static dvec2 gradients2d[PSIZE];
// 		static latticepoint2D_t lookup2d[NUM_LATTICE * NUM_POINTS_PER_2D_LATICE];
// 		static short perm[PSIZE];
// 		static dvec2 permGrad2d[PSIZE];

// 		//Fill Gradients Array -- TODO find out why this array and N2
// 		const double N2 = 0.05481866495625118;

// 		static const dvec2 arr[24] = {
// 			dvec2(0.130526192220052, 0.99144486137381) / N2,
// 			dvec2(0.38268343236509, 0.923879532511287) / N2,
// 			dvec2(0.608761429008721, 0.793353340291235) / N2,
// 			dvec2(0.793353340291235, 0.608761429008721) / N2,
// 			dvec2(0.923879532511287, 0.38268343236509) / N2,
// 			dvec2(0.99144486137381, 0.130526192220051) / N2,
// 			dvec2(0.99144486137381, -0.130526192220051) / N2,
// 			dvec2(0.923879532511287, -0.38268343236509) / N2,
// 			dvec2(0.793353340291235, -0.60876142900872) / N2,
// 			dvec2(0.608761429008721, -0.793353340291235) / N2,
// 			dvec2(0.38268343236509, -0.923879532511287) / N2,
// 			dvec2(0.130526192220052, -0.99144486137381) / N2,
// 			dvec2(-0.130526192220052, -0.99144486137381) / N2,
// 			dvec2(-0.38268343236509, -0.923879532511287) / N2,
// 			dvec2(-0.608761429008721, -0.793353340291235) / N2,
// 			dvec2(-0.793353340291235, -0.608761429008721) / N2,
// 			dvec2(-0.923879532511287, -0.38268343236509) / N2,
// 			dvec2(-0.99144486137381, -0.130526192220052) / N2,
// 			dvec2(-0.99144486137381, 0.130526192220051) / N2,
// 			dvec2(-0.923879532511287, 0.38268343236509) / N2,
// 			dvec2(-0.793353340291235, 0.608761429008721) / N2,
// 			dvec2(-0.608761429008721, 0.793353340291235) / N2,
// 			dvec2(-0.38268343236509, 0.923879532511287) / N2,
// 			dvec2(-0.130526192220052, 0.99144486137381) / N2
// 		};
// 		for(int i = 0; i < PSIZE; i++) {
// 			gradients2d[i] = arr[i % 24];
// 		}

// 		//Fill Lookup Array -- TODO find out why

// 		for(int i = 0; i < NUM_LATTICE; i++) {
// 			int i1, i2, j1, j2;
// 			if((i & 1) == 0) {//Once every 2 Times
// 				if((i & 2) == 0) { //Twice every 4 times
// 					i1 = -1;
// 					j1 = 0;
// 				} else { //Twice every 4 times
// 					i1 = 1;
// 					j1 = 0;
// 				}
// 				if((i & 4) == 0) { //Quadrice every 8 times
// 					i2 = 0;
// 					j2 = -1;
// 				} else { //Quadrice every 8 times
// 					i2 = 0;
// 					j2 = 1;
// 				}
// 			} else {//Once every 2 Times
// 				if((i & 2) == 0) { //Twice every 4 times
// 					i1 = 0;
// 					j1 = 1;
// 				} else { //Twice every 4 times
// 					i1 = 2;
// 					j1 = 1;
// 				}
// 				if((i & 4) == 0) { //Quadrice every 8 times
// 					i2 = 1;
// 					j2 = 0;
// 				} else { //Quadrice every 8 times
// 					i2 = 1;
// 					j2 = 2;
// 				}
// 			}
// 			lookup2d[i * NUM_POINTS_PER_2D_LATICE + 0] = latticepoint2D(0, 0);
// 			lookup2d[i * NUM_POINTS_PER_2D_LATICE + 1] = latticepoint2D(1, 1);
// 			lookup2d[i * NUM_POINTS_PER_2D_LATICE + 2] = latticepoint2D(i1, j1);
// 			lookup2d[i * NUM_POINTS_PER_2D_LATICE + 3] = latticepoint2D(i2, j2);
// 		}
		
// 		//Fill Perm Array -- TODO find why

// 		short source[PSIZE];
// 		for(int i = 0; i < PSIZE; i++) {
// 			source[i] = i;
// 		}
// 		for(int i = PSIZE - 1; i >= 0; i--) {
// 			//Whyyyy
// 			seed = seed * 6364136223846793005L + 1442695040888963407L;
// 			int r = (int)((seed + 31) % (i + 1));
// 			if (r < 0) {
// 				r += (i + 1);
// 			}
// 			perm[i] = source[r];
// 			permGrad2d[i] = gradients2d[perm[i]];
// 			source[r] = source[i];
// 		}

// 		CLErr(cl_mem clPerm = clCreateBuffer(programglobal::oclContext->getContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(perm), perm, &clhelpererr));
// 		CLErr(cl_mem clPermGrad2d = clCreateBuffer(programglobal::oclContext->getContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(permGrad2d), permGrad2d, &clhelpererr));
// 		CLErr(cl_mem clLookup2d = clCreateBuffer(programglobal::oclContext->getContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(lookup2d), lookup2d, &clhelpererr));
// 		cl_uint pointCount = dim[0] * dim[1];
// 		size_t globalWorkSize = pointCount;

// 		programglobal::oclContext->setKernelParameters(kernel, { param(0, clPerm), param(1, clPermGrad2d), param(2, clLookup2d), param(3, inputGrid), param(4, outputNoise.cl), param(5, pointCount), param(6, octaves) });
// 		programglobal::oclContext->runCLKernel(kernel, 1, &globalWorkSize, NULL, { outputNoise });

// 		clReleaseMemObject(clPerm);
// 		clReleaseMemObject(clPermGrad2d);
// 		clReleaseMemObject(clLookup2d);
// 	} catch(string errorString) {
// 		throwErr(errorString);
// 	}
// }

// void create3DNoiseTexture(cl_kernel kernel, cl_mem inputGrid, clglmem &outputNoise, ivec3 dim, float amplitude, long seed) {
// 	try {
// 		static dvec3 gradients3d[PSIZE];
// 		static latticepoint3D_t lookup3d[NUM_LATTICE * NUM_POINTS_PER_3D_LATICE];
// 		static short perm[PSIZE];
// 		static dvec3 permGrad3d[PSIZE];

// 		//Fill Gradients Array -- TODO find out why this array and N2

// 		const double N3 = 0.2781926117527186;

// 		static const dvec3 arr[] = {
// 			dvec3(-2.22474487139, -2.22474487139, -1.0) / N3,
// 			dvec3(-2.22474487139, -2.22474487139, 1.0) / N3,
// 			dvec3(-3.0862664687972017, -1.1721513422464978, 0.0) / N3,
// 			dvec3(-1.1721513422464978, -3.0862664687972017, 0.0) / N3,
// 			dvec3(-2.22474487139, -1.0, -2.22474487139) / N3,
// 			dvec3(-2.22474487139, 1.0, -2.22474487139) / N3,
// 			dvec3(-1.1721513422464978, 0.0, -3.0862664687972017) / N3,
// 			dvec3(-3.0862664687972017, 0.0, -1.1721513422464978) / N3,
// 			dvec3(-2.22474487139, -1.0, 2.22474487139) / N3,
// 			dvec3(-2.22474487139, 1.0, 2.22474487139) / N3,
// 			dvec3(-3.0862664687972017, 0.0, 1.1721513422464978) / N3,
// 			dvec3(-1.1721513422464978, 0.0, 3.0862664687972017) / N3,
// 			dvec3(-2.22474487139, 2.22474487139, -1.0) / N3,
// 			dvec3(-2.22474487139, 2.22474487139, 1.0) / N3,
// 			dvec3(-1.1721513422464978, 3.0862664687972017, 0.0) / N3,
// 			dvec3(-3.0862664687972017, 1.1721513422464978, 0.0) / N3,
// 			dvec3(-1.0, -2.22474487139, -2.22474487139) / N3,
// 			dvec3(1.0, -2.22474487139, -2.22474487139) / N3,
// 			dvec3(0.0, -3.0862664687972017, -1.1721513422464978) / N3,
// 			dvec3(0.0, -1.1721513422464978, -3.0862664687972017) / N3,
// 			dvec3(-1.0, -2.22474487139, 2.22474487139) / N3,
// 			dvec3(1.0, -2.22474487139, 2.22474487139) / N3,
// 			dvec3(0.0, -1.1721513422464978, 3.0862664687972017) / N3,
// 			dvec3(0.0, -3.0862664687972017, 1.1721513422464978) / N3,
// 			dvec3(-1.0, 2.22474487139, -2.22474487139) / N3,
// 			dvec3(1.0, 2.22474487139, -2.22474487139) / N3,
// 			dvec3(0.0, 1.1721513422464978, -3.0862664687972017) / N3,
// 			dvec3(0.0, 3.0862664687972017, -1.1721513422464978) / N3,
// 			dvec3(-1.0, 2.22474487139, 2.22474487139) / N3,
// 			dvec3(1.0, 2.22474487139, 2.22474487139) / N3,
// 			dvec3(0.0, 3.0862664687972017, 1.1721513422464978) / N3,
// 			dvec3(0.0, 1.1721513422464978, 3.0862664687972017) / N3,
// 			dvec3(2.22474487139, -2.22474487139, -1.0) / N3,
// 			dvec3(2.22474487139, -2.22474487139, 1.0) / N3,
// 			dvec3(1.1721513422464978, -3.0862664687972017, 0.0) / N3,
// 			dvec3(3.0862664687972017, -1.1721513422464978, 0.0) / N3,
// 			dvec3(2.22474487139, -1.0, -2.22474487139) / N3,
// 			dvec3(2.22474487139, 1.0, -2.22474487139) / N3,
// 			dvec3(3.0862664687972017, 0.0, -1.1721513422464978) / N3,
// 			dvec3(1.1721513422464978, 0.0, -3.0862664687972017) / N3,
// 			dvec3(2.22474487139, -1.0, 2.22474487139) / N3,
// 			dvec3(2.22474487139, 1.0, 2.22474487139) / N3,
// 			dvec3(1.1721513422464978, 0.0, 3.0862664687972017) / N3,
// 			dvec3(3.0862664687972017, 0.0, 1.1721513422464978) / N3,
// 			dvec3(2.22474487139, 2.22474487139, -1.0) / N3,
// 			dvec3(2.22474487139, 2.22474487139, 1.0) / N3,
// 			dvec3(3.0862664687972017, 1.1721513422464978, 0.0) / N3,
// 			dvec3(1.1721513422464978, 3.0862664687972017, 0.0) / N3
// 		};

// 		for (int i = 0; i < PSIZE; i++){
// 			gradients3d[i] = arr[i % 48];
// 		}

// 		//Fill Lookup Array -- TODO find out why

// 		int j = 7;
// 		for (int i = 0; i < NUM_LATTICE; i++){
// 			int i1, j1, k1, i2, j2, k2;
// 			i1 = (i >> 0) & 1;
// 			j1 = (i >> 1) & 1;
// 			k1 = (i >> 2) & 1;
// 			i2 = i1 ^ 1;
// 			j2 = j1 ^ 1;
// 			k2 = k1 ^ 1;

// 			// The two points within this octant, one from each of the two cubic half-lattices.
// 			latticepoint3D_t c0 = latticepoint3D(i1, j1, k1, 0);
// 			latticepoint3D_t c1 = latticepoint3D(i1 + i2, j1 + j2, k1 + k2, 1);

// 			// (1, 0, 0) vs (0, 1, 1) away from octant.
// 			latticepoint3D_t c2 = latticepoint3D(i1 ^ 1, j1, k1, 0);
// 			latticepoint3D_t c3 = latticepoint3D(i1, j1 ^ 1, k1 ^ 1, 0);

// 			// (1, 0, 0) vs (0, 1, 1) away from octant, on second half-lattice.
// 			latticepoint3D_t c4 = latticepoint3D(i1 + (i2 ^ 1), j1 + j2, k1 + k2, 1);
// 			latticepoint3D_t c5 = latticepoint3D(i1 + i2, j1 + (j2 ^ 1), k1 + (k2 ^ 1), 1);

// 			// (0, 1, 0) vs (1, 0, 1) away from octant.
// 			latticepoint3D_t c6 = latticepoint3D(i1, j1 ^ 1, k1, 0);
// 			latticepoint3D_t c7 = latticepoint3D(i1 ^ 1, j1, k1 ^ 1, 0);

// 			// (0, 1, 0) vs (1, 0, 1) away from octant, on second half-lattice.
// 			latticepoint3D_t c8 = latticepoint3D(i1 + i2, j1 + (j2 ^ 1), k1 + k2, 1);
// 			latticepoint3D_t c9 = latticepoint3D(i1 + (i2 ^ 1), j1 + j2, k1 + (k2 ^ 1), 1);

// 			// (0, 0, 1) vs (1, 1, 0) away from octant.
// 			latticepoint3D_t cA = latticepoint3D(i1, j1, k1 ^ 1, 0);
// 			latticepoint3D_t cB = latticepoint3D(i1 ^ 1, j1 ^ 1, k1, 0);

// 			// (0, 0, 1) vs (1, 1, 0) away from octant, on second half-lattice.
// 			latticepoint3D_t cC = latticepoint3D(i1 + i2, j1 + j2, k1 + (k2 ^ 1), 1);
// 			latticepoint3D_t cD = latticepoint3D(i1 + (i2 ^ 1), j1 + (j2 ^ 1), k1 + k2, 1);

// 			// First two points are guaranteed.
// 			c0.nextOnFailure = c0.nextOnSuccess = j + 1;
// 			c1.nextOnFailure = c1.nextOnSuccess = j + 2;

// 			// If c2 is in range, then we know c3 and c4 are not.
// 			c2.nextOnFailure = j + 3;
// 			c2.nextOnSuccess = j + 5;
// 			c3.nextOnFailure = j + 4;
// 			c3.nextOnSuccess = j + 4;

// 			// If c4 is in range, then we know c5 is not.
// 			c4.nextOnFailure = j + 5;
// 			c4.nextOnSuccess = j + 6;
// 			c5.nextOnFailure = c5.nextOnSuccess = j + 6;

// 			// If c6 is in range, then we know c7 and c8 are not.
// 			c6.nextOnFailure = j + 7;
// 			c6.nextOnSuccess = j + 9;
// 			c7.nextOnFailure = j + 8;
// 			c7.nextOnSuccess = j + 8;

// 			// If c8 is in range, then we know c9 is not.
// 			c8.nextOnFailure = j + 9;
// 			c8.nextOnSuccess = j + 10;
// 			c9.nextOnFailure = c9.nextOnSuccess = j + 10;

// 			// If cA is in range, then we know cB and cC are not.
// 			cA.nextOnFailure = j + 11;
// 			cA.nextOnSuccess = j + 13;
// 			cB.nextOnFailure = j + 12;
// 			cB.nextOnSuccess = j + 12;

// 			// If cC is in range, then we know cD is not.
// 			cC.nextOnFailure = j + 13;
// 			cC.nextOnSuccess = -1;
// 			cD.nextOnFailure = cD.nextOnSuccess = -1;

// 			lookup3d[i] = c0;
// 			lookup3d[++j] = c1;
// 			lookup3d[++j] = c2;
// 			lookup3d[++j] = c3;
// 			lookup3d[++j] = c4;
// 			lookup3d[++j] = c5;
// 			lookup3d[++j] = c6;
// 			lookup3d[++j] = c7;
// 			lookup3d[++j] = c8;
// 			lookup3d[++j] = c9;
// 			lookup3d[++j] = cA;
// 			lookup3d[++j] = cB;
// 			lookup3d[++j] = cC;
// 			lookup3d[++j] = cD;
// 		}
		
// 		//Fill Perm Array -- TODO find why

// 		short source[PSIZE];
// 		for(int i = 0; i < PSIZE; i++) {
// 			source[i] = i;
// 		}
// 		for(int i = PSIZE - 1; i >= 0; i--) {
// 			//Whyyyy
// 			seed = seed * 6364136223846793005L + 1442695040888963407L;
// 			int r = (int)((seed + 31) % (i + 1));
// 			if (r < 0) {
// 				r += (i + 1);
// 			}
// 			perm[i] = source[r];
// 			permGrad3d[i] = gradients3d[perm[i]];
// 			source[r] = source[i];
// 		}

// 		CLErr(cl_mem clPerm = clCreateBuffer(programglobal::oclContext->getContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(perm), perm, &clhelpererr));
// 		CLErr(cl_mem clPermGrad3d = clCreateBuffer(programglobal::oclContext->getContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(permGrad3d), permGrad3d, &clhelpererr));
// 		CLErr(cl_mem clLookup3d = clCreateBuffer(programglobal::oclContext->getContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(lookup3d), lookup3d, &clhelpererr));
// 		cl_uint pointCount = dim[0] * dim[1] * dim[2];
// 		size_t globalWorkSize = pointCount;

// 		programglobal::oclContext->setKernelParameters(kernel, { param(0, clPerm), param(1, clPermGrad3d), param(2, clLookup3d), param(3, inputGrid), param(4, outputNoise.cl), param(5, pointCount) });
// 		programglobal::oclContext->runCLKernel(kernel, 1, &globalWorkSize, NULL, {outputNoise});

// 		clReleaseMemObject(clPerm);
// 		clReleaseMemObject(clPermGrad3d);
// 		clReleaseMemObject(clLookup3d);
// 	} catch(string errorString) {
// 		throwErr(errorString);
// 	}
// }

// GLuint createNoiseTexture(cl_kernel kernel, noisetype type, cl_mem inputGrid, const int* dim, float amplitude, long seed) {
// 	GLuint tex;
// 	glGenTextures(1, &tex);
// 	try {
// 		clglmem outputNoise;
// 		if(type == Noise2D) {
// 			outputNoise = programglobal::oclContext->createCLGLBuffer(sizeof(float) * dim[0] * dim[1], GL_MAP_WRITE_BIT | GL_MAP_READ_BIT, CL_MEM_READ_WRITE);
// 			create2DNoiseTexture(kernel, inputGrid, outputNoise, ivec2(dim[0], dim[1]), amplitude, seed);
// 			glBindTexture(GL_TEXTURE_2D, tex);
// 			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, outputNoise.gl);
// 			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, dim[0], dim[1], 0, GL_RED, GL_FLOAT, NULL);
// 			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
// 			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
// 			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
// 			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
// 			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
// 		} else if(type == Noise3D) {
// 			outputNoise = programglobal::oclContext->createCLGLBuffer(sizeof(float) * dim[0] * dim[1] * dim[2], GL_MAP_WRITE_BIT | GL_MAP_READ_BIT, CL_MEM_READ_WRITE);
// 			create3DNoiseTexture(kernel, inputGrid, outputNoise, ivec3(dim[0], dim[1], dim[2]), amplitude, seed);
// 			glBindTexture(GL_TEXTURE_3D, tex);
// 			// glTexImage3D();
// 		} else {
// 			throwErr(ERROR_INVALID_NOISE);
// 		}
// 	} catch(string errString) {
// 		throwErr(errString);
// 	}
// 	return tex;
// }

// GLuint createFBMTexture(cl_kernel kernel, noisetype type, cl_mem inputGrid, const int* dim, int octaves, long seed) {
// 	GLuint tex;
// 	glGenTextures(1, &tex);
// 	try {
// 		clglmem outputNoise;
// 		if(type == FBM2D) {
// 			outputNoise = programglobal::oclContext->createCLGLBuffer(sizeof(float) * dim[0] * dim[1], GL_MAP_WRITE_BIT | GL_MAP_READ_BIT, CL_MEM_READ_WRITE);
// 			create2DFBMTexture(kernel, inputGrid, outputNoise, ivec2(dim[0], dim[1]), octaves, seed);
// 			glBindTexture(GL_TEXTURE_2D, tex);
// 			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, outputNoise.gl);
// 			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, dim[0], dim[1], 0, GL_RED, GL_FLOAT, NULL);
// 			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
// 			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
// 			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
// 			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
// 			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
// 		} else if(type == FBM3D) {
// 			outputNoise = programglobal::oclContext->createCLGLBuffer(sizeof(float) * dim[0] * dim[1] * dim[2], GL_MAP_WRITE_BIT | GL_MAP_READ_BIT, CL_MEM_READ_WRITE);
// 			// create3DNoiseTexture(kernel, inputGrid, outputNoise, ivec3(dim[0], dim[1], dim[2]), amplitude, seed);
// 			glBindTexture(GL_TEXTURE_3D, tex);
// 			// glTexImage3D();
// 		} else {
// 			throwErr(ERROR_INVALID_NOISE);
// 		}
// 	} catch(string errString) {
// 		throwErr(errString);
// 	}
// 	return tex;
// }

// GLuint opensimplexnoise::createNoiseTextureOnUniformInput(noisetype type, const int* dim, const int* offset, float timeInterval, float amplitude, long seed) {
// 	if(noisekernelnamelookup.count(type) == 0) {
// 		throwErr("Invalid Noise Type");
// 	}
// 	cl_kernel noiseKernel = programglobal::oclContext->getKernel(noisekernelnamelookup[type]);
// 	// programglobal::oclContext->printKernelList(cout);
// 	cl_mem inputGrid;
// 	inputGrid = createUniformInput(type, dim, offset, timeInterval);
// 	GLuint outputNoise = createNoiseTexture(noiseKernel, type, inputGrid, dim, amplitude, seed);
// 	clReleaseMemObject(inputGrid);

// 	return outputNoise;
// }

// GLuint opensimplexnoise::createFBMTextureOnUniformInput(noisetype type, const int* dim, const int* offset, float timeInterval, int octaves, long seed) {
// 	if(fbmkernelnamelookup.count(type) == 0) {
// 		throwErr("Invalid FBM Type");
// 	}
// 	cl_kernel noiseKernel = programglobal::oclContext->getKernel(fbmkernelnamelookup[type]);
// 	// programglobal::oclContext->printKernelList(cout);
// 	cl_mem inputGrid;
// 	inputGrid = createUniformInput(type, dim, offset, timeInterval);
// 	GLuint outputNoise = createFBMTexture(noiseKernel, type, inputGrid, dim, octaves, seed);
// 	clReleaseMemObject(inputGrid);

// 	return outputNoise;
// }