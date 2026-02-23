#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <string>
#include <map>

class TextureLoader {

	static std::string directory;

	static GLuint LoadTGA(const char* file_path);
	static GLuint LoadPNG(const char* filename);

public:

	static void SetDirectory(const std::string& directoryPath);

	static GLuint LoadTexture(const char* file_path);
};



#endif