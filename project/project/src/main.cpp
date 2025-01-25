#include <glad/glad.h>
#include "project/config.hpp"
#include <GLFW/glfw3.h>
#include "tools.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glengine/orbitalCamera.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void onMouseMove(GLFWwindow* window, double xpos, double ypos);
void onMouseButton(GLFWwindow* window, int button, int action, int mods);
void onMouseScroll(GLFWwindow* window, double xoffset, double yoffset);

//VARIABLES USED IN THE PROGRAM

// Near and far plane
const float nearPlane = 0.1f;
const float farPlane = 10.0f;

// Mouse state
bool firstMouse = true;
float lastX;
float lastY;

enum class MousePressedButton { NONE, LEFT, RIGHT, MIDDLE };
MousePressedButton mouseButtonState = MousePressedButton::NONE;

GLEngine::OrbitalCamera orbitalCamera(glm::vec3(0.3f, 0.4f, 3.0f), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

//Background color of the rendering
glm::vec3 backgroundColor(0.0f, 0.0f, 0.0f);
float backgroundColorArray[3] = { backgroundColor.r, backgroundColor.g, backgroundColor.b };

//Position of the light, stored in an array for ImGui
glm::vec3 lightPos(0.5f, 1.0f, 4.0f);
float lightPosArray[3] = { lightPos.x, lightPos.y, lightPos.z };

//Color of the light, stored in an array for ImGui
glm::vec3 lightColor(0.8f, 0.8f, 0.8f);
float lightColorArray[3] = { lightColor.r, lightColor.g, lightColor.b };

// Color of the dragon, stored in an array for ImGui
glm::vec3 dragonColor(1.0f, 1.0f, 1.0f);
float dragonColorArray[3] = { dragonColor.r, dragonColor.g, dragonColor.b };

glm::vec3 outlineColor(0.0f, 0.0f, 0.0f);
float outlineColorArray[3] = { outlineColor.r, outlineColor.g, outlineColor.b };

float outlineThickness = 0.01f;

//Dithering (replacing all the 'dithering' fragments by 'dithering color')
int dithering = 4;
glm::vec3 ditheringColor(0.0f, 0.0f, 0.0f);
float ditheringColorArray[3] = { ditheringColor.r, ditheringColor.g, ditheringColor.b };

//The intensity of the dithering (black pencil effect) of the dragon
float edgeThreshold = 0.3f;
//Color of the normals discontinuity
glm::vec3 edgeColor(0.0f, 0.0f, 0.0f);
float edgeColorArray[3] = { edgeColor.r, edgeColor.g, edgeColor.b };

//The number of colors used for the dragon gradient
int colorThreshold = 5;

// Showing the ImGui window
bool imgui_window = true;

string currentObjFile;
vector<string> availableObjFiles;

float modelRotationX = 0.0f;     // Initial X rotation 
float modelRotationY = -120.0f;  // Initial Y rotation 
float modelRotationZ = 0.0f;     // Initial Z rotation

// Rendering style (with the mesh or not)
bool showMesh = false;

int main() {

    //Base shaders
    string vertexShaderSource = string(_resources_directory).append("shaders/simple.vert");
    string fragmentShaderSource = string(_resources_directory).append("shaders/simple.frag");

    //Normales
    string vertexNormalShaderSource = string(_resources_directory).append("shaders/lighting.vert");
    string fragmentNormalShaderSource = string(_resources_directory).append("shaders/lighting.frag");

    //Outlines
    string vertexOutlineShaderSource = string(_resources_directory).append("shaders/outline.vert");
    string fragmentOutlineShaderSource = string(_resources_directory).append("shaders/outline.frag");
    
    //Reading shaders
    //Vertex Shader
    string vertexShaderCode = readVertexShader(vertexShaderSource);
    //Fragment Shader
    string fragmentShaderCode = readFragmentShader(fragmentShaderSource);

    //Vertex Shader of the normals
    string vertexNormalShaderCode = readVertexShader(vertexNormalShaderSource);
    //Fragment Shader of the normals
    string fragmentNormalShaderCode = readFragmentShader(fragmentNormalShaderSource); 

    //Outline Vertex Shader
    string vertexOutlineShaderCode = readVertexShader(vertexOutlineShaderSource);
    //Outline fragment Shader
    string fragmentOutlineShaderCode = readFragmentShader(fragmentOutlineShaderSource);

    //Converting all results to const char *
    const char * vertexShaderCodeCStr = vertexShaderCode.c_str();
    const char * vertexNormalShaderCodeCStr = vertexNormalShaderCode.c_str();
    const char * vertexOutlineShaderCodeCStr = vertexOutlineShaderCode.c_str();

    const char * fragmentShaderCodeCStr = fragmentShaderCode.c_str();
    const char * fragmentNormalShaderCodeCStr = fragmentNormalShaderCode.c_str();
    const char * fragmentOutlineShaderCodeCStr = fragmentOutlineShaderCode.c_str();


    vector<float> vertices;
    vector<unsigned int> faces;
    vector<float> texCoords;
    vector<float> normals;

    unsigned int VAO, VBO, EBO;

    unsigned int lightingVAO, normalVBO;

    unsigned int vertexShader, vertexNormalShader, vertexOutlineShader;

    unsigned int fragmentShader, fragmentNormalShader, fragmentOutlineShader;

    unsigned int shaderProgram, lightingProgram, outlineProgram;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int width = 1280;
    int height = 920;
    GLFWwindow* window = glfwCreateWindow(width, height, "OpenGL project", NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }  
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  

    //Setting up ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    glViewport(0, 0, width, height);

    //Avoid the depth issues when rendering a 3D object in the shaders
    glEnable(GL_DEPTH_TEST);  
    glEnable(GL_STENCIL_TEST);  
    //To see the mesh
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &normalVBO);
    glGenBuffers(1, &EBO);

    //Main VAO
    glBindVertexArray(VAO);

    //VBO with vertices data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    //EBO with faces data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(unsigned int), faces.data(), GL_STATIC_DRAW);

    //Give the vertices to the vertex shader
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //Give the normalVBO the normals data
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);

    //Give the attributes to the shader
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    //Deactivate the VAO
    glBindVertexArray(0);

    //VAO for the lighting
    glGenVertexArrays(1, &lightingVAO);
    glBindVertexArray(lightingVAO);

    //VBO data for the lighting this time
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //Bind the EBO to that VAO too
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    //Deactivate the current VAO
    glBindVertexArray(0);

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderCodeCStr, NULL);
    glCompileShader(vertexShader);

    vertexNormalShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexNormalShader, 1, &vertexNormalShaderCodeCStr, NULL);
    glCompileShader(vertexNormalShader);

    vertexOutlineShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexOutlineShader, 1, &vertexOutlineShaderCodeCStr, NULL);
    glCompileShader(vertexOutlineShader);

    int  success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
    }

    glGetShaderiv(vertexNormalShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertexNormalShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::NORMAL::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
    }

    glGetShaderiv(vertexOutlineShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertexOutlineShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::OUTLINE::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
    }

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderCodeCStr, NULL);
    glCompileShader(fragmentShader);

    fragmentNormalShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentNormalShader, 1, &fragmentNormalShaderCodeCStr, NULL);
    glCompileShader(fragmentNormalShader);

    fragmentOutlineShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentOutlineShader, 1, &fragmentOutlineShaderCodeCStr, NULL);
    glCompileShader(fragmentOutlineShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    lightingProgram = glCreateProgram();
    glAttachShader(lightingProgram, vertexNormalShader);
    glAttachShader(lightingProgram, fragmentNormalShader);
    glLinkProgram(lightingProgram);

    outlineProgram = glCreateProgram();
    glAttachShader(outlineProgram, vertexOutlineShader);
    glAttachShader(outlineProgram, fragmentOutlineShader);
    glLinkProgram(outlineProgram);


    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cout << "Error linking the shaders:\n" << infoLog <<  endl;
    }

    glfwSetMouseButtonCallback(window, onMouseButton);
    glfwSetCursorPosCallback(window, onMouseMove);
    glfwSetScrollCallback(window, onMouseScroll);

    string objDir = string(_resources_directory) + "../objects/";
    availableObjFiles = listObjFiles(objDir);
    if (!availableObjFiles.empty()) {
        currentObjFile = availableObjFiles[0];
        string fullPath = string(_resources_directory) + currentObjFile;
        loadModel(fullPath, vertices, faces, texCoords, normals, VBO, EBO, normalVBO);
    } 
    else {
        cerr << "No .obj files found in " << objDir << endl;
        return -1;
    }

    while(!glfwWindowShouldClose(window)){

        processInput(window);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if(imgui_window){
            ImGui::Begin("OpenGL Project", &imgui_window);

            if (ImGui::CollapsingHeader("Background")){
                ImGui::ColorEdit3("Background color", backgroundColorArray);
                backgroundColor = glm::vec3(backgroundColorArray[0], backgroundColorArray[1], backgroundColorArray[2]);
            }

            // Dragon
            if (ImGui::CollapsingHeader("Model")) {
                //Showing the mesh or not
                ImGui::Checkbox("Show mesh", &showMesh);
                //Gets the name of the file by removing all the path before
                if (ImGui::BeginCombo("Object file", currentObjFile.substr(currentObjFile.find_last_of("/") + 1).c_str())) {
                    for (const string& file : availableObjFiles) {
                        string filename = file.substr(file.find_last_of("/") + 1);
                        bool isSelected = currentObjFile == file;
                        if (ImGui::Selectable(filename.c_str(), isSelected)) 
                            if (currentObjFile != file) {
                                currentObjFile = file;
                                string fullPath = string(_resources_directory) + currentObjFile;
                                loadModel(fullPath, vertices, faces, texCoords, normals, VBO, EBO, normalVBO);
                            }
                        if (isSelected) 
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::ColorEdit3("Model color", dragonColorArray);
                dragonColor = glm::vec3(dragonColorArray[0], dragonColorArray[1], dragonColorArray[2]);
                ImGui::ColorEdit3("Outline color", outlineColorArray);
                outlineColor = glm::vec3(outlineColorArray[0], outlineColorArray[1], outlineColorArray[2]);
                ImGui::SliderFloat("Outline Thickness", &outlineThickness, 0.0f, 0.1f);   
                
                ImGui::SliderFloat("Rotation X", &modelRotationX, -360.0f, 360.0f);
                ImGui::SliderFloat("Rotation Y", &modelRotationY, -360.0f, 360.0f);
                ImGui::SliderFloat("Rotation Z", &modelRotationZ, -360.0f, 360.0f);
            }
            
            // Light
            if (ImGui::CollapsingHeader("Light")) {
                ImGui::SliderFloat3("Light position", lightPosArray, -100, 100);
                lightPos = glm::vec3(lightPosArray[0], lightPosArray[1], lightPosArray[2]);
                ImGui::ColorEdit3("Light Color", lightColorArray);
                lightColor = glm::vec3(lightColorArray[0], lightColorArray[1], lightColorArray[2]);
            }

            // NPR
            if (ImGui::CollapsingHeader("NPR settings")) {
                ImGui::SliderInt("Color threshold", &colorThreshold, 1, 50);
                ImGui::SliderFloat("Edge threshold", &edgeThreshold, 0.0f, 1.0f);
                ImGui::ColorEdit3("Edges color", edgeColorArray);
                edgeColor = glm::vec3(edgeColorArray[0], edgeColorArray[1], edgeColorArray[2]);
                ImGui::SliderInt("Dithering", &dithering, 1, 20);
                ImGui::ColorEdit3("Dithering Color", ditheringColorArray);
                ditheringColor = glm::vec3(ditheringColorArray[0], ditheringColorArray[1], ditheringColorArray[2]);
            }

            ImGui::End();
        }
        
        glStencilFunc(GL_ALWAYS, 1, 0xFF); 
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); 
        glStencilMask(0xFF);
        glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        if(showMesh)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        //Drawing the large dragon with the lighting effect
        glUseProgram(lightingProgram);

        //Matrix transformations for the dragon
        //Model matrix
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(modelRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(modelRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(modelRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(0.0f, -0.3f, 0.0f));

        //View matrix
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(-0.2f, 0.3f, -2.0f));
        view = orbitalCamera.getViewMatrix();
        
        //Projection matrix
        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(-45.0f), (float)width/(float)height, 0.1f, 100.0f);
        projection = glm::perspective(orbitalCamera.getFov(), (float)width / (float)height, nearPlane, farPlane);

        //Passing as uniforms
        //Light position passed as uniform
        int lightPosLoc = glGetUniformLocation(lightingProgram, "lightPos");
        glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);

        //Light color passed as uniform
        int lightColorLoc = glGetUniformLocation(lightingProgram, "lightColor");
        glUniform3f(lightColorLoc, lightColor.r, lightColor.g, lightColor.b);

        //Current position of the camera
        glm::vec3 viewPos = orbitalCamera.getPosition();
        glUniform3f(glGetUniformLocation(lightingProgram, "viewPos"), viewPos.x, viewPos.y, viewPos.z);

        //Color of the object (dragon)
        glUniform3f(glGetUniformLocation(lightingProgram, "dragonColor"), dragonColor.r, dragonColor.g, dragonColor.b);

        //Dithering (and dithering color) of the dragon
        glUniform1iv(glGetUniformLocation(lightingProgram, "dithering"), 1, &dithering);
        glUniform3f(glGetUniformLocation(lightingProgram, "ditheringColor"), ditheringColor.r, ditheringColor.g, ditheringColor.b);

        glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1iv(glGetUniformLocation(lightingProgram, "nbColors"), 1, &colorThreshold);
        glUniform1fv(glGetUniformLocation(lightingProgram, "edgeThreshold"), 1, &edgeThreshold);
        glUniform3f(glGetUniformLocation(lightingProgram, "edgeColor"), edgeColor.r, edgeColor.g, edgeColor.b);

        //Bind the dragon's VAO and draw it
        
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, faces.size(), GL_UNSIGNED_INT, 0);

        glStencilFunc(GL_NOTEQUAL, 1, 0xFF); 
        glStencilMask(0x00); 
        glDisable(GL_DEPTH_TEST);
        glUseProgram(outlineProgram);

        //Passing as uniforms
        glUniformMatrix4fv(glGetUniformLocation(outlineProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(outlineProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(outlineProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(outlineProgram, "outlineColor"), outlineColor.r, outlineColor.g, outlineColor.b);
        glUniform1f(glGetUniformLocation(outlineProgram, "outlineThickness"), outlineThickness);
        glDrawElements(GL_TRIANGLES, faces.size(), GL_UNSIGNED_INT, 0);
        glEnable(GL_DEPTH_TEST);

        //Base shader for the light source (little dragon)
        glUseProgram(shaderProgram);

        //Matrix transformations for the little dragon
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        //Normal VAO
        glBindVertexArray(lightingVAO);
        glDrawElements(GL_TRIANGLES, faces.size(), GL_UNSIGNED_INT, 0);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &lightingVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &normalVBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(lightingProgram);
    glDeleteProgram(outlineProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexNormalShader);
    glDeleteShader(fragmentNormalShader);
    glDeleteShader(vertexOutlineShader);
    glDeleteShader(fragmentOutlineShader);
    glfwTerminate();

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void onMouseButton(GLFWwindow* window, int button, int action, int mods) {
    if (!ImGui::GetIO().WantCaptureMouse) {
        if (action == GLFW_RELEASE) {
            mouseButtonState = MousePressedButton::NONE;
        }
        else {
            switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT: mouseButtonState = MousePressedButton::LEFT;
                break;
            case GLFW_MOUSE_BUTTON_RIGHT: mouseButtonState = MousePressedButton::RIGHT;
                break;
            case GLFW_MOUSE_BUTTON_MIDDLE: mouseButtonState = MousePressedButton::MIDDLE;
                break;
            }
        }
    }
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods); 
}

void onMouseMove(GLFWwindow* window, double xpos, double ypos) {
    if (!ImGui::GetIO().WantCaptureMouse) {
        if (mouseButtonState == MousePressedButton::NONE) {
            lastX = (float)xpos;
            lastY = (float)ypos;
        }
        else {
            if (firstMouse) {
                lastX = xpos;
                lastY = ypos;
                firstMouse = false;
            }

            float xoffset = (float)xpos - lastX;
            float yoffset = lastY - (float)ypos;

            lastX = (float)xpos;
            lastY = (float)ypos;

            switch (mouseButtonState) {
                case MousePressedButton::LEFT: orbitalCamera.orbit(xoffset, yoffset);
                    break;
                case MousePressedButton::RIGHT:
                    orbitalCamera.track(xoffset);
                    orbitalCamera.pedestal(yoffset);
                    break;
                case MousePressedButton::MIDDLE: orbitalCamera.dolly(yoffset);
                    break;
                case MousePressedButton::NONE: break;
            }
        }
    }
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos); 
}

void onMouseScroll(GLFWwindow* window, double xoffset, double yoffset) {
    if (!ImGui::GetIO().WantCaptureMouse) {
        orbitalCamera.zoom((float)yoffset);
    }
}