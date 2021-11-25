#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include "Mesh.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class Model
{
    public:
        std::vector<Texture> textures_loaded;
        std::vector<Mesh> meshes;

        Model() 
        {
            //code
        }

        ~Model()
        {
            for(int i = 0; i < textures_loaded.size(); i++)
            {
                if(textures_loaded[i].id)
                {
                    glDeleteTextures(1, &(textures_loaded[i].id));
                    textures_loaded[i].id = 0;
                }
            }
        }

        void LoadModel(std::string const &path)
        {
            Assimp::Importer importer;
            const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
            if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            {
                MessageBox(NULL, TEXT("Error"), TEXT("Assimp failed to load model"), MB_OK);
                return;
            }

            ProcessNode(scene->mRootNode, scene); 
        } 

        void Draw(unsigned int shaderProgramObject)
        {
            for(int i = 0; i < meshes.size(); i++)
            {
                meshes[i].Draw(shaderProgramObject);
            }
        }

    private:
        void ProcessNode(aiNode *node, const aiScene *scene)
        {
            for(int i = 0; i < node->mNumMeshes; i++)
            {
                aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
                meshes.push_back(ProcessMesh(mesh, scene));
            }

            for(int i = 0; i < node->mNumChildren; i++)
            {
                ProcessNode(node->mChildren[i], scene);
            }
        }

        Mesh ProcessMesh(aiMesh *mesh, const aiScene *scene)
        {
            std::vector<Vertex> vertices;
            std::vector<GLuint> indices;
            std::vector<Texture> textures;
            Material temp_material;

            for(int i = 0; i < mesh->mNumVertices; i++)
            {
                Vertex vertex;
                vmath::vec3 vec;

                if(mesh->HasPositions())
                {
                    vec[0] = mesh->mVertices[i].x;
                    vec[1] = mesh->mVertices[i].y;
                    vec[2] = mesh->mVertices[i].z;
                    vertex.Position = vec;
                }

                if(mesh->HasNormals())
                {
                    vec[0] = mesh->mNormals[i].x;
                    vec[1] = mesh->mNormals[i].y;
                    vec[2] = mesh->mNormals[i].z;
                    vertex.Normal = vec;
                }
                
                if(mesh->HasTextureCoords(0))
                {
                    vec[0] = mesh->mTextureCoords[0][i].x;
                    vec[1] = mesh->mTextureCoords[0][i].y;
                    vertex.Texcoord = vmath::vec2(vec[0], vec[1]);
                }
                else
                {
                    vertex.Texcoord = vmath::vec2(0.0f, 0.0f);
                }

                if(mesh->HasTangentsAndBitangents())
                {
                    vec[0] = mesh->mTangents[i].x;
                    vec[1] = mesh->mTangents[i].y;
                    vec[2] = mesh->mTangents[i].z;
                    vertex.Tangent = vec;

                    vec[0] = mesh->mBitangents[i].x;
                    vec[1] = mesh->mBitangents[i].y;
                    vec[2] = mesh->mBitangents[i].z;
                    vertex.Bitangent = vec;
                }

                vertices.push_back(vertex);
            } 

            for(int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                for(int j = 0; j < face.mNumIndices; j++)
                {
                    indices.push_back(face.mIndices[j]);
                }
            }

            aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

            //get material properties
            aiColor3D color(0.0f, 0.0f, 0.0f);
            material->Get(AI_MATKEY_COLOR_AMBIENT, color);
            temp_material.ambient_color[0] = color[0];
            temp_material.ambient_color[1] = color[1];
            temp_material.ambient_color[2] = color[2];

            material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
            temp_material.diffuse_color[0] = color[0];
            temp_material.diffuse_color[1] = color[1];
            temp_material.diffuse_color[2] = color[2];

            material->Get(AI_MATKEY_COLOR_SPECULAR, color);
            temp_material.specular_color[0] = color[0];
            temp_material.specular_color[1] = color[1];
            temp_material.specular_color[2] = color[2];
            
            float shininess = 0.0f;
            material->Get(AI_MATKEY_SHININESS, shininess);
            temp_material.shininess = shininess;

            //get textures
            std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "diffuse_texture");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

            std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "specular_texture");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        
            std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "normal_texture");
            textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

            std::vector<Texture> ambientMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "ambient_texture");
            textures.insert(textures.end(), ambientMaps.begin(), ambientMaps.end());

            return Mesh(vertices, indices, textures, temp_material);
        }

        std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
        {
            std::vector<Texture> textures;
            for(int i = 0; i < mat->GetTextureCount(type); i++)
            {
                aiString str;
                mat->GetTexture(type, i, &str);
        
                bool skip = false;
                for(int j = 0; j < textures_loaded.size(); j++)
                {
                    if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                    {
                        textures.push_back(textures_loaded[j]);
                        skip = true;
                        break;
                    }
                }

                if(!skip)
                {
                    Texture texture;
                    texture.id = load_texture(str.C_Str());
                    texture.path = str.C_Str();
                    texture.type = typeName;
                    textures.push_back(texture);
                    textures_loaded.push_back(texture);
                }
            }

            return (textures);
        }

        unsigned int load_texture(const char *filename)
        {
            //variable declarations
            unsigned int texture;
            unsigned char *pixel_data = NULL;
            int width, height, nrComponents;
            GLenum format;

            //code
            char filepath[128];
            strcpy(filepath, "textures\\");
            strcat(filepath, filename);

            pixel_data = stbi_load(filepath, &width, &height, &nrComponents, 0);
            if(pixel_data == NULL)
            {
                TCHAR str[255];
                wsprintf(str, TEXT("stbi_load failed to load texture %s"), filename);
                MessageBox(NULL, str, TEXT("Error"), MB_OK);
                return (0);
            }

            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            //set up texture parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

            //push the data to texture memory
            glTexImage2D(GL_TEXTURE_2D, 0, format, (GLint)width, (GLint)height, 0, format, GL_UNSIGNED_BYTE, (const void*)pixel_data);
            glGenerateMipmap(GL_TEXTURE_2D);

            stbi_image_free(pixel_data);
            pixel_data = NULL;

            return (texture);
        }
};

