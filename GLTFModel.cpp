#include "GLTFModel.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

// Constructors
GLTFModel::GLTFModel()
{
	// Init
	model = new tinygltf::Model();
	setPosition(0, 0, 0);
	setRotation(0, 0, 0);
	setScale(1, 1, 1);
	world = glm::mat4(1.0);
}

GLTFModel::GLTFModel(tinygltf::Model& m, glm::vec3 pos = glm::vec3(0.0), glm::vec3 rot = glm::vec3(0.0), glm::vec3 scl = glm::vec3(1.0))
{
	setModel(m);
	setPosition(pos.x, pos.y, pos.z);
	setRotation(rot.x, rot.y, rot.z);
	setScale(scl.x, scl.y, scl.z);
	world = glm::mat4(1.0);
}

GLTFModel::~GLTFModel()
{
	unbind();

	if (model != nullptr) delete model;
}

bool GLTFModel::load(const char* filename)
{
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool success = loader.LoadASCIIFromFile(model, &err, &warn, filename);
	if (!warn.empty()) std::cout << "WARN: " << warn << std::endl;
	if (!err.empty()) std::cout << "ERROR: " << err << std::endl;

	if (success) std::cout << "Loaded glTF model: " << filename << std::endl;
	else std::cout << "Failed to load glTF model: " << filename << std::endl;

	return success;
}

// Getters
tinygltf::Model* GLTFModel::getModel() const
{
	return model;
}

glm::mat4 GLTFModel::getWorld() const
{
	return world;
}

glm::vec3 GLTFModel::getPosition() const
{
	return position;
}

glm::vec3 GLTFModel::getRotation() const
{
	return rotation;
}

glm::vec3 GLTFModel::getScale() const
{
	return scale;
}

// Setters
void GLTFModel::setModel(tinygltf::Model& m)
{
	model = &m;
}

void GLTFModel::setPosition(double x, double y, double z)
{
	position = glm::vec3(x, y, z);
}

void GLTFModel::setRotation(double xr, double yr, double zr)
{
	rotation = glm::vec3(xr, yr, zr);
}

void GLTFModel::setScale(double xs, double ys, double zs)
{
	scale = glm::vec3(xs, ys, zs);
}

// Generate data
void GLTFModel::generateBuffers()
{
	if (buffer_generated) {
		std::cout << "WARN: buffers already generated!" << std::endl;
		return;
	}

	for (size_t i = 0; i < model->bufferViews.size(); ++i) {
		const tinygltf::BufferView& bufferView = model->bufferViews[i];

		if (bufferView.target == 0) { // unsupported yet
			std::cout << "WARN: bufferView.target is unsupported (0)" << std::endl;
			continue;
		}

		const tinygltf::Buffer& buffer = model->buffers[bufferView.buffer];

		GLuint bo; // it could be vbo or ebo (check target)
		glGenBuffers(1, &bo);
		buffer_objects[i] = bo;

		glBindBuffer(bufferView.target, bo);
		glBufferData(bufferView.target, bufferView.byteLength, &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
	}

	buffer_generated = true;
}

void GLTFModel::generateTextures()
{
	if (textures_generated) {
		std::cout << "WARN: textures already generated!" << std::endl;
		return;
	}

	for (size_t ti = 0; ti < model->textures.size(); ++ti) { 
		tinygltf::Texture& tex = model->textures[ti];
		tinygltf::Sampler& sampler = model->samplers[tex.sampler];

		if (tex.source > -1) {
			GLuint texid;
			glGenTextures(1, &texid);
			textures.push_back(texid);

			tinygltf::Image& image = model->images[tex.source];

			// glActiveTexture(GL_TEXTURE0); // by default, it activated
			glBindTexture(GL_TEXTURE_2D, texid);
			//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			GLfloat min_filter = !generate_mipmaps || sampler.minFilter == -1 ? GL_LINEAR : sampler.minFilter; // GL_LINEAR == 9729 (0x2601)
			GLfloat mag_filter = !generate_mipmaps || sampler.magFilter == -1 ? GL_LINEAR : sampler.magFilter;
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler.wrapS);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler.wrapT);

			GLenum format;
			switch (image.component) {
			case 1: format = GL_RED; break;
			case 2: format = GL_RG; break;
			case 3: format = GL_RGB; break;
			default: format = GL_RGBA; break;
			}

			GLenum type = GL_UNSIGNED_BYTE;
			if (image.bits == 8) {
				type = GL_UNSIGNED_BYTE;
			}
			else if (image.bits == 16) {
				type = GL_UNSIGNED_SHORT;
			}

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0,
				format, type, &image.image.at(0));

			// Mipmap
			if (generate_mipmaps)
				glGenerateMipmap(GL_TEXTURE_2D);

		}
	}

	textures_generated = true;
}

