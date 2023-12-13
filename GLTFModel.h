#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "tiny_gltf.h"

#include <glm/glm.hpp>

#include "Shader.h"


class GLTFModel
{
public:
	GLTFModel();
	GLTFModel(tinygltf::Model& m, glm::vec3 pos, glm::vec3 rot, glm::vec3 scl);
	~GLTFModel();

	bool load(const char* filename);

	// Getters
	tinygltf::Model* getModel() const;
	glm::mat4 getWorld() const;
	glm::vec3 getPosition() const;
	glm::vec3 getRotation() const;
	glm::vec3 getScale() const;

	// Setters
	void setModel(tinygltf::Model& m);
	void setPosition(double x, double y, double z);
	void setRotation(double xr, double yr, double zr);
	void setScale(double xs, double ys, double zs);
	
	// In-scene
	void bind();
	void unbind();
	void draw(Shader& shader);

private:
	void generateBuffers();
	void generateTextures();

	void traverseNode(tinygltf::Node& node, glm::mat4 wrld);
	void bindMesh(tinygltf::Mesh& mesh);

	void drawNode(Shader& shader, tinygltf::Node& node);
	void drawMesh(Shader& shader, tinygltf::Mesh& mesh);

	void proccessMaterial(Shader& shader, tinygltf::Primitive& primitive);
	glm::mat4 getMeshWorld(int mesh_index);

private:
	std::map<int, GLuint> buffer_objects; // vbo & ebo map

	std::vector<GLuint> textures;
	std::vector<GLuint> meshes_vaos;
	std::vector<glm::mat4> meshes_world;

	bool generate_mipmaps = false; // sometimes it requires a lot of time
	bool textures_generated = false; // to avoid multiple generations
	bool buffer_generated = false;

	size_t debug_vao_cnt = 0; // debug only, print vao index in console

private:
	tinygltf::Model* model;

	glm::mat4 world;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;	
};

