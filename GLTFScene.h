#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "tiny_gltf.h"

#include "camera.h"
#include "Shader.h"

#include "GLTFModel.h"


/*
	Movement - WASD
	Camera - Mouse
	Quit - Escape
	F5 - reload scene file (scene_json_file)

	drop your models into "model" folder
	drop your shaders into "shaders" folder
*/

class GLTFScene
{
public:
	GLTFScene();

	void init();
	void processInput(GLFWwindow* window, float delta);

	void render_setup();
	void render(GLFWwindow* window);

	void scene_init();
	void reload_models_transform();

	void cleanup();

	Camera& getCamera();

public:
	const std::string scene_json_file = "./scene_setup.json";

private:
	// to handle single pressing
	bool input_pressed_f5 = false;

private:
	Camera camera;
	std::vector<GLTFModel*> models;

	std::map<std::string, Shader*> shaders;
	Shader* shader_current;
};

