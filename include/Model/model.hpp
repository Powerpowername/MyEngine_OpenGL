#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "mesh.hpp"
#include "shader.h"
#include "MyEngine.h"
#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>
#include "QuaternionTransForm.hpp"
#include <deque>
using std::string,std::unordered_map,std::vector;
static int ModelNum;
class Model
{
public:
    
    std::unique_ptr<QuaternionTransForm> transform = nullptr;
    vector<Texture> textures_loaded;
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection;
    Model(string const &path,bool gamma);
    void SetShader(Object* viewObject);
    void Render();
    void OnGUI()
    {
        transform->OnGUI();
    }

    void Update()
    {
        transform->Update();
    }
private:
    
    int ID = -1;
    Shader shader;

    void loadModel(string const &path);
    void processNode(aiNode* node,const aiScene* scence);//递归实现
    void processNodeQueueManus(aiNode* node,const aiScene* scence);//层次遍历实现

    Mesh processMesh(aiMesh* mesh,const aiScene* scene);
    vector<Texture> loadMaterialTextures(aiMaterial* mat,aiTextureType type,string typeName);
    unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);


};
inline Model::Model(string const &path,bool gamma =false) :gammaCorrection(gamma),shader("../../shader/modelShader/modelShader.vs","../../shader/modelShader/modelShader.fs")
{
    transform = std::make_unique<QuaternionTransForm>();
    ID++;
    transform->name = "Model" + std::to_string(ID);
    transform->moveObject = QuaternionTransForm::MoveObject::RENDEROBJECT;
    loadModel(path);
}

inline void Model::SetShader(Object* viewObject)
{
    shader.use();
    QuaternionTransForm* viewTransform = dynamic_cast<QuaternionTransForm*>(viewObject);
    shader.setMat4("projection",viewTransform->GetProjectionMatrixTransposed());
    shader.setMat4("view", viewTransform->GetViewMatrixTransposed());
    shader.setMat4("model", transform->GetModeMatrixTransposed());


}

inline void Model::Render()
{
    for(unsigned int i = 0;i < meshes.size();i++)
        meshes[i].Draw(shader);
}

inline void Model::loadModel(string const &path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }

    directory = path.substr(0,path.find_last_of('/'));
    // processNode(scene->mRootNode,scene);//后期修改为非递归版的，提高效率
    processNodeQueueManus(scene->mRootNode,scene);//采用层次遍历实现


}

inline void Model::processNode(aiNode* node,const aiScene* scence)
{
    for(unsigned int i = 0;i < node->mNumMeshes;i++)
    {
        aiMesh* mesh = scence->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scence));//后续考虑使用移动构造，或者定制emplace_back()
    }
    for(unsigned int i = 0;i < node->mNumChildren;i++)
    {
        processNode(node->mChildren[i],scence);
    }
}

inline Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    for(unsigned int i = 0;i < mesh->mNumVertices;i++)
    {
        Vertex vertex;
        glm::vec3 vector;
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        // normals
        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }
        else
        {
            vector = glm::vec3(1.0f,1.0f,1.0f);
            vertex.Normal = vector;
            std::cout<<"Do not have normal texture!"<<std::endl;
        }
        // texture coordinates
        // texture coordinates
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
        }
        else
        {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            std::cout<<"Do not have texture coordinates!"<<std::endl;
        }
        vertices.push_back(vertex);
    }
    for(unsigned int i = 0;i < mesh->mNumFaces;i++)//此处的一个面就是Mesh中的一个三角形面
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0;j < face.mNumIndices;j++)
            indices.push_back(face.mIndices[j]);
    }

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    // 1. diffuse maps
    vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    // 2. specular maps
    vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. normal maps
    std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    // textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    // 4. height maps
    std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    // textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    


    return Mesh(vertices, indices, textures);
}

inline vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
{
    vector<Texture> textures;
    for(unsigned int i = 0;i < mat->GetTextureCount(type);i++)
    {
        aiString str;
        mat->GetTexture(type,i,&str);//应该是纹理的路径(相对路径)
        bool skip = false;
        for(unsigned int j = 0;j < textures_loaded.size();j++)
        {
            //是否是已近保存的
            if(std::strcmp(textures_loaded[j].path.data(),str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if(!skip)
        {
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(),this->directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }
    return textures;
}

inline unsigned int Model::TextureFromFile(const char *path, const string &directory, bool gamma)
{
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    //这边需要翻转一下才可以正确加载纹理，不然显示的图像是不正确的
    stbi_set_flip_vertically_on_load(1); 
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

inline void Model::processNodeQueueManus(aiNode *node, const aiScene *scence)
{
    std::deque<aiNode*> nodeQueue;
    nodeQueue.push_back(node);
    while(!nodeQueue.empty())
    {
        aiNode* temp = nodeQueue.front();
        nodeQueue.pop_front();
        for (unsigned int i = 0; i < temp->mNumChildren; i++)
        {
            nodeQueue.push_back(temp->mChildren[i]);
        }

        for (unsigned int i = 0; i < temp->mNumMeshes; i++)
        {
            aiMesh *mesh = scence->mMeshes[temp->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scence)); // 后续考虑使用移动构造，或者定制emplace_back()
        }
    }
}