// Binding
void GLTFModel::bind()
{
	std::cout << "binding gltf model..." << std::endl;
	// generates textures if they have not been generated previously
	generateTextures();

	// generates buffers (vbo/ebo) if they have not been generated previously
	generateBuffers();

	// traverse scene nodes
	const tinygltf::Scene& scene = model->scenes[model->defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model->nodes.size()));
		traverseNode(model->nodes[scene.nodes[i]], glm::mat4(1.0));
	}

	//cleanup buffer_objects
	for (auto it = buffer_objects.cbegin(); it != buffer_objects.end(); ) {
		tinygltf::BufferView bufferView = model->bufferViews[it->first];
		glDeleteBuffers(1, &buffer_objects[it->first]);
		buffer_objects.erase(it++);
	}
}

void GLTFModel::unbind()
{
	for (auto& vao : meshes_vaos) {
		glDeleteVertexArrays(1, &vao);
	}
	meshes_vaos.clear();

	for (auto it = buffer_objects.cbegin(); it != buffer_objects.end(); ) {
		tinygltf::BufferView bufferView = model->bufferViews[it->first];
		glDeleteBuffers(1, &buffer_objects[it->first]);
		buffer_objects.erase(it++);
	}
}

void GLTFModel::traverseNode(tinygltf::Node& node, glm::mat4 wrld)
{
	glm::vec3 trsl = glm::vec3(0.0);
	if (node.translation.size() > 0) trsl = glm::make_vec3(node.translation.data());

	glm::quat rot = glm::quat(1., 0., 0., 0.);
	if (node.rotation.size() > 0) {
		double* rv = node.rotation.data();
		rot = glm::quat(rv[3], rv[0], rv[1], rv[2]); // should be make_quat
	}

	glm::vec3 scl = glm::vec3(1.0, 1.0, 1.0);
	if (node.scale.size() > 0) scl = glm::make_vec3(node.scale.data());

	glm::mat4 matWrld = glm::mat4(1.0);
	if (node.matrix.size() > 0) matWrld = glm::make_mat4(node.matrix.data());

	// init matrices
	glm::mat4 matTrsl = glm::mat4(1.0);
	glm::mat4 matRot = glm::mat4(1.0);
	glm::mat4 matScl = glm::mat4(1.0);

	matTrsl = glm::translate(matTrsl, trsl);
	matRot = glm::mat4_cast(rot);
	matScl = glm::scale(matScl, scl);

	// multiply all mat together
	glm::mat4 matNextNode = wrld * matWrld * matTrsl * matRot * matScl;

	// If node has mesh, bind it
	if ((node.mesh >= 0) && (node.mesh < model->meshes.size())) { // there, mesh is index
		// save matrices for mesh
		meshes_world.push_back(matNextNode);

		bindMesh(model->meshes[node.mesh]);
	}

	// if has children, traverse nodes
	for (size_t i = 0; i < node.children.size(); ++i) {
		assert((node.children[i] >= 0) && (node.children[i] < model->nodes.size()));
		traverseNode(model->nodes[node.children[i]], matNextNode);
	}
}

void GLTFModel::bindMesh(tinygltf::Mesh& mesh)
{
	/*
		BufferView:
		buffer		- (uint) buffer index
		byteOffset	- (uint) offset inside buffer in bytes
		byteLength	- (uint) length of the part of the buffer (in bytes)
		target		- 34962 is ARRAY_BUFFER, 34963 is ELEMENT_ARRAY_BUFFER
	*/

	GLuint VAO; // gen & bind vao for mesh
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	meshes_vaos.push_back(VAO);
	std::cout << " -> bind mesh: " << debug_vao_cnt++ << std::endl;

	//generateBuffers();
	for (size_t i = 0; i < model->bufferViews.size(); ++i) {
		const tinygltf::BufferView& bufferView = model->bufferViews[i];

		if (bufferView.target == 0) { // unsupported yet
			std::cout << "WARN: bufferView.target is unsupported (0)" << std::endl;
			continue;
		}

		const tinygltf::Buffer& buffer = model->buffers[bufferView.buffer];

		glBindBuffer(bufferView.target, buffer_objects[i]);
	}

	/*
		Accessor:
		bufferView		- (uint) buffer view index
		byteOffset		- (uint) offset within buffer view (because diff accessors can use same view)
		componentType	- 5123 - USHORT, 5126 - FLOAT
		count			- (uint) count of data
		type			- SCALAR, VEC(I) or MAT(I)
		max				- (array) max values
		min				- (array) min values
	*/
	for (size_t i = 0; i < mesh.primitives.size(); ++i) {
		tinygltf::Primitive primitive = mesh.primitives[i];
		tinygltf::Accessor indexAccessor = model->accessors[primitive.indices];

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_objects.at(indexAccessor.bufferView));

		for (auto& attrib : primitive.attributes) {
			const tinygltf::Accessor& accessor = model->accessors[attrib.second];
			const tinygltf::BufferView& bufferView = model->bufferViews[accessor.bufferView];

			int byteStride = accessor.ByteStride(bufferView);
			if (byteStride == -1) {
				std::cout << "Err: bindMesh invalid accessor byteStride for primitive " << i << std::endl;
				continue;
			}

			// Accessors available for vbo, not ebo. So GL_ARRAY_BUFFER is only option.
			// But to be sure if anything is OK, lets check it
			if (bufferView.target != GL_ARRAY_BUFFER) {
				std::cout << "Err: bindMesh buffer object target isn't GL_ARRAY_BUFFER for primitive " << i << std::endl;
				continue;
			}

			glBindBuffer(GL_ARRAY_BUFFER, buffer_objects[accessor.bufferView]); // GL_ARRAY_BUFFER

			int size = 1;
			if (accessor.type != TINYGLTF_TYPE_SCALAR) {
				size = accessor.type;
			}

			// Note: If your shaders has other attributes, change it here
			int vaa = -1;
			if (attrib.first.compare("POSITION") == 0)		vaa = 0;
			if (attrib.first.compare("NORMAL") == 0)		vaa = 1;
			if (attrib.first.compare("TEXCOORD_0") == 0)	vaa = 2;

			if (vaa > -1) {
				glEnableVertexAttribArray(vaa);
				glVertexAttribPointer(vaa, size, accessor.componentType,
					accessor.normalized ? GL_TRUE : GL_FALSE,
					byteStride, BUFFER_OFFSET(accessor.byteOffset));
			}
			else {
				std::cout << "ERROR: vaa is missing: " << attrib.first << std::endl;
			}
		}
	}

	glBindVertexArray(0); // unbind vao
}

