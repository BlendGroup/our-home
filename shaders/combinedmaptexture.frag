#version 460 core

layout(location = 0)out float HeightMap;

layout(binding = 0)uniform sampler2D texValley;
layout(binding = 1)uniform sampler2D texMountain;
layout(binding = 2)uniform sampler2D texTerrainMap;
layout(binding = 3)uniform sampler2D texLakeMap;
layout(binding = 4)uniform sampler2D texRoadMap;

in vec2 texCoord;

void main(void) {
	float roadH = texture(texRoadMap, texCoord).r;
	float valleyH = texture(texValley, texCoord).r * mix(10.0, 1.0, roadH);
	float mountainH = texture(texMountain, texCoord).r * mix(150.0, 1.0, roadH);
	float lakeH = texture(texLakeMap, texCoord).r * 30.0;
	float mapVal = texture(texTerrainMap, texCoord).r;
	HeightMap = mix(valleyH, mountainH, mapVal) - lakeH;
}