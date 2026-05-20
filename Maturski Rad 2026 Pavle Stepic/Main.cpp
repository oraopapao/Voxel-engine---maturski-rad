#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include"shaderClass.h"
#include"VBO.h"
#include"EBO.h"
#include"VAO.h"
#include"textureClass.h"
#include"TextureArray.h"

#include"Camera.h"
#include"Chunk.h"
#include"FBO.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>

#include <chrono>

GLuint windowWidth = 1024;
GLuint windowHeight = 1024;
float deltaTime = 0.0f;
float delta = 0.0f;
float lastFrame = 0.0f;
unsigned int amountOfFrames = 0;
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	if (width == 0 || height == 0) return;

	glViewport(0, 0, width, height);

	Camera* camera = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window));

	if (camera != nullptr) {
		camera->width = width;
		camera->height = height;
		
	}
}

int main() {
    
    glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Pavle Stepic - Maturski Rad", NULL, NULL);

	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	gladLoadGL();

	glViewport(0, 0, windowWidth, windowHeight);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_MULTISAMPLE);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	Shader shaderProgram("default.vert", "default.frag");
	shaderProgram.Activate();
	glUniform1i(glGetUniformLocation(shaderProgram.ID, "tex0"), 0);
	glUniform1i(glGetUniformLocation(shaderProgram.ID, "shadowMap"), 1);
	glm::vec3 lightDir = glm::normalize(glm::vec3(2.0f, 2.0f, 1.0f));
	glUniform3fv(glGetUniformLocation(shaderProgram.ID, "lightDir"), 1, glm::value_ptr(lightDir));
	GLuint defaultModelLoc = glGetUniformLocation(shaderProgram.ID, "model");
	GLuint defaultCamMatrixLoc = glGetUniformLocation(shaderProgram.ID, "camMatrix");

	TextureArray textureArray({ "grasstop.png", "grass.png","dirt.png","stone.png"}, 16, 16);
	textureArray.TexUnit(shaderProgram, "tex0", 0);

	Camera camera(windowWidth, windowHeight, glm::vec3(0.0f, 10.0f, 0.0f));
	glfwSetWindowUserPointer(window, &camera);

	GlobalEBO globEBO;
	std::unordered_map<int, int> chunkMap;
	std::vector<Chunk> chunks;
	chunks.reserve(64*64);

	auto start = std::chrono::high_resolution_clock::now();

	for (int i = -32; i < 32; i++) {
		for (int j = -32; j < 32; j++) {
			chunkMap[i * 100 + j] = chunks.size();
			chunks.emplace_back(glm::vec3(i * CHUNK_SIZE_X, 0, j * CHUNK_SIZE_Z));
			chunks.back().Generate();
		}
	}

	auto t1 = std::chrono::high_resolution_clock::now();

	for (int i = -32; i < 32; i++) {
		for (int j = -32; j < 32; j++) {
			Chunk& chunk = chunks[chunkMap[i * 100 + j]];

			if (chunkMap.count(i * 100 + (j + 1))) chunk.neighbors[0] = &chunks[chunkMap[i * 100 + (j + 1)]];
			if (chunkMap.count(i * 100 + (j - 1))) chunk.neighbors[1] = &chunks[chunkMap[i * 100 + (j - 1)]];
			if (chunkMap.count((i + 1) * 100 + j)) chunk.neighbors[2] = &chunks[chunkMap[(i + 1) * 100 + j]];
			if (chunkMap.count((i - 1) * 100 + j)) chunk.neighbors[3] = &chunks[chunkMap[(i - 1) * 100 + j]];
		}
	}

	auto t2 = std::chrono::high_resolution_clock::now();

	for (auto& chunk : chunks) {
		chunk.BuildMesh(globEBO);
	}

	auto t3 = std::chrono::high_resolution_clock::now();

	printf("Generate loop: %.2f ms\n", std::chrono::duration<float, std::milli>(t1 - start).count());
	printf("Neighbors: %.2f ms\n", std::chrono::duration<float, std::milli>(t2 - t1).count());
	printf("BuildMesh loop: %.2f ms\n", std::chrono::duration<float, std::milli>(t3 - t2).count());


	while (!glfwWindowShouldClose(window)) {
		
		//main pass
		glClearColor(0.25f, 0.15f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		shaderProgram.Activate();
		#ifdef _DEBUG
				shaderProgram.checkShaderError();
		#endif

		camera.Matrix(70.0f, 0.1f, 2000.0f, defaultCamMatrixLoc, "camMatrix");
		camera.Inputs(window, deltaTime);

		glActiveTexture(GL_TEXTURE0);
		textureArray.Bind();

		for (auto& chunk : chunks) {
			glm::mat4 model = glm::translate(glm::mat4(1.0f), chunk.Position);
			glUniformMatrix4fv(defaultModelLoc, 1, GL_FALSE, glm::value_ptr(model));
			chunk.Render();
		}


		glfwPollEvents();
		glfwSwapBuffers(window);
		amountOfFrames++;
	}

	std::cout << amountOfFrames /  glfwGetTime() << std::endl; //prosecan fps ovaj run
	std::cout << glfwGetTime() / amountOfFrames << std::endl; //prosecan 
	std::cout << "quads: "+ std::to_string(chunks[0].totalWorldQuads) << std::endl; //prosecan ms
	std::cout << "verts: " + std::to_string(chunks[0].totalWorldQuads * 4) << std::endl; //prosecan ms


	shaderProgram.Delete();
	textureArray.Delete();

	glfwTerminate();
	return 0;
}