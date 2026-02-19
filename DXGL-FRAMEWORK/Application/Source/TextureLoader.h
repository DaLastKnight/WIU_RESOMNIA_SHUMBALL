#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <string>
#include <map>

class TextureLoader {

	static std::string directory;

public:

	static void SetDirectory(const std::string& directoryPath);

	static GLuint LoadTGA(const char* file_path);
	static GLuint LoadPNG(const char* filename);
};



#endif