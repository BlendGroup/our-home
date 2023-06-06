#include<shapes.h>
#include<vmath.h>

using namespace vmath;

typedef struct vertex_t {
	vec3 position;
	vec3 normal;
	vec2 texcoord;
} vertex_t;

void initQuad(buffer &q) {
	vertex_t array[] = {
		//Position			//Normal			//TexCoords
		{vec3(1.0f, 1.0f, 0.0f),	vec3(0.0f, 0.0f, 1.0f),	vec2(1.0f, 1.0f)},
		{vec3(-1.0f, 1.0f, 0.0f),	vec3(0.0f, 0.0f, 1.0f),	vec2(0.0f, 1.0f)},
		{vec3(1.0f, -1.0f, 0.0f),	vec3(0.0f, 0.0f, 1.0f),	vec2(1.0f, 0.0f)},
		{vec3(-1.0f, -1.0f, 0.0f),	vec3(0.0f, 0.0f, 1.0f),	vec2(0.0f, 0.0f)}
	};
	glGenVertexArrays(1, &q.vao);
	glGenBuffers(1, &q.vbo);
	glBindVertexArray(q.vao);
	glBindBuffer(GL_ARRAY_BUFFER, q.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(array), array, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)(offsetof(vertex_t, position)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)(offsetof(vertex_t, normal)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)(offsetof(vertex_t, texcoord)));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	q.vertexCount = 4;
}

void initSphere(buffer &q, unsigned stacks, unsigned slices) {
	const float radius = 1.0f;
	glGenVertexArrays(1, &q.vao);
	glBindVertexArray(q.vao);

	glGenBuffers(1, &q.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, q.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t) * (stacks + 1) * (slices + 1), NULL, GL_STATIC_DRAW);
	vertex_t *array = (vertex_t*)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(vertex_t) * (stacks + 1) * (slices + 1), GL_MAP_WRITE_BIT);

	int index = 0;
	for(int i = 0; i <= stacks; i++) {
		float phi = M_PI * ((float)i / stacks);
		for(int j = 0; j <= slices; j++) {
			float theta = 2.0f * M_PI * ((float)j / slices);
			vec3 pos = vec3(sin(phi) * sin(theta) * radius, cos(phi) * radius, sin(phi) * cos(theta) * radius);
			array[index].position = pos;
			array[index].normal = normalize(pos);
			array[index].texcoord = vec2((float)j / slices, 1.0f - ((float)i / stacks));
			index++;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)(offsetof(vertex_t, position)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)(offsetof(vertex_t, normal)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)(offsetof(vertex_t, texcoord)));

	glGenBuffers(1, &q.eabo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, q.eabo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uvec3) * slices * stacks * 2, NULL, GL_STATIC_DRAW);
	uvec3 *elementarray = (uvec3*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uvec3) * stacks * slices * 2, GL_MAP_WRITE_BIT);
	
	index = 0;
	for(int i = 0; i < stacks; i++) {
		for(int j = 0; j < slices; j++) {
			elementarray[index++] = uvec3(i * (slices + 1) + j, (i + 1) * (slices + 1) + j, (i + 1) * (slices + 1) + j + 1);
			elementarray[index++] = uvec3((i + 1) * (slices + 1) + j + 1, i * (slices + 1) + j + 1, i * (slices + 1) + j);
		}
	}
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	q.vertexCount = stacks * slices * 2 * 3;
}

shaperenderer::shaperenderer(void) {
	initQuad(this->quad);
	initSphere(this->sphere, 25, 50);
}

void shaperenderer::renderQuad(void) {
	glBindVertexArray(this->quad.vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, this->quad.vertexCount);
	glBindVertexArray(0);
}

void shaperenderer::renderSphere(void) {
	glBindVertexArray(this->sphere.vao);
	glDrawElements(GL_TRIANGLES, this->sphere.vertexCount, GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
}

shaperenderer::~shaperenderer() {
	glDeleteVertexArrays(1, &this->quad.vao);
	glDeleteBuffers(1, &this->quad.vbo);
	glDeleteVertexArrays(1, &this->sphere.vao);
	glDeleteBuffers(1, &this->sphere.eabo);
	glDeleteBuffers(1, &this->sphere.vbo);
}