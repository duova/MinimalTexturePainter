#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <nfd/nfd.h>

#include "DirectionalLight.h"
#include "Renderer.h"
#include "WorldObject.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

unsigned int screen_width = 1280;
unsigned int screen_height = 720;

//For rendering output texture.
float quadVert[] = { 1.f, 1.f, 0,
				1.f, -1.f, 0,
				-1.f, -1.f, 0,
				-1.f, 1.f, 0 };

unsigned int quadElem[] = { 0, 1, 3,
					1, 2, 3 };

unsigned int quadVBO;
unsigned int quadVAO;
unsigned int quadEBO;

unsigned int mainFramebuffer;
unsigned int mainTexture;

std::vector<WorldObject> worldObjects{};

bool toolbarActive;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void processInput(GLFWwindow* window);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void start(Shader& mainShader, Shader& shadowShader, Shader& quadShader, Renderer& renderer);

void tick(float deltaTime, Shader& mainShader, Shader& shadowShader, Shader& quadShader, Renderer& renderer);

void generateMainFramebufferAttachments();

void openModel();

int main()
{
	//GLFW init.
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "MinimalTexturePainter", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD." << std::endl;
		return -1;
	}

	//IMGUI init.
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	Shader mainShader("default.vert", "default.frag");
	Shader shadowShader("shadow.vert", "shadow.frag");
	Shader quadShader("quad.vert", "quad.frag");
	Renderer renderer(mainShader, shadowShader, 1024, 1024);

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	start(mainShader, shadowShader, quadShader, renderer);

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		processInput(window);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		tick(deltaTime, mainShader, shadowShader, quadShader, renderer);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screen_width = width;
	screen_height = height;

	//Need to change render resolution.
	glBindFramebuffer(GL_FRAMEBUFFER, mainFramebuffer);
	generateMainFramebufferAttachments();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
}

void start(Shader& mainShader, Shader& shadowShader, Shader& quadShader, Renderer& renderer)
{
	glEnable(GL_DEPTH_TEST);

	//Setup main framebuffer.
	glGenFramebuffers(1, &mainFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, mainFramebuffer);

	generateMainFramebufferAttachments();

	//Check if framebuffer is complete.
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" <<
		std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Setup quad
	//Gen buffers
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glGenBuffers(1, &quadEBO);

	//Buffer vertices
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVert), &quadVert, GL_STATIC_DRAW);

	//Buffer element data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadElem), &quadElem, GL_STATIC_DRAW);

	//Set VAOs
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}

void tick(float deltaTime, Shader& mainShader, Shader& shadowShader, Shader& quadShader, Renderer& renderer)
{
	//Handle UI
	ImGui::Begin("Toolbar", &toolbarActive, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::Button("Open"))
		{
			openModel();
		}
		ImGui::EndMenuBar();
	}
	ImGui::End();

	//Spin object
	for (WorldObject& worldObject : worldObjects)
	{
		worldObject.applyTransform(glm::rotate(glm::mat4(1.f), glm::radians(30.f) * deltaTime, glm::vec3(0, 1, 0)));
	}

	//Render to framebuffer
	CameraParams cameraParams{glm::vec3(0, 0, 5),
		glm::vec3(0, 0, -1.f),
		glm::vec3(0, 1, 0),
		glm::radians(60.f),
		(float)screen_width/(float)screen_height};
	DirectionalLight dirLight{glm::vec3(1.0f, -1.0f, -0.1f),
		glm::vec3(0.05f, 0.05f, 0.05f),
		glm::vec3(0.8f, 0.8f, 0.8f),
		glm::vec3(1.0f, 1.0f, 1.0f)};

	renderer.render(mainFramebuffer, cameraParams, worldObjects, dirLight, screen_width, screen_height);

	//Draw render to screen
	glViewport(0, 0, screen_width, screen_height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	quadShader.useProgram();
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(quadShader.getID(), "render"), 0);
	glBindTexture(GL_TEXTURE_2D, mainTexture);

	glBindVertexArray(quadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
}

void generateMainFramebufferAttachments()
{
	// generate texture
	glGenTextures(1, &mainTexture);
	glBindTexture(GL_TEXTURE_2D, mainTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screen_width, screen_height, 0, GL_RGB,
		GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		mainTexture, 0);

	//Add render buffer for depth and stencil.
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screen_width, screen_height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
		GL_RENDERBUFFER, rbo);
}

void openModel()
{
	NFD_Init();

	nfdu8char_t* outPath;
	nfdu8filteritem_t filters[1] = { { "OBJ file", "obj" }};
	nfdopendialogu8args_t args = { 0 };
	args.filterList = filters;
	args.filterCount = 1;
	nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
	if (result == NFD_OKAY)
	{
		//Create WorldObject.
		worldObjects.clear();
		worldObjects.emplace_back(glm::mat4(1.f), std::make_shared<Model>(outPath));

		NFD_FreePathU8(outPath);
	}
	else if (result == NFD_CANCEL)
	{
	}
	else
	{
		printf("Error: %s\n", NFD_GetError());
	}

	NFD_Quit();
}
