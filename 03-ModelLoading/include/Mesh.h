#pragma once 
#include <vector>
#include <string>
#include <gl/glew.h>
#include <gl/GL.h>
#include "vmath.h"

struct Vertex
{
    vmath::vec3 Position;
    vmath::vec3 Normal;
    vmath::vec2 Texcoord;
    vmath::vec3 Tangent;
    vmath::vec3 Bitangent;
};

struct Texture
{
    unsigned int id;
    std::string type;
    std::string path;
};

struct Material
{
    vmath::vec3 ambient_color;
    vmath::vec3 diffuse_color;
    vmath::vec3 specular_color;
    float shininess;
};

class Mesh
{
    public:
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;
        std::vector<Texture> textures;
        Material material;
        GLuint vao;

        Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures, Material material)
        {
            this->vertices = vertices;
            this->indices = indices;
            this->textures = textures;

            this->material.ambient_color = material.ambient_color; 
            this->material.diffuse_color = material.diffuse_color; 
            this->material.specular_color = material.specular_color; 
            this->material.shininess = material.shininess; 

            SetupMesh();
        }

        void Draw(unsigned int shaderProgramObject)
        {
            for(int i = 0; i < textures.size(); i++)
            {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, textures[i].id);
                glUniform1i(glGetUniformLocation(shaderProgramObject, textures[i].type.c_str()), i);
            }

            glUniform3fv(glGetUniformLocation(shaderProgramObject, "u_matAmbient"), 1, material.ambient_color);
            glUniform3fv(glGetUniformLocation(shaderProgramObject, "u_matDiffuse"), 1, material.diffuse_color);
            glUniform3fv(glGetUniformLocation(shaderProgramObject, "u_matSpecular"), 1, material.specular_color);
            glUniform1f(glGetUniformLocation(shaderProgramObject, "u_matShininess"), material.shininess);

            glBindVertexArray(vao);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

    private:
        GLuint vbo;
        GLuint ebo;

        void SetupMesh()  
        {
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
                glGenBuffers(1, &vbo);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

                    //position
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
                    glEnableVertexAttribArray(0);
                    //normal
                    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
                    glEnableVertexAttribArray(1);
                    //texcoord
                    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Texcoord));
                    glEnableVertexAttribArray(2);
                    //tangent
                    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
                    glEnableVertexAttribArray(3);
                    //bitangent
                    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
                    glEnableVertexAttribArray(4);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glGenBuffers(1, &ebo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
};

