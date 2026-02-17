#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <string>

class TextureLoader {

	static std::string directory;

public:

	static void setDirectory(const std::string& directoryPath);

	static GLuint LoadTGA(const char* file_path);
	
};



#endif