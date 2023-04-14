kernel void calcNormal(read_only image2d_t heightMap, write_only image2d_t normalMap) {
	int2 coord = (int2)(get_global_id(0), get_global_id(1));
	float4 color = read_imagef(heightMap, coord);
	write_imagef(normalMap, coord, color);
}