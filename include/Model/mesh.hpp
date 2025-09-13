#pragma once
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "shader.h"
#include <string>
#include <vector>
using std::string,std::vector;
#define MAX_BONE_INFLUENCE 4
struct Vertex{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    float m_Weights[MAX_BONE_INFLUENCE];
};
struct Texture
{ 
    unsigned int id;
    string type;
    string path;    
};

class Mesh
{
public:
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    unsigned int VAO;
    Mesh(vector<Vertex>& vertices,vector<unsigned int>& indices,vector<Texture>& textures); 

    void Draw(Shader& shader);
private:
    unsigned int VBO,IBO; 
    void setupMesh();
};
inline Mesh::Mesh(vector<Vertex>& vertices,vector<unsigned int>& indices,vector<Texture>& textures)
{
    // this->vertices = vertices;

    this->vertices = std::move(vertices);
    this->indices = std::move(indices);
    this->textures = std::move(textures);
    //先不优化，和后期优化做对比
    // this->vertices = vertices;
    // this->indices = indices;
    // this->textures = textures;
    setupMesh();

}
inline void Mesh::Draw(Shader& shader)
{
    unsigned int diffuseNr = 1;//漫反射纹理数量
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;//可能是陡峭视差贴图
    for(unsigned int i = 0;i < textures.size();i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);//激活纹理单元,即纹理插槽
        string number;
        string name = textures[i].type;
        if (name == "texture_diffuse")
        {
            number = std::to_string(diffuseNr++);
        }
        else if (name == "texture_specular")
        {
            number = std::to_string(specularNr++);
        }
        else if (name == "texture_normal")
        {
            number = std::to_string(normalNr++);
        } 
        else if (name == "texture_height")
        {
            number = std::to_string(heightNr++);
        }                                
        shader.setInt(name + number, i); // 告诉采样器从哪一个纹理单元采样
        glBindTexture(GL_TEXTURE_2D,textures[i].id);//说明为纹理格式为GL_TEXTURE_2D，绑定到当前激活的纹理插槽上
    }
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES,static_cast<unsigned int>(indices.size()),GL_UNSIGNED_INT,0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);//恢复纹理状态
}

inline void Mesh::setupMesh()
{
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glGenBuffers(1,&IBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,vertices.size() * sizeof(Vertex),&vertices[0],GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices.size() * sizeof(unsigned int),&indices[0],GL_STATIC_DRAW);

    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Bitangent));
    // ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void *)offsetof(Vertex, m_BoneIDs));

    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, m_Weights));

    //恢复绑定点
    glBindVertexArray(0);
}