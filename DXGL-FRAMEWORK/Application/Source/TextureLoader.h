#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <string>
#include <map>

class TextureLoader {

	static std::string directory;

public:

	static void SetDirectory(const std::string& directoryPath);

	static TextureLoader& GetInstance();

	static GLuint LoadTGA(const char* file_path);
	void LoadPNG(int key, const char* filename);

private:
	std::map<int, unsigned int> m_textures;
};



#endif