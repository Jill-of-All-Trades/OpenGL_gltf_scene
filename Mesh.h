#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>

#include "Shader.h"

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;		// ����������� ������
	glm::vec3 Bitangent;	// ������ ��������� (������, ���������������� ������������ ������� � ������� �������)
};

struct Texture {
	unsigned int id;
	std::string type;
	std::string path;
};

class Mesh
{
public:
	// mesh-������
	std::vector<Vertex>			vertices;
	std::vector<unsigned int>	indices;
	std::vector<Texture>		textures;
	unsigned int VAO;

	Mesh(std::vector<Vertex> verties, std::vector<unsigned int> indices, std::vector<Texture> textures);
	void Draw(Shader shader);

private:
	// ������ ����������
	unsigned int VBO, EBO;

	void setupMesh();
};

