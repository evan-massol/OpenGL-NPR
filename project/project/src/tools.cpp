#include "tools.hpp"

vector<float> fetchAllVertices(const string& filename){
    ifstream verticesStream;
    string vertice;
    vector<float> vertices;

    verticesStream.open(filename);
    
    if(verticesStream.is_open()){
        do{
            getline(verticesStream, vertice);
            // If the line starts with 'v' 
            if (!vertice.empty() && vertice[0] == 'v' && vertice[1] == ' ') {
                // Only get what's after the v
                istringstream iss(vertice.substr(2));
                float point1, point2, point3;
                // Get the 3 points
                if(iss >> point1 >> point2 >> point3){
                    vertices.push_back(point1);
                    vertices.push_back(point2);
                    vertices.push_back(point3);
                }
                else
                    cerr << "Couldn't read the line for the vertices" << endl;
            }
        } while(!verticesStream.eof());
        verticesStream.close();
    }
    else
        cerr << "Couldn't open file " << filename << " to read the vertices." << endl;

    return vertices;
}


vector<unsigned int> fetchAllFaces(const string& filename){
    ifstream facesStream;
    string face;
    vector<unsigned int> faces;

    facesStream.open(filename);
    
    if(facesStream.is_open()){
        do{
            getline(facesStream, face);
            // If it starts with 'f' 
            if (!face.empty() && face[0] == 'f' && face[1] == ' ') {
                // Only get what's after the f
                istringstream iss(face.substr(2));
                unsigned int face1, face2, face3;
                // Get the 3 faces
                if(iss >> face1 >> face2 >> face3){
                    // Offset the index by 1
                    face1--;
                    face2--;
                    face3--;
                    faces.push_back(face1);
                    faces.push_back(face2);
                    faces.push_back(face3);
                }
                else
                    cerr << "Couldn't read the line for the faces" << endl;
            }
        } while(!facesStream.eof());
        facesStream.close();
    }
    else
        cerr << "Couldn't open file " << filename << " to read the faces." << endl;

    return faces;
}

vector<float> fetchAllTexCoords(const string& filename){
    ifstream texturesStream;
    string texCoord;
    vector<float> textures;

    texturesStream.open(filename);
    
    if(texturesStream.is_open()){
        do{
            getline(texturesStream, texCoord);
            // If the line starts with 'v' 
            if (!texCoord.empty() && texCoord[0] == 'v' && texCoord[1] == 't') {
                // Only get what's after the v
                istringstream iss(texCoord.substr(3));
                float point1, point2, point3;
                // Get the 3 points of the texture coordinates
                if(iss >> point1 >> point2 >> point3){
                    textures.push_back(point1);
                    textures.push_back(point2);
                    textures.push_back(point3);
                }
                else
                    cerr << "Couldn't read the line for the texture coordinates" << endl;
            }
        } while(!texturesStream.eof());
        texturesStream.close();
    }
    else
        std::cerr << "Couldn't open file " << filename << " to read the textures." << std::endl;

    return textures;
}


vector<float> computeNormal(const vector<float>& vertices,
                            const vector<unsigned int>& faces){
    vector<float> normals;
    //Initialize the normals vector
    for (size_t i = 0; i < vertices.size(); i++)
        normals.push_back(0);

    //Compute the normals
    for (unsigned int i = 0; i < faces.size() - 3; i+=3){
        unsigned int i1 = faces[i];
        unsigned int i2 = faces[i+1];
        unsigned int i3 = faces[i+2];

        //Vertices
        glm::vec3 v1 = glm::vec3(vertices[3*i1], vertices[3*i1 + 1], vertices[3*i1 + 2]);
        glm::vec3 v2 = glm::vec3(vertices[3*i2], vertices[3*i2 + 1], vertices[3*i2 + 2]);
        glm::vec3 v3 = glm::vec3(vertices[3*i3], vertices[3*i3 + 1], vertices[3*i3 + 2]);

        //Compute normal
        glm::vec3 normal = glm::cross(v3 - v1, v3 - v2);

        for (size_t di = 0; di < 3; di++){
            normals[3 * i1 + di] += normal[di];
            normals[3 * i2 + di] += normal[di];
            normals[3 * i3 + di] += normal[di];
        }
    }

    // Normalize
    for (size_t k = 0; k < normals.size() - 3; k += 3) {
        glm::vec3 n = glm::normalize(glm::vec3(normals[k], normals[k + 1], normals[k + 2]));
        normals[k] = n[0];
        normals[k+1] = n[1];
        normals[k+2] = n[2];
    }

    return normals;
}


string readVertexShader(const string& filename) {
    ifstream vertexShaderFile(filename);
    if (!vertexShaderFile.is_open()) {
        std::cerr << "Couldn't open the vertex shader file" << std::endl;
        return "";
    }
    stringstream iss;
    iss << vertexShaderFile.rdbuf();
    return iss.str();
}

string readFragmentShader(const string& filename) {
    ifstream fragmentShaderFile(filename);
    if (!fragmentShaderFile.is_open()) {
        std::cerr << "Couldn't open the fragment shader file" << std::endl;
        return "";
    }
    stringstream iss;
    iss << fragmentShaderFile.rdbuf();
    return iss.str();
}


//Reloading the 3D model
void loadModel(const string& filename, 
                vector<float>& vertices,
                vector<unsigned int>& faces, 
                vector<float>& texCoords,
                vector<float>& normals,
                int VBO, 
                int EBO, 
                int normalVBO) {
    vertices = fetchAllVertices(filename);
    faces = fetchAllFaces(filename);
    texCoords = fetchAllTexCoords(filename);
    normals = computeNormal(vertices, faces);

    // We update the buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(unsigned int), faces.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
}

vector<string> listObjFiles(const string& directory) {
    vector<string> objFiles;
    DIR *dir;
    struct dirent *ent;
    
    if ((dir = opendir(directory.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            string filename = ent->d_name;
            // We only read .obj files
            if (filename.length() > 4 && 
                filename.substr(filename.length() - 4) == ".obj") 
                objFiles.push_back("../objects/" + filename);
        }
        closedir(dir);
    }
    return objFiles;
}

GLuint loadTexture(const char* path) {
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        std::cerr << "Failed to load texture: " << path << std::endl;
    }
    stbi_image_free(data);
    
    return textureID;
}