#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <numeric>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;

#pragma GCC optimize("O3,unroll-loops")
#pragma GCC target("avx2")

glm::vec3 cameraPosition;
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f;
float pitch = -30.0f;
float radius = 40.0f;
float rotationSpeed = 0.1f;
bool firstMouse = true;
float lastX, lastY;

GLuint gProgram[2];
GLuint gBufferShader, quadshader, depthshader, lightingshader;
int gWidth, gHeight;
int quadFacesSize, gFacesSize, sphereFacesSize, bunnyFacesSize, teapotFacesSize;

GLint modelLoc1, viewLoc1, projectionLoc1,
    colorLoc1, specularLoc1;
GLint quadModelLoc, quadProjectLoc, quadViewLoc,
    quadTexLoc, depthModelLoc, depthProjectLoc, depthViewLoc,
    depthTexLoc;

GLint gPositionLoc, gNormalLoc, gAlbedoLoc, viewPosLoc;

GLint blurDepthLoc, blurColorLoc;

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix(1.0f);
glm::vec3 eyePos(0, 0, 0);

float angle;

int lightLocation;

int modelLoc2, projectLoc2, viewLoc2, eyePosLoc2, objColorLoc2, lightPos2;

struct Vertex
{
    Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) {}
    GLfloat x, y, z;
};

struct Texture
{
    Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) {}
    GLfloat u, v;
};

struct Normal
{
    Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) {}
    GLfloat x, y, z;
};

struct Face
{
    Face(int v[], int t[], int n[])
    {
        vIndex[0] = v[0];
        vIndex[1] = v[1];
        vIndex[2] = v[2];
        tIndex[0] = t[0];
        tIndex[1] = t[1];
        tIndex[2] = t[2];
        nIndex[0] = n[0];
        nIndex[1] = n[1];
        nIndex[2] = n[2];
    }
    GLuint vIndex[3], tIndex[3], nIndex[3];
};

enum ObjectType {
    Teapot,
    Bunny,
    Armadillo
};

int quadWidth = 40;
int numOfObjRows = 6;
int numOfObjects = numOfObjRows*numOfObjRows;
std::vector<ObjectType> objects(numOfObjects);

std::vector<Vertex> gVertices;
std::vector<Texture> gTextures;
std::vector<Normal> gNormals;
std::vector<Face> gFaces;
std::vector<glm::vec3> Colors;

GLuint gVertexAttribBuffer, gIndexBuffer, gTexCoordBuffer;
GLuint gQuadVertexAttribBuffer, gQuadIndexBuffer, gQuadTexCoordBuffer;
GLuint gSphereVertexAttribBuffer, gSphereIndexBuffer, gSphereTexCoordBuffer;
GLuint gBunnyVertexAttribBuffer, gBunnyIndexBuffer, gBunnyTexCoordBuffer;
GLuint gTeapotVertexAttribBuffer, gTeapotIndexBuffer, gTeapotTexCoordBuffer;
GLint gInVertexLoc, gInNormalLoc;
int gVertexDataSizeInBytes, gNormalDataSizeInBytes, gTexCoordDataSizeInBytes,
    gQuadVertexDataSizeInBytes, gQuadNormalDataSizeInBytes, gQuadTexCoordDataSizeInBytes,
    gSphereVertexDataSizeInBytes, gSphereNormalDataSizeInBytes, gSphereTexCoordDataSizeInBytes,
    gBunnyVertexDataSizeInBytes, gBunnyNormalDataSizeInBytes, gBunnyTexCoordDataSizeInBytes,
    gTeapotVertexDataSizeInBytes, gTeapotNormalDataSizeInBytes, gTeapotTexCoordDataSizeInBytes;

unsigned int gBuffer, gPosition, gTexDepth, gNormal, gAlbedoSpec;

glm::vec3 baseLightPositions[20] = {
        glm::vec3(19.0f, 2.0f, 17.0f), 
        glm::vec3(17.0f, 2.0f, 15.0f), 
        glm::vec3(10.0f, 2.0f, 8.0f),
        glm::vec3(5.0f, 2.0f, 3.0f), 
        glm::vec3(0.0f, 2.0f, -2.0f), 
        glm::vec3(-5.0f, 2.0f, -7.0f), 
        glm::vec3(-10.0f, 2.0f, -17.0f),
        glm::vec3(-15.0f, 2.0f, -13.0f), 
        glm::vec3(-17.0f, 2.0f, -15.0f), 
        glm::vec3(-19.0f, 2.0f, -17.0f),
        glm::vec3(19.0f, 2.0f, 1.0f), 
        glm::vec3(17.0f, 2.0f, 3.0f), 
        glm::vec3(10.0f, 2.0f, 10.0f),
        glm::vec3(5.0f, 2.0f, 15.0f), 
        glm::vec3(0.0f, 2.0f, -20.0f), 
        glm::vec3(-5.0f, 2.0f, -15.0f), 
        glm::vec3(-10.0f, 2.0f, -10.0f),
        glm::vec3(-15.0f, 2.0f, -5.0f), 
        glm::vec3(-17.0f, 2.0f, -3.0f), 
        glm::vec3(-19.0f, 2.0f, -1.0f)
    };

glm::vec3 lightPositions[20] = {
        glm::vec3(19.0f, 2.0f, 17.0f), 
        glm::vec3(17.0f, 2.0f, 15.0f), 
        glm::vec3(10.0f, 2.0f, 8.0f),
        glm::vec3(5.0f, 2.0f, 3.0f), 
        glm::vec3(0.0f, 2.0f, -2.0f), 
        glm::vec3(-5.0f, 2.0f, -7.0f), 
        glm::vec3(-10.0f, 2.0f, -17.0f),
        glm::vec3(-15.0f, 2.0f, -13.0f), 
        glm::vec3(-17.0f, 2.0f, -15.0f), 
        glm::vec3(-19.0f, 2.0f, -17.0f),
        glm::vec3(19.0f, 2.0f, 1.0f), 
        glm::vec3(17.0f, 2.0f, 3.0f), 
        glm::vec3(10.0f, 2.0f, 10.0f),
        glm::vec3(5.0f, 2.0f, 15.0f), 
        glm::vec3(0.0f, 2.0f, -20.0f), 
        glm::vec3(-5.0f, 2.0f, -15.0f), 
        glm::vec3(-10.0f, 2.0f, -10.0f),
        glm::vec3(-15.0f, 2.0f, -5.0f), 
        glm::vec3(-17.0f, 2.0f, -3.0f), 
        glm::vec3(-19.0f, 2.0f, -1.0f)
    };

