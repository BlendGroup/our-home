#version 460 core

layout(location = 0)out float HeightMap;

layout(binding = 0)uniform sampler2D texLand;
layout(binding = 1)uniform sampler2D texTerrainMap;

in vec2 texCoord;

void main(void) {
	float land = texture(texLand, texCoord).r * 3.0;
	float mapVal = texture(texTerrainMap, texCoord).r * 30.0;
	HeightMap = land - mapVal;
}