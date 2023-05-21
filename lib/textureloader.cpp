#include <cstddef>
#define STB_IMAGE_IMPLEMENTATION
#include<stb_image.h>
#include<gltextureloader.h>
#include<iostream>
#include<fstream>
#include<unordered_map>
#include<errorlog.h>

using namespace std;

static unordered_map<string, GLuint> textureMap;

void initTextureLoader() {
	stbi_set_flip_vertically_on_load(1);
}

bool isTexturePresent(string filename) {
	ifstream i(filename);
	bool b = i.is_open();
	i.close();
	return b;
}

unordered_map<int, GLenum> formatMap = {
	{1, GL_RED},
	{3, GL_RGB},
	{4, GL_RGBA},
};

GLuint createTexture2D(string filename, GLint minFilter, GLint magFilter, GLint wrapS, GLint wrapT) {
	if(textureMap.count(filename) == 0) {
		int w, h, channels;
		unsigned char* data = stbi_load(filename.c_str(), &w, &h, &channels, 0);

		if(data == NULL){
			throwErr("'" + filename + "': texture couldn't load");
		}
		GLenum format;
		if(formatMap.count(channels) == 0) {
			throwErr("error: Unknown Number of Channels");
		}
		format = formatMap[channels];
	
		GLuint texId;
		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_2D, texId);
		glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
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

GLuint createTextureCubemap(string filename, GLint minFilter, GLint magFilter, GLint wrapS, GLint wrapT) {
	if(textureMap.count(filename) == 0) {
		int w, h, channels;
		unsigned char* data;
		GLenum format;

		string dir = filename.substr(0, filename.find_last_of('.')) + "/";
		string ext = filename.substr(filename.find_last_of('.'));
		pair<string, GLenum> cubemapfaces[] = {
			{"px", GL_TEXTURE_CUBE_MAP_POSITIVE_X},
			{"nx", GL_TEXTURE_CUBE_MAP_NEGATIVE_X},
			{"py", GL_TEXTURE_CUBE_MAP_POSITIVE_Y},
			{"ny", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y},
			{"pz", GL_TEXTURE_CUBE_MAP_POSITIVE_Z},
			{"nz", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z}
		};

		GLuint texId;
		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texId);
		for(int i = 0; i < 6; i++) {
			data = stbi_load((dir + cubemapfaces[i].first + ext).c_str(), &w, &h, &channels, 0);
			if(data == NULL){
				throwErr("'" + (dir + cubemapfaces[i].first + ext) + "': texture couldn't load");
			}
			if(formatMap.count(channels) == 0) {
				throwErr("error: Unknown Number of Channels");
			}
			format = formatMap[channels];

			glTexImage2D(cubemapfaces[i].second, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, magFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapT);
		if(minFilter == GL_NEAREST_MIPMAP_NEAREST || minFilter == GL_NEAREST_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST || minFilter == GL_LINEAR_MIPMAP_LINEAR) {
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		}
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		textureMap[filename] = texId;
		return texId;
	} else {
		return textureMap[filename];
	}
}