bool ParseObj(const std::string &fileName)
{
    std::fstream myfile;

    // Open the input
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        std::string curLine;

        while (getline(myfile, curLine))
        {
            std::stringstream str(curLine);
            GLfloat c1, c2, c3;
            GLuint index[9];
            std::string tmp;

            if (curLine.length() >= 2)
            {
                if (curLine[0] == 'v')
                {
                    if (curLine[1] == 't') // texture
                    {
                        str >> tmp; // consume "vt"
                        str >> c1 >> c2;
                        gTextures.push_back(Texture(c1, c2));
                    }
                    else if (curLine[1] == 'n') // normal
                    {
                        str >> tmp; // consume "vn"
                        str >> c1 >> c2 >> c3;
                        gNormals.push_back(Normal(c1, c2, c3));
                    }
                    else // vertex
                    {
                        str >> tmp; // consume "v"
                        str >> c1 >> c2 >> c3;
                        gVertices.push_back(Vertex(c1, c2, c3));
                    }
                }
                else if (curLine[0] == 'f') // face
                {
                    str >> tmp; // consume "f"
                    char c;
                    int vIndex[3], nIndex[3], tIndex[3];
                    str >> vIndex[0];
                    str >> c >> c; // consume "//"
                    str >> nIndex[0];
                    str >> vIndex[1];
                    str >> c >> c; // consume "//"
                    str >> nIndex[1];
                    str >> vIndex[2];
                    str >> c >> c; // consume "//"
                    str >> nIndex[2];

                    assert(vIndex[0] == nIndex[0] &&
                           vIndex[1] == nIndex[1] &&
                           vIndex[2] == nIndex[2]); // a limitation for now

                    // make indices start from 0
                    for (int c = 0; c < 3; ++c)
                    {
                        vIndex[c] -= 1;
                        nIndex[c] -= 1;
                        tIndex[c] -= 1;
                    }

                    gFaces.push_back(Face(vIndex, tIndex, nIndex));
                }
                else
                {
                    std::cout << "Ignoring unidentified line in obj file: " << curLine << std::endl;
                }
            }

            // data += curLine;
            if (!myfile.eof())
            {
                // data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

    assert(gVertices.size() == gNormals.size());

    return true;
}

bool ReadDataFromFile(
    const string &fileName, ///< [in]  Name of the shader file
    string &data)           ///< [out] The contents of the file
{
    fstream myfile;

    // Open the input
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            data += curLine;
            if (!myfile.eof())
            {
                data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

    return true;
}

GLuint createVS(const char *shaderName)
{
    string shaderSource;

    string filename(shaderName);
    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar *shader = (const GLchar *)shaderSource.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &shader, &length);
    glCompileShader(vs);

    char output[1024] = {0};
    glGetShaderInfoLog(vs, 1024, &length, output);
    printf("VS compile log: %s\n", output);

    return vs;
}

GLuint createFS(const char *shaderName)
{
    string shaderSource;

    string filename(shaderName);
    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar *shader = (const GLchar *)shaderSource.c_str();

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &shader, &length);
    glCompileShader(fs);

    char output[1024] = {0};
    glGetShaderInfoLog(fs, 1024, &length, output);
    printf("FS compile log: %s\n", output);

    return fs;
}

void initShaders()
{
    // Create the programs

    gProgram[0] = glCreateProgram();
    gProgram[1] = glCreateProgram();
    gBufferShader = glCreateProgram();
    quadshader = glCreateProgram();
    depthshader = glCreateProgram();
    lightingshader = glCreateProgram();

    // Create the shaders for both programs

    GLuint vs1 = createVS("blur_vert.glsl");
    GLuint fs1 = createFS("blur_frag.glsl");
    GLuint vs2 = createVS("vert2.glsl");
    GLuint fs2 = createFS("frag2.glsl");
    GLuint vert1 = createVS("geometry_vert.glsl");
    GLuint frag1 = createFS("geometry_frag.glsl");
    GLuint quadvert = createVS("quad_vert.glsl");
    GLuint quadfrag = createFS("quad_frag.glsl");
    GLuint depthfrag = createFS("depth_frag.glsl");
    GLuint lightvert = createVS("lighting_vert.glsl");
    GLuint lightfrag = createFS("lighting_frag.glsl");

    // Attach the shaders to the programs

    glAttachShader(gProgram[0], vs1);
    glAttachShader(gProgram[0], fs1);

    glAttachShader(gProgram[1], vs2);
    glAttachShader(gProgram[1], fs2);

    glAttachShader(gBufferShader, vert1);
    glAttachShader(gBufferShader, frag1);

    glAttachShader(quadshader, quadvert);
    glAttachShader(quadshader, quadfrag);

    glAttachShader(depthshader, quadvert);
    glAttachShader(depthshader, depthfrag);

    glAttachShader(lightingshader, lightvert);
    glAttachShader(lightingshader, lightfrag);

    // Link the programs

    glLinkProgram(gProgram[0]);
    GLint status;
    glGetProgramiv(gProgram[0], GL_LINK_STATUS, &status);

    if (status != GL_TRUE)
    {
        cout << "Program link failed" << endl;
        exit(-1);
    }

    glLinkProgram(gProgram[1]);
    glGetProgramiv(gProgram[1], GL_LINK_STATUS, &status);

    if (status != GL_TRUE)
    {
        cout << "Program link failed" << endl;
        exit(-1);
    }

    glLinkProgram(gBufferShader);
    glGetProgramiv(gBufferShader, GL_LINK_STATUS, &status);

    if (status != GL_TRUE)
    {
        cout << "Program link failed" << endl;
        exit(-1);
    }

    glLinkProgram(quadshader);
    glGetProgramiv(quadshader, GL_LINK_STATUS, &status);

    if (status != GL_TRUE)
    {
        std::cout << "Program link failed" << std::endl;
        exit(-1);
    }

    glLinkProgram(depthshader);
    glGetProgramiv(depthshader, GL_LINK_STATUS, &status);

    if (status != GL_TRUE)
    {
        std::cout << "Program link failed" << std::endl;
        exit(-1);
    }

    glLinkProgram(lightingshader);
    glGetProgramiv(lightingshader, GL_LINK_STATUS, &status);

    if (status != GL_TRUE)
    {
        std::cout << "Program link failed" << std::endl;
        exit(-1);
    }

    // Get the locations of the uniform variables from both programs

    // gProgram[0] uniform locations
    blurDepthLoc = glGetUniformLocation(gProgram[0], "gTexDepth");
    blurColorLoc = glGetUniformLocation(gProgram[0], "gAlbedoSpec");

    // gProgram[1] uniform locations
    modelLoc2 = glGetUniformLocation(gProgram[1], "modelingMatrix");
    viewLoc2 = glGetUniformLocation(gProgram[1], "viewingMatrix");
    projectLoc2 = glGetUniformLocation(gProgram[1], "projectionMatrix");
    eyePosLoc2 = glGetUniformLocation(gProgram[1], "eyePos");
    objColorLoc2 = glGetUniformLocation(gProgram[1], "objColor");
    lightPos2 = glGetUniformLocation(gProgram[1], "lightPositions");

    // gBufferShader uniform locations
    modelLoc1 = glGetUniformLocation(gBufferShader, "model");
    viewLoc1 = glGetUniformLocation(gBufferShader, "view");
    projectionLoc1 = glGetUniformLocation(gBufferShader, "projection");
    colorLoc1 = glGetUniformLocation(gBufferShader, "objColor");
    specularLoc1 = glGetUniformLocation(gBufferShader, "objSpec");

    // quad shader uniform locations
    quadModelLoc = glGetUniformLocation(quadshader, "model");
    quadProjectLoc = glGetUniformLocation(quadshader, "projection");
    quadViewLoc = glGetUniformLocation(quadshader, "view");

    quadTexLoc = glGetUniformLocation(quadshader, "ourTexture");

    // depth shader locations
    depthModelLoc = glGetUniformLocation(depthshader, "model");
    depthProjectLoc = glGetUniformLocation(depthshader, "projection");
    depthViewLoc = glGetUniformLocation(depthshader, "view");

    depthTexLoc = glGetUniformLocation(depthshader, "ourTexture");

    // lighting shader locations
    gPositionLoc = glGetUniformLocation(lightingshader, "gPosition");
    gNormalLoc = glGetUniformLocation(lightingshader, "gNormal");
    gAlbedoLoc = glGetUniformLocation(lightingshader, "gAlbedoSpec");
    viewPosLoc = glGetUniformLocation(lightingshader, "viewPos");
    lightLocation = glGetUniformLocation(lightingshader, "lightPositions");
}

void initVBO()
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    assert(vao > 0);
    glBindVertexArray(vao);
    std::cout << "vao = " << vao << std::endl;

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &gVertexAttribBuffer);
    glGenBuffers(1, &gIndexBuffer);

    assert(gVertexAttribBuffer > 0 && gIndexBuffer > 0);

    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

    gVertexDataSizeInBytes = gVertices.size() * 3 * sizeof(GLfloat);
    gNormalDataSizeInBytes = gNormals.size() * 3 * sizeof(GLfloat);
    gTexCoordDataSizeInBytes = gTextures.size() * 2 * sizeof(GLfloat);
    int indexDataSizeInBytes = gFaces.size() * 3 * sizeof(GLuint);
    GLfloat *vertexData = new GLfloat[gVertices.size() * 3];
    GLfloat *normalData = new GLfloat[gNormals.size() * 3];
    GLfloat *texCoordData = new GLfloat[gTextures.size() * 2];
    GLuint *indexData = new GLuint[gFaces.size() * 3];

    for (int i = 0; i < gVertices.size(); ++i)
    {
        vertexData[3 * i] = gVertices[i].x;
        vertexData[3 * i + 1] = gVertices[i].y;
        vertexData[3 * i + 2] = gVertices[i].z;
    }

    for (int i = 0; i < gNormals.size(); ++i)
    {
        normalData[3 * i] = gNormals[i].x;
        normalData[3 * i + 1] = gNormals[i].y;
        normalData[3 * i + 2] = gNormals[i].z;
    }

    std::cout << "texCoordData: " << std::endl;
    for (int i = 0; i < gTextures.size(); ++i)
    {
        texCoordData[2 * i] = gTextures[i].u;
        texCoordData[2 * i + 1] = gTextures[i].v;

        std::cout << texCoordData[2 * i] << ", " << texCoordData[2 * i + 1] << std::endl;
    }

    for (int i = 0; i < gFaces.size(); ++i)
    {
        indexData[3 * i] = gFaces[i].vIndex[0];
        indexData[3 * i + 1] = gFaces[i].vIndex[1];
        indexData[3 * i + 2] = gFaces[i].vIndex[2];
    }

    int totalSize = gVertexDataSizeInBytes + gNormalDataSizeInBytes + gTexCoordDataSizeInBytes;

    glBufferData(GL_ARRAY_BUFFER, totalSize, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes, vertexData);
    glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes, gNormalDataSizeInBytes, normalData);
    glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes + gNormalDataSizeInBytes, gTexCoordDataSizeInBytes, texCoordData);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

    // done copying; can free now
    delete[] vertexData;
    delete[] normalData;
    delete[] texCoordData;
    delete[] indexData;

    gVertices.clear();
    gTextures.clear();
    gNormals.clear();
    gFacesSize = gFaces.size();
    gFaces.clear();

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes + gNormalDataSizeInBytes));
}

