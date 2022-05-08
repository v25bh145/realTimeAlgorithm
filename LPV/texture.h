#pragma once

#include <glad/glad.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../include/stb_image.h"

using namespace std;

unsigned int loadTexture(char const* path);
unsigned int loadCubemap(vector<string> paths, int& imageSize, int& nrComponents, vector<unsigned char*>& datas, bool getData = true);
void createTextureCubeMapNull(unsigned& texture, unsigned width, unsigned height, unsigned nrComponent);
void createTexture2DNull(unsigned& texture, unsigned width, unsigned height, unsigned internalFormat, unsigned format, unsigned type);
void createTexture3DNull(unsigned& texture, unsigned width, unsigned height, unsigned depth, unsigned internalFormat, unsigned format, unsigned type);
//void createTexture2DNull(unsigned& texture, unsigned width, unsigned height, unsigned nrComponent);
//void createTexture1DNull(unsigned& texture, unsigned width, unsigned nrComponent);