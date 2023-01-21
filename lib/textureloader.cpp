#define STB_IMAGE_IMPLEMENTATION
#include"../include/stb_image.h"
#include"../include/gltextureloader.h"

using namespace std;

string createTexture2D(GLuint &texId, string filename, GLint minFilter, GLint magFilter, GLint wrapS, GLint wrapT) {
	int w, h, channels;

	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	stbi_uc* data = stbi_load(filename.c_str(), &w, &h, &channels, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
	if(minFilter == GL_NEAREST_MIPMAP_NEAREST || minFilter == GL_NEAREST_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST || minFilter == GL_LINEAR_MIPMAP_LINEAR) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	return "";
}