void initQuadVBO()
{
    GLuint quadvao;
    glGenVertexArrays(1, &quadvao);
    assert(quadvao > 0);
    glBindVertexArray(quadvao);
    std::cout << "quadvao = " << quadvao << std::endl;

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &gQuadVertexAttribBuffer);
    glGenBuffers(1, &gQuadIndexBuffer);

    assert(gQuadVertexAttribBuffer > 0 && gQuadIndexBuffer > 0);

    glBindBuffer(GL_ARRAY_BUFFER, gQuadVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gQuadIndexBuffer);

    gQuadVertexDataSizeInBytes = gVertices.size() * 3 * sizeof(GLfloat);
    gQuadNormalDataSizeInBytes = gNormals.size() * 3 * sizeof(GLfloat);
    gQuadTexCoordDataSizeInBytes = gTextures.size() * 2 * sizeof(GLfloat);
    int indexDataSizeInBytes = gFaces.size() * 3 * sizeof(GLuint);
    GLfloat *vertexData = new GLfloat[gVertices.size() * 3];
    GLfloat *normalData = new GLfloat[gNormals.size() * 3];
    GLfloat *texCoordData = new GLfloat[gTextures.size() * 2];
    GLuint *indexData = new GLuint[gFaces.size() * 3];

    for (int i = 0; i < gVertices.size(); ++i)
    {
        vertexData[3 * i] = gVertices[i].x;
        vertexData[3 * i + 1] = gVertices[i].y;
        vertexData[3 * i + 2] = gVertices[i].z;
    }

    for (int i = 0; i < gNormals.size(); ++i)
    {
        normalData[3 * i] = gNormals[i].x;
        normalData[3 * i + 1] = gNormals[i].y;
        normalData[3 * i + 2] = gNormals[i].z;
    }

    std::cout << "texCoordData: " << std::endl;
    for (int i = 0; i < gTextures.size(); ++i)
    {
        texCoordData[2 * i] = gTextures[i].u;
        texCoordData[2 * i + 1] = gTextures[i].v;

        std::cout << texCoordData[2 * i] << ", " << texCoordData[2 * i + 1] << std::endl;
    }

    for (int i = 0; i < gFaces.size(); ++i)
    {
        indexData[3 * i] = gFaces[i].vIndex[0];
        indexData[3 * i + 1] = gFaces[i].vIndex[1];
        indexData[3 * i + 2] = gFaces[i].vIndex[2];
    }

    int totalSize = gQuadVertexDataSizeInBytes + gQuadNormalDataSizeInBytes + gQuadTexCoordDataSizeInBytes;

    glBufferData(GL_ARRAY_BUFFER, totalSize, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gQuadVertexDataSizeInBytes, vertexData);
    glBufferSubData(GL_ARRAY_BUFFER, gQuadVertexDataSizeInBytes, gQuadNormalDataSizeInBytes, normalData);
    glBufferSubData(GL_ARRAY_BUFFER, gQuadVertexDataSizeInBytes + gQuadNormalDataSizeInBytes, gQuadTexCoordDataSizeInBytes, texCoordData);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

    // done copying; can free now
    delete[] vertexData;
    delete[] normalData;
    delete[] texCoordData;
    delete[] indexData;

    gVertices.clear();
    gTextures.clear();
    gNormals.clear();
    quadFacesSize = gFaces.size();
    gFaces.clear();

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gQuadVertexDataSizeInBytes));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gQuadVertexDataSizeInBytes + gQuadNormalDataSizeInBytes));
}

