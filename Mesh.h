#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>

#include "Shader.h"

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;		// Касательный вектор
	glm::vec3 Bitangent;	// Вектор бинормали (вектор, перпендикулярный касательному вектору и вектору нормали)
};

struct Texture {
	unsigned int id;
	std::string type;
	std::string path;
};

class Mesh
{
public:
	// mesh-данные
	std::vector<Vertex>			vertices;
	std::vector<unsigned int>	indices;
	std::vector<Texture>		textures;
	unsigned int VAO;

	Mesh(std::vector<Vertex> verties, std::vector<unsigned int> indices, std::vector<Texture> textures);
	void Draw(Shader shader);

private:
	// Данные рендеринга
	unsigned int VBO, EBO;

	void setupMesh();
};

