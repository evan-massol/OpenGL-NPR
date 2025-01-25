#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <format>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <dirent.h>
#include "stbimage/stb_image.h"


using namespace std;

//Reading objects and applying normals
vector<unsigned int> fetchAllFaces(const string& filename);
vector<float> fetchAllVertices(const string& filename);
vector<float> fetchAllTexCoords(const string& filename);
vector<float> computeNormal(const vector<float>& vertices,
                            const vector<unsigned int>& faces);


//Reading shaders
string readVertexShader(const string& filename);
string readFragmentShader(const string& filename);

//Reloading the 3D model chosen
void loadModel(const string& filename, 
                vector<float>& vertices,
                vector<unsigned int>& faces, 
                vector<float>& texCoords,
                vector<float>& normals,
                int VBO, 
                int EBO, 
                int normalVBO);

//Listing OBJ files
vector<string> listObjFiles(const string& directory);
//Loading a texture
GLuint loadTexture(const char* path);