void initSphereVBO()
{
    GLuint spherevao;
    glGenVertexArrays(1, &spherevao);
    assert(spherevao > 0);
    glBindVertexArray(spherevao);
    std::cout << "spherevao = " << spherevao << std::endl;

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &gSphereVertexAttribBuffer);
    glGenBuffers(1, &gSphereIndexBuffer);

    assert(gSphereVertexAttribBuffer > 0 && gSphereIndexBuffer > 0);

    glBindBuffer(GL_ARRAY_BUFFER, gSphereVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gSphereIndexBuffer);

    gSphereVertexDataSizeInBytes = gVertices.size() * 3 * sizeof(GLfloat);
    gSphereNormalDataSizeInBytes = gNormals.size() * 3 * sizeof(GLfloat);
    gSphereTexCoordDataSizeInBytes = gTextures.size() * 2 * sizeof(GLfloat);
    int indexDataSizeInBytes = gFaces.size() * 3 * sizeof(GLuint);
    GLfloat *vertexData = new GLfloat[gVertices.size() * 3];
    GLfloat *normalData = new GLfloat[gNormals.size() * 3];
    GLfloat *texCoordData = new GLfloat[gTextures.size() * 2];
    GLuint *indexData = new GLuint[gFaces.size() * 3];

    for (int i = 0; i < gVertices.size(); ++i)
    {
        vertexData[3 * i] = gVertices[i].x;
        vertexData[3 * i + 1] = gVertices[i].y;
        vertexData[3 * i + 2] = gVertices[i].z;
    }

    for (int i = 0; i < gNormals.size(); ++i)
    {
        normalData[3 * i] = gNormals[i].x;
        normalData[3 * i + 1] = gNormals[i].y;
        normalData[3 * i + 2] = gNormals[i].z;
    }

    std::cout << "texCoordData: " << std::endl;
    for (int i = 0; i < gTextures.size(); ++i)
    {
        texCoordData[2 * i] = gTextures[i].u;
        texCoordData[2 * i + 1] = gTextures[i].v;

        std::cout << texCoordData[2 * i] << ", " << texCoordData[2 * i + 1] << std::endl;
    }

    for (int i = 0; i < gFaces.size(); ++i)
    {
        indexData[3 * i] = gFaces[i].vIndex[0];
        indexData[3 * i + 1] = gFaces[i].vIndex[1];
        indexData[3 * i + 2] = gFaces[i].vIndex[2];
    }

    int totalSize = gSphereVertexDataSizeInBytes + gSphereNormalDataSizeInBytes + gSphereTexCoordDataSizeInBytes;

    glBufferData(GL_ARRAY_BUFFER, totalSize, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gSphereVertexDataSizeInBytes, vertexData);
    glBufferSubData(GL_ARRAY_BUFFER, gSphereVertexDataSizeInBytes, gSphereNormalDataSizeInBytes, normalData);
    glBufferSubData(GL_ARRAY_BUFFER, gSphereVertexDataSizeInBytes + gSphereNormalDataSizeInBytes, gSphereTexCoordDataSizeInBytes, texCoordData);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

    // done copying; can free now
    delete[] vertexData;
    delete[] normalData;
    delete[] texCoordData;
    delete[] indexData;

    gVertices.clear();
    gTextures.clear();
    gNormals.clear();
    sphereFacesSize = gFaces.size();
    gFaces.clear();

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gSphereVertexDataSizeInBytes));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gSphereVertexDataSizeInBytes + gSphereNormalDataSizeInBytes));
}

