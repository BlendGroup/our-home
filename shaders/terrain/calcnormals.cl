kernel void calcNormal(read_only image2d_t heightMap, write_only image2d_t normalMap) {
	int2 coord = (int2)(get_global_id(0), get_global_id(1));

	int2 imageDim = get_image_dim(heightMap) - (int2)(1, 1);

	float3 current 	= (float3)((float)coord.x / imageDim.x * 2.0 - 1.0, read_imagef(heightMap, coord).r / 50.0f, (float)coord.y  / imageDim.y * 2.0 - 1.0);
	float3 top 		= (float3)((float)coord.x / imageDim.x * 2.0 - 1.0, read_imagef(heightMap, coord + (int2)(0, -1)).r / 50.0f, (float)(coord.y - 1)  / imageDim.y * 2.0 - 1.0);
	float3 bottom 	= (float3)((float)coord.x / imageDim.x * 2.0 - 1.0, read_imagef(heightMap, coord + (int2)(0, 1)).r / 50.0f, (float)(coord.y + 1)  / imageDim.y * 2.0 - 1.0);
	float3 left 	= (float3)((float)(coord.x - 1) / imageDim.x * 2.0 - 1.0, read_imagef(heightMap, coord + (int2)(-1, 0)).r / 50.0f, (float)coord.y  / imageDim.y * 2.0 - 1.0);
	float3 right 	= (float3)((float)(coord.x + 1) / imageDim.x * 2.0 - 1.0, read_imagef(heightMap, coord + (int2)(1, 0)).r / 50.0f, (float)coord.y  / imageDim.y * 2.0 - 1.0);

	float3 n1 = normalize(cross(top - current, left - current));
	float3 n2 = normalize(cross(left - current, bottom - current));
	float3 n3 = normalize(cross(bottom - current, right - current));
	float3 n4 = normalize(cross(right - current, top - current));

	write_imagef(normalMap, coord, (float4)(normalize(n1 + n2 + n3 + n4), 1.0));
}