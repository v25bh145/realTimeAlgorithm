#ifndef MODEL_H
#define MODEL_H
#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <limits>
using namespace std;

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

class Model
{
public:
    // self-constructed data
    pair<glm::vec3, glm::vec3> boudingVolume;
    // model data 
    vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh>    meshes;
    string directory;
    bool gammaCorrection;
    Model() {
        boudingVolume.first.x = boudingVolume.first.y = boudingVolume.first.z = numeric_limits<float>::max();
        boudingVolume.second.x = boudingVolume.second.y = boudingVolume.second.z = numeric_limits<float>::min();
    }
    // constructor, expects a filepath to a 3D model.
    Model(string const& path, bool gamma = false) : gammaCorrection(gamma)
    {
        boudingVolume.first.x = boudingVolume.first.y = boudingVolume.first.z = numeric_limits<float>::max();
        boudingVolume.second.x = boudingVolume.second.y = boudingVolume.second.z = numeric_limits<float>::min();
        loadModel(path);
    }

    // draws the model, and thus all its meshes
    void Draw(Shader& shader);

private:
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const& path);

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene);

    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);

    pair<glm::vec3, glm::vec3> maxBoundingVolume(glm::vec3 point);
};


unsigned int TextureFromFile(const char* path, const string& directory, bool gamma);
#endif