void initBunnyVBO()
{
    GLuint bunnyvao;
    glGenVertexArrays(1, &bunnyvao);
    assert(bunnyvao > 0);
    glBindVertexArray(bunnyvao);
    std::cout << "bunnyvao = " << bunnyvao << std::endl;

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &gBunnyVertexAttribBuffer);
    glGenBuffers(1, &gBunnyIndexBuffer);

    assert(gBunnyVertexAttribBuffer > 0 && gBunnyIndexBuffer > 0);

    glBindBuffer(GL_ARRAY_BUFFER, gBunnyVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gBunnyIndexBuffer);

    gBunnyVertexDataSizeInBytes = gVertices.size() * 3 * sizeof(GLfloat);
    gBunnyNormalDataSizeInBytes = gNormals.size() * 3 * sizeof(GLfloat);
    gBunnyTexCoordDataSizeInBytes = gTextures.size() * 2 * sizeof(GLfloat);
    int indexDataSizeInBytes = gFaces.size() * 3 * sizeof(GLuint);
    GLfloat *vertexData = new GLfloat[gVertices.size() * 3];
    GLfloat *normalData = new GLfloat[gNormals.size() * 3];
    GLfloat *texCoordData = new GLfloat[gTextures.size() * 2];
    GLuint *indexData = new GLuint[gFaces.size() * 3];

    for (int i = 0; i < gVertices.size(); ++i)
    {
        vertexData[3 * i] = gVertices[i].x;
        vertexData[3 * i + 1] = gVertices[i].y;
        vertexData[3 * i + 2] = gVertices[i].z;
    }

    for (int i = 0; i < gNormals.size(); ++i)
    {
        normalData[3 * i] = gNormals[i].x;
        normalData[3 * i + 1] = gNormals[i].y;
        normalData[3 * i + 2] = gNormals[i].z;
    }

    std::cout << "texCoordData: " << std::endl;
    for (int i = 0; i < gTextures.size(); ++i)
    {
        texCoordData[2 * i] = gTextures[i].u;
        texCoordData[2 * i + 1] = gTextures[i].v;

        std::cout << texCoordData[2 * i] << ", " << texCoordData[2 * i + 1] << std::endl;
    }

    for (int i = 0; i < gFaces.size(); ++i)
    {
        indexData[3 * i] = gFaces[i].vIndex[0];
        indexData[3 * i + 1] = gFaces[i].vIndex[1];
        indexData[3 * i + 2] = gFaces[i].vIndex[2];
    }

    int totalSize = gBunnyVertexDataSizeInBytes + gBunnyNormalDataSizeInBytes + gBunnyTexCoordDataSizeInBytes;

    glBufferData(GL_ARRAY_BUFFER, totalSize, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gBunnyVertexDataSizeInBytes, vertexData);
    glBufferSubData(GL_ARRAY_BUFFER, gBunnyVertexDataSizeInBytes, gBunnyNormalDataSizeInBytes, normalData);
    glBufferSubData(GL_ARRAY_BUFFER, gBunnyVertexDataSizeInBytes + gBunnyNormalDataSizeInBytes, gBunnyTexCoordDataSizeInBytes, texCoordData);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

    // done copying; can free now
    delete[] vertexData;
    delete[] normalData;
    delete[] texCoordData;
    delete[] indexData;

    gVertices.clear();
    gTextures.clear();
    gNormals.clear();
    bunnyFacesSize = gFaces.size();
    gFaces.clear();

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gBunnyVertexDataSizeInBytes));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gBunnyVertexDataSizeInBytes + gBunnyNormalDataSizeInBytes));
}

