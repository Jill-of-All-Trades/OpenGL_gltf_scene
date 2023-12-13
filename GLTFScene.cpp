#include "GLTFScene.h"

#include "json.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

GLTFScene::GLTFScene()
{
	models = std::vector<GLTFModel*>();

	shaders = std::map<std::string, Shader*>();
	shader_current = nullptr;

	camera.MovementSpeed = 20.;
}

void GLTFScene::init()
{
	// Load models
	std::ifstream f(scene_json_file);
	nlohmann::json json = nlohmann::json::parse(f);

	std::vector<std::string> model_paths{};
	for (auto& p : json["models"])
		model_paths.push_back(p);

	// Note: we are using pointers so model will not disappear after
	// we left the init method
	for (std::string model_path : model_paths) {
		GLTFModel* gtlf_model = new GLTFModel();
		if (gtlf_model->load(model_path.c_str())) {
			models.push_back(gtlf_model);
		}
		else {
			delete gtlf_model;
		}
	}

	// Load shaders
	shaders["passthrough"] = new Shader("./shaders/passthrough.vert", "./shaders/passthrough.frag");
}

void GLTFScene::processInput(GLFWwindow* window, float delta)
{
	// Close app
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Reload Models Transform
	if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS && !input_pressed_f5) {
		input_pressed_f5 = true;
		reload_models_transform();
	}
	else input_pressed_f5 = false;

	// CAMERA
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, delta);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, delta);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, delta);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, delta);
}

Camera& GLTFScene::getCamera()
{
	return camera;
}

// RENDER HERE
void GLTFScene::render_setup()
{
	// Setup GPU
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Set shader
	shader_current = shaders["passthrough"];
	shader_current->use();

	// Bind models
	for (auto& model : models) {
		model->bind();
	}
}

void GLTFScene::render(GLFWwindow* window)
{
	// Clear
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Camera
	GLint window_width, window_height;
	glfwGetWindowSize(window, &window_width, &window_height);

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)window_width / (float)window_height, 0.1f, 1000.0f);

	shader_current->setMat4("view", view);
	shader_current->setMat4("projection", projection);

	// Models
	for (size_t model_index = 0; model_index < models.size(); ++model_index) {
		GLTFModel* model = models.at(model_index);

		model->draw(*shader_current);
	}
}

void GLTFScene::scene_init()
{
	// Change to suit your needs
	reload_models_transform();
}

// Utilities
void GLTFScene::reload_models_transform()
{
	std::ifstream f(scene_json_file);
	nlohmann::json json = nlohmann::json::parse(f);

	std::vector<glm::vec3> m_poses{};
	std::vector<glm::vec3> m_rotate{};
	std::vector<glm::vec3> m_scale{};

	for (auto& transf : json["transform"]) {
		std::vector<double> tpos = transf["pos"];
		std::vector<double> trot = transf["rot"];
		std::vector<double> tscl = transf["scl"];

		m_poses.push_back(glm::make_vec3(tpos.data()));
		m_rotate.push_back(glm::make_vec3(trot.data()));
		m_scale.push_back(glm::make_vec3(tscl.data()));
	}

	// apply to models
	for (size_t mi = 0; mi < models.size(); ++mi) {
		GLTFModel* model = models.at(mi);

		glm::vec3 vec_pos = m_poses[mi % models.size()];
		glm::vec3 vec_rot = m_rotate[mi % models.size()];
		glm::vec3 vec_scl = m_scale[mi % models.size()];

		//glm::vec3 vec_pos_adj = vec_pos * (glm::vec3(1.0) / vec_scl);

		model->setPosition(vec_pos.x, vec_pos.y, vec_pos.z);
		model->setRotation(vec_rot.x, vec_rot.y, vec_rot.z);
		model->setScale(vec_scl.x, vec_scl.y, vec_scl.z);
	}
}

void GLTFScene::cleanup()
{
	// clean models
	for (auto& m : models) {
		delete m;
	}

	// clean shaders
	for (auto& shd : shaders) {
		delete shd.second;
	}
}


