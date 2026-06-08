#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <chrono>

#include "shaderClass.h"
#include "VBO.h"
#include "EBO.h"
#include "VAO.h"
#include "textureClass.h"
#include "TextureArray.h"
#include "Camera.h"
#include "Chunk.h"
#include "FBO.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// ─── globals ──────────────────────────────────────────────────────────────────

GLuint       windowWidth = 1024;
GLuint       windowHeight = 1024;
float        deltaTime = 0.0f;
float        lastFrame = 0.0f;
unsigned int amountOfFrames = 0;

// ─── callbacks ────────────────────────────────────────────────────────────────

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (width == 0 || height == 0) return;
    glViewport(0, 0, width, height);
    Camera* camera = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (camera) {
        camera->width = width;
        camera->height = height;
    }
}

// ─── main ─────────────────────────────────────────────────────────────────────

int main() {

    // ── init ─────────────────────────────────────────────────────────────────
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Pavle Stepic - Maturski Rad", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window\n";
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

    // ── shaderi i teksture ───────────────────────────────────────────────────
    Shader shaderProgram("default.vert", "default.frag");
    shaderProgram.Activate();
    glUniform1i(glGetUniformLocation(shaderProgram.ID, "tex0"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram.ID, "shadowMap"), 1);

    glm::vec3 lightDir = glm::normalize(glm::vec3(2.0f, 2.0f, 1.0f));
    glUniform3fv(glGetUniformLocation(shaderProgram.ID, "lightDir"), 1, glm::value_ptr(lightDir));

    GLuint defaultModelLoc = glGetUniformLocation(shaderProgram.ID, "model");
    GLuint defaultCamMatrixLoc = glGetUniformLocation(shaderProgram.ID, "camMatrix");

    TextureArray textureArray(
        { "grasstop.png", "grass.png", "dirt.png", "stone.png", "snowtop.png", "snow.png", "water.png", "sand.png" },
        16, 16
    );
    textureArray.TexUnit(shaderProgram, "tex0", 0);

    // ── kamera ───────────────────────────────────────────────────────────────
    Camera camera(windowWidth, windowHeight, glm::vec3(0.0f, 10.0f, 0.0f));
    glfwSetWindowUserPointer(window, &camera);

    // ── generisanje sveta ────────────────────────────────────────────────────
    GlobalEBO globEBO;
    std::unordered_map<int, int> chunkMap;
    std::vector<Chunk> chunks;
    const int RADIUS = 64;
    chunks.reserve(RADIUS * RADIUS * 4);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = -RADIUS; i < RADIUS; i++)
        for (int j = -RADIUS; j < RADIUS; j++) {
            chunkMap[i * 1000 + j] = chunks.size();
            chunks.emplace_back(glm::vec3(i * CHUNK_SIZE_X, 0, j * CHUNK_SIZE_Z));
            chunks.back().Generate();
        }

    auto t1 = std::chrono::high_resolution_clock::now();

    for (int i = -RADIUS; i < RADIUS; i++)
        for (int j = -RADIUS; j < RADIUS; j++) {
            Chunk& chunk = chunks[chunkMap[i * 1000 + j]];
            if (chunkMap.count(i * 1000 + (j + 1))) chunk.neighbors[0] = &chunks[chunkMap[i * 1000 + (j + 1)]];
            if (chunkMap.count(i * 1000 + (j - 1))) chunk.neighbors[1] = &chunks[chunkMap[i * 1000 + (j - 1)]];
            if (chunkMap.count((i + 1) * 1000 + j)) chunk.neighbors[2] = &chunks[chunkMap[(i + 1) * 1000 + j]];
            if (chunkMap.count((i - 1) * 1000 + j)) chunk.neighbors[3] = &chunks[chunkMap[(i - 1) * 1000 + j]];
        }

    auto t2 = std::chrono::high_resolution_clock::now();

    for (auto& chunk : chunks)
        chunk.BuildMesh(globEBO);

    auto t3 = std::chrono::high_resolution_clock::now();

    printf("Generate:  %.2f ms\n", std::chrono::duration<float, std::milli>(t1 - start).count());
    printf("Neighbors: %.2f ms\n", std::chrono::duration<float, std::milli>(t2 - t1).count());
    printf("BuildMesh: %.2f ms\n", std::chrono::duration<float, std::milli>(t3 - t2).count());

    float loadTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        
        glClearColor(0.25f, 0.15f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        shaderProgram.Activate();

        camera.Matrix(70.0f, 0.1f, 2000.0f, defaultCamMatrixLoc, "camMatrix");
        camera.Inputs(window, deltaTime);

        glActiveTexture(GL_TEXTURE0);
        textureArray.Bind();

        for (auto& chunk : chunks) {
            glm::vec3 toChunk = chunk.Position - camera.Position;
            if (glm::length(toChunk) > CHUNK_SIZE_X * 2 && glm::dot(camera.Orientation, glm::normalize(toChunk)) < -0.25f) continue;

            glm::mat4 model = glm::translate(glm::mat4(1.0f), chunk.Position);
            glUniformMatrix4fv(defaultModelLoc, 1, GL_FALSE, glm::value_ptr(model));
            chunk.Render();
        }

        glfwPollEvents();
        glfwSwapBuffers(window);
        amountOfFrames++;
    }

    printf("FPS:   %.2f\n", amountOfFrames / glfwGetTime());
    printf("ms:    %.4f\n", glfwGetTime() / amountOfFrames*1000);
    printf("quads: %d\n", chunks[0].totalWorldQuads);
    printf("verts: %d\n", chunks[0].totalWorldQuads * 4);

    shaderProgram.Delete();
    textureArray.Delete();
    glfwTerminate();
    return 0;
}