void initTeapotVBO()
{
    GLuint teapotvao;
    glGenVertexArrays(1, &teapotvao);
    assert(teapotvao > 0);
    glBindVertexArray(teapotvao);
    std::cout << "teapotvao = " << teapotvao << std::endl;

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &gTeapotVertexAttribBuffer);
    glGenBuffers(1, &gTeapotIndexBuffer);

    assert(gTeapotVertexAttribBuffer > 0 && gTeapotIndexBuffer > 0);

    glBindBuffer(GL_ARRAY_BUFFER, gTeapotVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gTeapotIndexBuffer);

    gTeapotVertexDataSizeInBytes = gVertices.size() * 3 * sizeof(GLfloat);
    gTeapotNormalDataSizeInBytes = gNormals.size() * 3 * sizeof(GLfloat);
    gTeapotTexCoordDataSizeInBytes = gTextures.size() * 2 * sizeof(GLfloat);
    int indexDataSizeInBytes = gFaces.size() * 3 * sizeof(GLuint);
    GLfloat *vertexData = new GLfloat[gVertices.size() * 3];
    GLfloat *normalData = new GLfloat[gNormals.size() * 3];
    GLfloat *texCoordData = new GLfloat[gTextures.size() * 2];
    GLuint *indexData = new GLuint[gFaces.size() * 3];

    for (int i = 0; i < gVertices.size(); ++i)
    {
        vertexData[3 * i] = gVertices[i].x;
        vertexData[3 * i + 1] = gVertices[i].y;
        vertexData[3 * i + 2] = gVertices[i].z;
    }

    for (int i = 0; i < gNormals.size(); ++i)
    {
        normalData[3 * i] = gNormals[i].x;
        normalData[3 * i + 1] = gNormals[i].y;
        normalData[3 * i + 2] = gNormals[i].z;
    }

    std::cout << "texCoordData: " << std::endl;
    for (int i = 0; i < gTextures.size(); ++i)
    {
        texCoordData[2 * i] = gTextures[i].u;
        texCoordData[2 * i + 1] = gTextures[i].v;

        std::cout << texCoordData[2 * i] << ", " << texCoordData[2 * i + 1] << std::endl;
    }

    for (int i = 0; i < gFaces.size(); ++i)
    {
        indexData[3 * i] = gFaces[i].vIndex[0];
        indexData[3 * i + 1] = gFaces[i].vIndex[1];
        indexData[3 * i + 2] = gFaces[i].vIndex[2];
    }

    int totalSize = gTeapotVertexDataSizeInBytes + gTeapotNormalDataSizeInBytes + gTeapotTexCoordDataSizeInBytes;

    glBufferData(GL_ARRAY_BUFFER, totalSize, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gTeapotVertexDataSizeInBytes, vertexData);
    glBufferSubData(GL_ARRAY_BUFFER, gTeapotVertexDataSizeInBytes, gTeapotNormalDataSizeInBytes, normalData);
    glBufferSubData(GL_ARRAY_BUFFER, gTeapotVertexDataSizeInBytes + gTeapotNormalDataSizeInBytes, gTeapotTexCoordDataSizeInBytes, texCoordData);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

    // done copying; can free now
    delete[] vertexData;
    delete[] normalData;
    delete[] texCoordData;
    delete[] indexData;

    gVertices.clear();
    gTextures.clear();
    gNormals.clear();
    teapotFacesSize = gFaces.size();
    gFaces.clear();

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gTeapotVertexDataSizeInBytes));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gTeapotVertexDataSizeInBytes + gTeapotNormalDataSizeInBytes));
}


void initGBuffer()
{
    // Create and bind the framebuffer
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // Position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gWidth, gHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // Normal buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gWidth, gHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    // Color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gWidth, gHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

    // Depth buffer
    glGenTextures(1, &gTexDepth);
    glBindTexture(GL_TEXTURE_2D, gTexDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, gWidth, gHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gTexDepth, 0);

    // Specify the color attachments for rendering
    unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Framebuffer not complete!" << std::endl;

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void generateTexture()
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    int width, height, nrChannels;
    unsigned char *data = stbi_load("shrek.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // solved the skewing of texture problem
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
}

void fwdDrawTeapot(glm::mat4 model, glm::vec3 color) {
    glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(objColorLoc2, 1, glm::value_ptr(color));

    glBindBuffer(GL_ARRAY_BUFFER, gTeapotVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gTeapotIndexBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gTeapotVertexDataSizeInBytes));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gTeapotVertexDataSizeInBytes + gTeapotNormalDataSizeInBytes));

    glDrawElements(GL_TRIANGLES, teapotFacesSize * 3, GL_UNSIGNED_INT, 0);
}

void fwdDrawBunny(glm::mat4 model, glm::vec3 color) {
    model = glm::translate(model, glm::vec3(0.0f, 0.35f, 0.0f));

    glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(objColorLoc2, 1, glm::value_ptr(color));

    glBindBuffer(GL_ARRAY_BUFFER, gBunnyVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gBunnyIndexBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gBunnyVertexDataSizeInBytes));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gBunnyVertexDataSizeInBytes + gBunnyNormalDataSizeInBytes));

    glDrawElements(GL_TRIANGLES, bunnyFacesSize * 3, GL_UNSIGNED_INT, 0);
}

void fwdDrawArmadillo(glm::mat4 model, glm::vec3 color) {
    model = glm::translate(model, glm::vec3(0.0f, 0.4f, 0.0f));

    glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(objColorLoc2, 1, glm::value_ptr(color));

    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes + gNormalDataSizeInBytes));

    glDrawElements(GL_TRIANGLES, gFacesSize * 3, GL_UNSIGNED_INT, 0);
}

void fwdDrawObjects() {

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(20.0f, 0.0f, -20.0f));

    GLfloat x, z, displacement;
    displacement = quadWidth / (numOfObjRows);
    x = 20.0f;
    z = 20.0f - displacement;

    for (int i = 0; i < numOfObjects; i++) {
        int objID = objects[i];
        glm::vec3 color = Colors[i];

        x -= displacement;

        if (x <= -20.0f) {
            x = 20.0f - displacement;
            z -= displacement;
            model = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, z));
        }
        else model = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, z));

        switch (i % 3) {
            case 0: fwdDrawTeapot(model, color); break;
            case 1: fwdDrawBunny(model, color); break;
            case 2: fwdDrawArmadillo(model, color); break;
            default: fwdDrawBunny(model, color); break;
        }

    }
}

