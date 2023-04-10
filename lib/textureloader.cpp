#define STB_IMAGE_IMPLEMENTATION
#include<stb_image.h>
#include<gltextureloader.h>

#include<unordered_map>

using namespace std;

static unordered_map<string, GLuint> textureMap;

GLuint createTexture2D(string filename, GLint minFilter, GLint magFilter, GLint wrapS, GLint wrapT) {
	if(textureMap.count(filename) == 0) {
		int w, h, channels;

		GLuint texId;
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
		textureMap[filename] = texId;
		return texId;
	} else {
		return textureMap[filename];
	}
}