// Render
void GLTFModel::draw(Shader& shader)
{
	const tinygltf::Scene& scene = model->scenes[model->defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		drawNode(shader, model->nodes[scene.nodes[i]]);
	}
}

void GLTFModel::drawNode(Shader& shader,tinygltf::Node& node)
{
	// if node has mesh - draw it
	if ((node.mesh >= 0) && (node.mesh < model->meshes.size())) {
		// set matrices for shader
		//glm::scale(meshes_world[node.mesh], glm::vec3(0.1,0.1,0.1))
		shader.setMat4("model", getMeshWorld(node.mesh));

		glBindVertexArray(meshes_vaos[node.mesh]);

		drawMesh(shader, model->meshes[node.mesh]);

		glBindVertexArray(0);
	}

	// recursively draw node and children nodes
	for (size_t i = 0; i < node.children.size(); i++) {
		drawNode(shader, model->nodes[node.children[i]]);
	}
}

void GLTFModel::drawMesh(Shader& shader, tinygltf::Mesh& mesh)
{
	for (size_t i = 0; i < mesh.primitives.size(); ++i) {
		tinygltf::Primitive primitive = mesh.primitives[i];
		tinygltf::Accessor indexAccessor = model->accessors[primitive.indices];

		// Apply material
		proccessMaterial(shader, primitive);
		
		// Draw
		glDrawElements(primitive.mode, indexAccessor.count,
			indexAccessor.componentType,
			BUFFER_OFFSET(indexAccessor.byteOffset));

		// Unbind
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void GLTFModel::proccessMaterial(Shader& shader, tinygltf::Primitive& primitive)
{
	tinygltf::Material material = model->materials[primitive.material];

	// Texture bind
	int tex_base_index = material.pbrMetallicRoughness.baseColorTexture.index;
	if (tex_base_index > -1) {
		GLuint tex_base = textures[tex_base_index];

		glActiveTexture(GL_TEXTURE0); // to change index just +1 (any number)
		shader.setInt("tex_diffuse", 0); // 0 is sampler index
		glBindTexture(GL_TEXTURE_2D, tex_base);
	}

	// Color Factor
	std::vector<double>* cf = &material.pbrMetallicRoughness.baseColorFactor;
	shader.setVec4("color_factor", (*cf)[0], (*cf)[1], (*cf)[2], (*cf)[3]);
	
	// WIP: there could be other parameters, like roughness, metallic ect
	// also, I ignore "alphaMode" (render_setup -> enable blend), and doubleSided (for optimization)
}

glm::mat4 GLTFModel::getMeshWorld(int mesh_index)
{
	glm::mat4 mesh_world = glm::translate(meshes_world[mesh_index], getPosition());
	mesh_world = glm::rotate(mesh_world, glm::radians(getRotation().x), glm::vec3(1, 0, 0));
	mesh_world = glm::rotate(mesh_world, glm::radians(getRotation().y), glm::vec3(0, 1, 0));
	mesh_world = glm::rotate(mesh_world, glm::radians(getRotation().z), glm::vec3(0, 0, 1));
	mesh_world = glm::scale(mesh_world, getScale());

	return mesh_world;

}

