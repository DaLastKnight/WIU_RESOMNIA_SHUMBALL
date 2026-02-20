
#include <iostream>
#include <fstream>
#include <GL\glew.h>

#include "Console.h"

#pragma warning(push)
#pragma warning(disable : 6262)
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "../../stb/include/stb_image.h"
#pragma warning(pop)

#include "TextureLoader.h"

std::string TextureLoader::directory = "Image/";

void TextureLoader::SetDirectory(const std::string& directoryPath) {

	directory = directoryPath;

	if (directory.back() != '/') {
		directory += "/";
	}
}

GLuint TextureLoader::LoadTexture(const char* file_path) {
	std::string filePath(file_path);
	if (filePath.find(".tga") != std::string::npos || filePath.find(".TGA") != std::string::npos) {
		return LoadTGA(file_path);
	}
	if (filePath.find(".png") != std::string::npos || filePath.find(".PNG") != std::string::npos) {
		return LoadPNG(file_path);
	}

	Error("TextureLoader::LoadTexture(): invalid file extension: " + filePath);
	return GLuint();
}

GLuint TextureLoader::LoadTGA(const char *file_path)				// load TGA file to memory
{
	std::string actualFilePath = directory + file_path;
	std::ifstream fileStream(actualFilePath, std::ios::binary);
	if(!fileStream.is_open()) {
		Error("TextureLoader::LoadTGA(): Impossible to open " + actualFilePath + ". Are you in the right directory?");
		return 0;
	}

	GLubyte		header[ 18 ];									// first 6 useful header bytes
	GLuint		bytesPerPixel;								    // number of bytes per pixel in TGA gile
	GLuint		imageSize;									    // for setting memory
	GLubyte *	data;
	GLuint		texture = 0;
	unsigned	width, height;

	fileStream.read((char*)header, 18);
	width = header[12] + header[13] * 256;
	height = header[14] + header[15] * 256;

 	if(	width <= 0 ||								// is width <= 0
		height <= 0 ||								// is height <=0
		(header[16] != 24 && header[16] != 32))		// is TGA 24 or 32 Bit
	{
		fileStream.close();							// close file on failure
		Error("TextureLoader::LoadTGA(): File header error.");
		return 0;										
	}

	bytesPerPixel	= header[16] / 8;						//divide by 8 to get bytes per pixel
	imageSize		= width * height * bytesPerPixel;	// calculate memory required for TGA data
	
	data = new GLubyte[ imageSize ];
	fileStream.seekg(18, std::ios::beg);
	fileStream.read((char *)data, imageSize);
	fileStream.close();	

	glGenTextures(1, &texture);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	if(bytesPerPixel == 3)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	else //bytesPerPixel == 4
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);

	//to do: modify the texture parameters code from here
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	float maxAnisotropy = 1.f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (GLint)maxAnisotropy);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//end of modifiable code

	delete []data;

	return texture;						
}

GLuint TextureLoader::LoadPNG(const char* filename)
{
	std::string actualFilePath = directory + filename;

	stbi_set_flip_vertically_on_load(true);

	int width, height, channels;
	unsigned char* img = stbi_load(actualFilePath.c_str(), &width, &height, &channels, 0);

	if (img == nullptr)
	{
		Error("TextureLoader::LoadPNG: Failed to load PNG: " + actualFilePath + "\nReason: " + stbi_failure_reason());
		return 0;
	}

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	if (channels == 4)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img);

	// Generate mipmaps after uploading texture
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	float maxAnisotropy = 1.f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (GLint)maxAnisotropy);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(img);

	return texture;
}