void fwdDisplay()
{

    // define model view and projection matrices for the object
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = viewingMatrix;
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    // set shader uniforms
    glUseProgram(gProgram[1]);
    glUniformMatrix4fv(viewLoc2, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectLoc2, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(eyePosLoc2, 1, glm::value_ptr(cameraPosition));
    glUniform3fv(lightPos2, 20, value_ptr(lightPositions[0]));

    // bind object vbo and draw
    fwdDrawObjects();

    // draw ground surface

    model = glm::mat4(1.0f); // Initialize to identity matrix
    model = glm::translate(model, glm::vec3(0.0f, -0.65f, 0.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.0f));

    glm::vec3 groundColor(1.0f, 1.0f, 1.0f);
    
    glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(objColorLoc2, 1, glm::value_ptr(groundColor));

    glBindBuffer(GL_ARRAY_BUFFER, gQuadVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gQuadIndexBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gQuadVertexDataSizeInBytes));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gQuadVertexDataSizeInBytes + gQuadNormalDataSizeInBytes));

    glDrawElements(GL_TRIANGLES, quadFacesSize * 3, GL_UNSIGNED_INT, 0);
}

void generateColors() {
    srand(time(NULL));
    glm::vec3 color;

    for (int i = 0; i < numOfObjects; i++) {
        color.x = rand() % 10;
        color.y = rand() % 10;
        color.z = rand() % 10;

        color = color * glm::vec3(0.1,0.1,0.1);
        Colors.push_back(color);
    }

}

void drawTeapot(glm::mat4 model, glm::vec3 color) {
    glUniformMatrix4fv(modelLoc1, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(colorLoc1, 1, glm::value_ptr(color));

    glBindBuffer(GL_ARRAY_BUFFER, gTeapotVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gTeapotIndexBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gTeapotVertexDataSizeInBytes));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gTeapotVertexDataSizeInBytes + gTeapotNormalDataSizeInBytes));

    glDrawElements(GL_TRIANGLES, teapotFacesSize * 3, GL_UNSIGNED_INT, 0);
}

void drawBunny(glm::mat4 model, glm::vec3 color) {
    model = glm::translate(model, glm::vec3(0.0f, 0.35f, 0.0f));

    glUniformMatrix4fv(modelLoc1, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(colorLoc1, 1, glm::value_ptr(color));

    glBindBuffer(GL_ARRAY_BUFFER, gBunnyVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gBunnyIndexBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gBunnyVertexDataSizeInBytes));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gBunnyVertexDataSizeInBytes + gBunnyNormalDataSizeInBytes));

    glDrawElements(GL_TRIANGLES, bunnyFacesSize * 3, GL_UNSIGNED_INT, 0);
}

void drawArmadillo(glm::mat4 model, glm::vec3 color) {
    model = glm::translate(model, glm::vec3(0.0f, 0.4f, 0.0f));

    glUniformMatrix4fv(modelLoc1, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(colorLoc1, 1, glm::value_ptr(color));

    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes + gNormalDataSizeInBytes));

    glDrawElements(GL_TRIANGLES, gFacesSize * 3, GL_UNSIGNED_INT, 0);
}

void drawObjects() {

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(20.0f, 0.0f, -20.0f));

    GLfloat x, z, displacement;
    displacement = quadWidth / (numOfObjRows);
    x = 20.0f;
    z = 20.0f - displacement;

    for (int i = 0; i < numOfObjects; i++) {
        int objID = objects[i];
        glm::vec3 color = Colors[i];

        x -= displacement;

        if (x <= -20.0f) {
            x = 20.0f - displacement;
            z -= displacement;
            model = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, z));
        }
        else model = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, z));

        switch (i % 3) {
            case 0: drawTeapot(model, color); break;
            case 1: drawBunny(model, color); break;
            case 2: drawArmadillo(model, color); break;
            default: drawBunny(model, color); break;
        }

    }
}

void writeToGBuffer()
{
    // bind gBuffer
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // define model view and projection matrices for the object
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = viewingMatrix;
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    float spec1 = 50.0f;

    // set shader uniforms
    glUseProgram(gBufferShader);
    glUniformMatrix4fv(viewLoc1, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc1, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1f(specularLoc1, spec1);

    // bind object vbo and draw
    drawObjects();

    // draw ground surface

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -0.65f, 0.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.0f));

    glm::vec3 groundColor(1.0f, 1.0f, 1.0f);
    
    glUniformMatrix4fv(modelLoc1, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(colorLoc1, 1, glm::value_ptr(groundColor));

    glBindBuffer(GL_ARRAY_BUFFER, gQuadVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gQuadIndexBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gQuadVertexDataSizeInBytes));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gQuadVertexDataSizeInBytes + gQuadNormalDataSizeInBytes));

    glDrawElements(GL_TRIANGLES, quadFacesSize * 3, GL_UNSIGNED_INT, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void displayTexture(unsigned int texName)
{
    // quad shader uniforms
    glUseProgram(quadshader);

    glm::mat4 model = glm::mat4(1.0f); // No transformation for quad
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    glUniformMatrix4fv(quadModelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(quadViewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(quadProjectLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texName); // CHANGE TEXTURE NAME TO VIEW DESIRED G-BUFFER TEXTURE

    glUniform1i(quadTexLoc, 0);

    glBindBuffer(GL_ARRAY_BUFFER, gQuadVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gQuadIndexBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gQuadVertexDataSizeInBytes));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gQuadVertexDataSizeInBytes + gQuadNormalDataSizeInBytes));

    glDrawElements(GL_TRIANGLES, quadFacesSize * 3, GL_UNSIGNED_INT, 0);
}

void lightingPass()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

    // light relevant uniforms
    glUseProgram(lightingshader);
    glUniform3fv(lightLocation, 20, value_ptr(lightPositions[0]));
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPosition));
    glUniform1i(gPositionLoc, 0);
    glUniform1i(gNormalLoc, 1);
    glUniform1i(gAlbedoLoc, 2);

    // Render Quad
    glBindBuffer(GL_ARRAY_BUFFER, gQuadVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gQuadIndexBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gQuadVertexDataSizeInBytes));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gQuadVertexDataSizeInBytes + gQuadNormalDataSizeInBytes));

    glDrawElements(GL_TRIANGLES, quadFacesSize * 3, GL_UNSIGNED_INT, 0);
}

