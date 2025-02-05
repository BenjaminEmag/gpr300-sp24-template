#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>


#include <ew/texture.h>
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/transform.h>

#include <ew/camera.h>
#include <ew/cameraController.h>
#include <iostream>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

ew::Camera camera;
ew::CameraController cameraController;

struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;

bool blur = true;
bool scanline = true;
bool vignette = true;
bool chromaticAbberation = true;
float abberationStrenght = 2.0f;

int main() {

	GLFWwindow* window = initWindow("Assignment 1", screenWidth, screenHeight);
	glEnable(GL_DEPTH_TEST);

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	ew::Shader litShader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Model monkeyModel = ew::Model("assets/suzanne.fbx");
	ew::Transform monkeyTransform;

	GLuint brickTexture = ew::loadTexture("assets/brick2_color.jpg");
	GLuint brickNormal = ew::loadTexture("assets/brick2_normal.jpg");

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); //Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f; //Vertical field of view, in degrees

	// Framebuffer Setup
	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	unsigned int textureColorbuffer;
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, screenWidth, screenHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind

	unsigned int dummyVAO;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCreateVertexArrays(1, &dummyVAO);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	ew::Shader postProcessShader("assets/buff.vert", "assets/buff.frag");

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;


		// Spin Monkey
		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

		// Move Camera
		cameraController.move(window, &camera, deltaTime);

		//RENDER
		glDepthMask(GL_TRUE);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		litShader.use();
		litShader.setMat4("_Model", monkeyTransform.modelMatrix());
		litShader.setInt("_MainTex", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, brickTexture);
		litShader.setInt("_NormalMap", 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, brickNormal);
		litShader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		litShader.setVec3("_EyePos", camera.position);
		litShader.setFloat("_Material.Ka", material.Ka);
		litShader.setFloat("_Material.Kd", material.Kd);
		litShader.setFloat("_Material.Ks", material.Ks);
		litShader.setFloat("_Material.Shininess", material.Shininess);

		monkeyModel.draw(); // Render the monkey model scene
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDepthMask(GL_FALSE);

		postProcessShader.use();

		// Bind our offscreen texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		postProcessShader.setInt("screenTexture", 0);

		postProcessShader.setInt("blur", blur);
		postProcessShader.setInt("chromaticAberration", chromaticAbberation);
		postProcessShader.setFloat("aberrationStrength", abberationStrenght);
		postProcessShader.setInt("vignette", vignette);
		postProcessShader.setInt("scanline", scanline);

		glDisable(GL_DEPTH_TEST);
		glBindVertexArray(dummyVAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);
		glEnable(GL_DEPTH_TEST);


		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

static void resetCamera(ew::Camera* camera, ew::CameraController* controller) {
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}


void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Cool Monkey");
	if (ImGui::Button("Reset Camera")) resetCamera(&camera, &cameraController);

	if (ImGui::CollapsingHeader("Material")) {
		ImGui::SliderFloat("AmbientK", &material.Ka, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.Kd, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.Ks, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
	}

	if (ImGui::CollapsingHeader("Post Process")) {
		ImGui::Checkbox("chromatic Aberration", &chromaticAbberation);
		ImGui::SliderFloat("Aberration Srenght", &abberationStrenght, 0.0f, 8.0f);
		ImGui::Checkbox("Blur", &blur);
		ImGui::Checkbox("Vignette", &vignette);
		ImGui::Checkbox("Scan Lines", &scanline);

	}
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
	camera.aspectRatio = (float)width / height;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}