kernel void calcNormal(read_only image2d_t heightMap, write_only image2d_t normalMap) {
	int2 coord = (int2)(get_global_id(0), get_global_id(1));

	float3 current 	= (float3)((float)coord.x / get_image_width(heightMap) * 2.0 - 1.0, read_imagef(heightMap, coord).r * 2.0 - 1.0, (float)coord.y  / get_image_height(heightMap) * 2.0 - 1.0);
	float3 top 		= (float3)((float)coord.x / get_image_width(heightMap) * 2.0 - 1.0, read_imagef(heightMap, coord + (int2)(0, -1)).r * 2.0 - 1.0, (float)(coord.y - 1)  / get_image_height(heightMap) * 2.0 - 1.0);
	float3 bottom 	= (float3)((float)coord.x / get_image_width(heightMap) * 2.0 - 1.0, read_imagef(heightMap, coord + (int2)(0, 1)).r * 2.0 - 1.0, (float)(coord.y + 1)  / get_image_height(heightMap) * 2.0 - 1.0);
	float3 left 	= (float3)((float)(coord.x - 1) / get_image_width(heightMap) * 2.0 - 1.0, read_imagef(heightMap, coord + (int2)(-1, 0)).r * 2.0 - 1.0, (float)coord.y  / get_image_height(heightMap) * 2.0 - 1.0);
	float3 right 	= (float3)((float)(coord.x + 1) / get_image_width(heightMap) * 2.0 - 1.0, read_imagef(heightMap, coord + (int2)(1, 0)).r * 2.0 - 1.0, (float)coord.y  / get_image_height(heightMap) * 2.0 - 1.0);

	float3 n1 = normalize(cross(top - current, left - current));
	float3 n2 = normalize(cross(left - current, bottom - current));
	float3 n3 = normalize(cross(bottom - current, right - current));
	float3 n4 = normalize(cross(right - current, top - current));

	write_imagef(normalMap, coord, normalize((float4)(n1 + n2 + n3 + n4, 1.0)));
}