void depthOfFieldPass() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gTexDepth);

    glUseProgram(gProgram[0]);
    glUniform1i(blurColorLoc, 0);
    glUniform1i(blurDepthLoc, 1);
    // Render Quad
    glBindBuffer(GL_ARRAY_BUFFER, gQuadVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gQuadIndexBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gQuadVertexDataSizeInBytes));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gQuadVertexDataSizeInBytes + gQuadNormalDataSizeInBytes));

    glDrawElements(GL_TRIANGLES, quadFacesSize * 3, GL_UNSIGNED_INT, 0);
}

void displayDepthBuff()
{
    // quad shader uniforms
    glUseProgram(depthshader);

    glm::mat4 model = glm::mat4(1.0f); // No transformation for quad
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    glUniformMatrix4fv(depthModelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(depthViewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(depthProjectLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTexDepth);

    glUniform1i(depthTexLoc, 0);

    glBindBuffer(GL_ARRAY_BUFFER, gQuadVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gQuadIndexBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gQuadVertexDataSizeInBytes));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gQuadVertexDataSizeInBytes + gQuadNormalDataSizeInBytes));

    glDrawElements(GL_TRIANGLES, quadFacesSize * 3, GL_UNSIGNED_INT, 0);
}

void framebuffer_size_callback(GLFWwindow *window, int w, int h)
{

    w = w < 1 ? 1 : w;
    h = h < 1 ? 1 : h;

    gWidth = w;
    gHeight = h;

    glViewport(0, 0, w, h);

    // Use perspective projection

    float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
    float fovy = 90.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    if (aspectRatio > 1.0f)
    {
        // If width is larger than height, decrease the vertical FOV
        fovy = atan(tan(fovy * (M_PI / 180.0f)) / aspectRatio) * (180.0f / M_PI);
    }

    projectionMatrix = glm::perspective(glm::radians(fovy), aspectRatio, nearPlane, farPlane);

    // Assume default camera position and orientation (camera is at
    // (0, 0, 0) with looking at -z direction and its up vector pointing
    // at +y direction)

    viewingMatrix = glm::mat4(1);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    static float sensitivity = 0.1f;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        yaw -= xOffset * sensitivity;
        pitch -= yOffset * sensitivity;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }
}

void init()
{
    initShaders();

    ParseObj("armadillo.obj");
    initVBO();

    ParseObj("teapot.obj");
    initTeapotVBO();

    ParseObj("sphere.obj");
    initSphereVBO();

    ParseObj("bunny.obj");
    initBunnyVBO();

    ParseObj("quad.obj");
    initQuadVBO();

    generateTexture();

    initGBuffer();

    generateColors();
}

void updateCamera()
{
    glm::vec3 direction;
    direction.x = radius * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    direction.y = radius * sin(glm::radians(pitch));
    direction.z = radius * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    cameraPosition = cameraTarget - direction;

    // Update view matrix
    viewingMatrix = glm::lookAt(cameraPosition, cameraTarget, upVector);
}

void showFPS(GLFWwindow* window) {

    static double startTime = glfwGetTime();
    static int frameCount = 0;
    double currentTime = glfwGetTime();
    double elapsedSeconds = currentTime - startTime;

    // display fps every 1 second
    if (elapsedSeconds > 1.0) {
        double averageFPS = static_cast<double>(frameCount) / elapsedSeconds;

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << "Average FPS: " << averageFPS;
        std::string fpsString = oss.str();
        glfwSetWindowTitle(window, fpsString.c_str());

        startTime = currentTime;
        frameCount = 0;
    }

    frameCount++;
}

void rotateLights(float angle) {
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    for (int i = 0; i < 20; ++i) {
        glm::vec4 position = glm::vec4(baseLightPositions[i], 1.0f);
        position = rotationMatrix * position;
        lightPositions[i] = glm::vec3(position);
    }
}

int main()
{
    // init window and set version to be 3.3 core
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glViewport(0, 0, 800, 600);

    glfwGetFramebufferSize(window, &gWidth, &gHeight);

    init();

    glViewport(0, 0, gWidth, gHeight);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // enable depth test so that we use the z-buffer
    glEnable(GL_DEPTH_TEST);

    float rotationAngle = 0.0f;
    const float rotationSpeed = 10.0f;

    // Timing variables
    auto lastTime = std::chrono::high_resolution_clock::now();

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsedTime = currentTime - lastTime;
        lastTime = currentTime;

        rotationAngle += rotationSpeed * elapsedTime.count();
        if (rotationAngle > 360.0f) rotationAngle -= 360.0f;

        rotateLights(rotationAngle);

        updateCamera();

        // HAVE ONLY ONE BLOCK UNCOMMENTED AT A TIME

        // *** UNCOMMENT TO USE DEFERRED SHADING
        writeToGBuffer();
        lightingPass();
        // ***

        // *** UNCOMMENT TO ADD DEPTH TO DEFERRED SHADING: NOT FUNCTIONAL!!!
        //writeToGBuffer();
        //lightingPass();
        //depthOfFieldPass();
        // ***

        // *** UNCOMMENT TO SEPERATELY RENDER G-BUFFER CONTENT
        //writeToGBuffer();
        //displayTexture(gAlbedoSpec); // VIEW ANY OF: gPosition, gNormal, gAlbedoSpec
        // ***

        // *** UNCOMMENT TO RENDER G-BUFFER DEPTH TEXTURE
        //writeToGBuffer();
        //displayDepthBuff();
        // ***

        // *** UNCOMMENT TO USE FORWARD RENDERING
        //fwdDisplay();
        // ***

        glfwSwapBuffers(window);
        glfwPollEvents();

        showFPS(window);
    }

    // clean up
    glDeleteProgram(gProgram[0]);

    return 0;
}
