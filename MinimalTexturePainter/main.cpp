#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <nfd/nfd.h>

#include "DirectionalLight.h"
#include "Renderer.h"
#include "TextureBlender.h"
#include "WorldObject.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "stb_image_write.h"

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

unsigned int uvRenderFramebuffer;
unsigned int uvRenderTexture;

unsigned int currentBrushDiffuse;
unsigned int currentBrushSpecular;
unsigned int currentBrushNormal;

float brushSize = 1;
float brushAlpha = 1;
float brushSourceScale = 1;

float lightYaw = 0;
float lightPitch = 0;
float modelScale = 1;

bool firstRightMouse = true;
float pitch = 0.0f;
float yaw = -90.0f;

bool firstMiddleMouse = true;

float lastX = 0.0f;
float lastY = 0.0f;

float uvMouseX = 0.0f;
float uvMouseY = 0.0f;

GLfloat* uvPixels;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 6.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

std::vector<WorldObject> worldObjects{};

std::unique_ptr<Shader> blendShader;
std::unique_ptr<TextureBlender> diffuseBlender;
std::unique_ptr<TextureBlender> specularBlender;
std::unique_ptr<TextureBlender> normalBlender;
string diffuseName;
string specularName;
string normalName;

bool toolbarActive;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void processInput(GLFWwindow* window, float deltaTime);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void start(Shader& mainShader, Shader& shadowShader, Shader& quadShader, Renderer& renderer);

void tick(float deltaTime, Shader& mainShader, Shader& shadowShader, Shader& quadShader, Shader& uvRenderShader, Renderer& renderer);

void generateMainFramebufferAttachments();

void generateUVFramebufferAttachements();

void openModel();

void openDiffuseTexture();

void openSpecularTexture();

void openNormalTexture();

unsigned int loadTextureFromFile(const char* path, bool useSRGB);

void saveDiffuseTexture();

void saveSpecularTexture();

void saveNormalTexture();

void paint(float deltaTime);

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
	Shader uvRenderShader("uvrender.vert", "uvrender.frag");
	blendShader = std::make_unique<Shader>("blend.vert", "blend.frag");
	Shader quadShader("quad.vert", "quad.frag");
	Renderer renderer(mainShader, shadowShader, 4096, 4096);

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	uvPixels = new GLfloat[screen_width * screen_height * 3];

	start(mainShader, shadowShader, quadShader, renderer);

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		processInput(window, deltaTime);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		tick(deltaTime, mainShader, shadowShader, quadShader, uvRenderShader, renderer);

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

	glBindFramebuffer(GL_FRAMEBUFFER, uvRenderFramebuffer);
	generateUVFramebufferAttachements();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	uvPixels = new GLfloat[screen_width * screen_height * 3];
}

void paint(float deltaTime)
{
	glBindTexture(GL_TEXTURE_2D, uvRenderTexture);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, uvPixels);

	GLfloat r, g, b;

	size_t x = uvMouseX;
	size_t y = screen_height - uvMouseY;

	size_t elmes_per_line = screen_width * 3;

	size_t row = y * elmes_per_line;
	size_t col = x * 3;

	r = uvPixels[row + col];
	g = uvPixels[row + col + 1];
	b = uvPixels[row + col + 2];

	glm::vec2 uv(r, g);

	glBindTexture(GL_TEXTURE_2D, 0);

	//b is used to indicate if the uv is valid.
	if (b < 0.99) return;

	if (diffuseBlender)
	{
		diffuseBlender->blend(uv, brushSize, brushAlpha * deltaTime, brushSourceScale, 4096);
	}
	if (specularBlender)
	{
		specularBlender->blend(uv, brushSize, brushAlpha * deltaTime, brushSourceScale, 4096);
	}
	if (normalBlender)
	{
		normalBlender->blend(uv, brushSize, brushAlpha * deltaTime, brushSourceScale, 4096);
	}
}

void processInput(GLFWwindow* window, float deltaTime)
{
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
	{
		paint(deltaTime);
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	uvMouseX = xpos;
	uvMouseY = ypos;

	//Handle rotation.
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
		if (firstRightMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstRightMouse = false;
		}
	}
	if (!firstRightMouse)
	{
		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;
		lastX = xpos;
		lastY = ypos;
		float sensitivity = 0.2f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;
		yaw += xoffset;
		pitch += yoffset;
		pitch = std::min(pitch, 89.0f);
		pitch = std::max(pitch, -89.0f);
		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(direction);
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE)
	{
		firstRightMouse = true;
	}

	//Handle pan.
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS) {
		if (firstMiddleMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMiddleMouse = false;
		}
	}
	if (!firstMiddleMouse)
	{
		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;
		lastX = xpos;
		lastY = ypos;
		float sensitivity = 0.005f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * xoffset;
		cameraPos -= glm::normalize(glm::cross(glm::cross(cameraFront, cameraUp), cameraFront)) * yoffset;
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_3) == GLFW_RELEASE)
	{
		firstMiddleMouse = true;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	float sensitivity = 0.2f;
	cameraPos += cameraFront * (float)yoffset * sensitivity;
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

	//Setup uv render framebuffer.
	glGenFramebuffers(1, &uvRenderFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, uvRenderFramebuffer);

	generateUVFramebufferAttachements();

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

void tick(float deltaTime, Shader& mainShader, Shader& shadowShader, Shader& quadShader, Shader& uvRenderShader, Renderer& renderer)
{
	//Draw UI
	ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;
	ImGui::Begin("Toolbar", &toolbarActive, flags);
	if (worldObjects.empty())
	{
		if (ImGui::Button("Open Model"))
		{
			openModel();
		}
	}
	else
	{
		ImGui::Text("LIGHT");
		ImGui::SliderAngle("Light Yaw", &lightYaw, 0, 360);
		ImGui::SliderAngle("Light Pitch", &lightPitch, -90, 90);
		float oldModelScale = modelScale;
		if (ImGui::InputFloat("Scale", &modelScale))
		{
			modelScale = std::max(modelScale, 0.01f);
			for (WorldObject& worldObject : worldObjects)
			{
				worldObject.applyTransform(glm::scale(glm::mat4(1.f), glm::vec3(modelScale / oldModelScale)));
			}
		}
		ImGui::Spacing();
		ImGui::Text("BRUSH");
		ImGui::Text("Diffuse texture: %s", diffuseName.c_str());
		if (ImGui::Button("Open Diffuse"))
		{
			openDiffuseTexture();
		}
		ImGui::Text("Specular texture: %s", specularName.c_str());
		if (ImGui::Button("Open Specular"))
		{
			openSpecularTexture();
		}
		ImGui::Text("Normal texture: %s", normalName.c_str());
		if (ImGui::Button("Open Normal"))
		{
			openNormalTexture();
		}
		ImGui::SliderFloat("Brush Size", &brushSize, 1, 100);
		ImGui::SliderFloat("Brush Alpha", &brushAlpha, 0.1f, 10);
		ImGui::SliderFloat("Brush Source Scale", &brushSourceScale, 0.1f, 10);
		ImGui::Spacing();
		ImGui::Text("SAVE");
		if (ImGui::Button("Save Diffuse"))
		{
			saveDiffuseTexture();
		}
		if (ImGui::Button("Save Specular"))
		{
			saveSpecularTexture();
		}
		if (ImGui::Button("Save Normal"))
		{
			saveNormalTexture();
		}
	}


	ImGui::End();

	//Set common params.
	CameraParams cameraParams{cameraPos,
		cameraFront,
		cameraUp,
		glm::radians(60.f),
		(float)screen_width/(float)screen_height};

	glm::vec3 lightDir = glm::rotate(glm::rotate(glm::mat4(1.0f), lightYaw, glm::vec3(0, 1, 0)), -lightPitch, glm::vec3(0, 0, 1)) * glm::vec4(1, 0, 0, 1);

	DirectionalLight dirLight{ lightDir,
		glm::vec3(0.1f, 0.1f, 0.1f),
		glm::vec3(1.6f, 1.6f, 1.6f),
		glm::vec3(2.0f, 2.0f, 2.0f)};

	//UV render to get the UV to paint the texture on.
	glViewport(0, 0, screen_width, screen_height);
	glBindFramebuffer(GL_FRAMEBUFFER, uvRenderFramebuffer);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	uvRenderShader.useProgram();
	glm::mat4 view = glm::lookAt(cameraParams.position, cameraParams.position + cameraParams.forward, cameraParams.up);
	glm::mat4 projection = glm::perspective(cameraParams.fov, cameraParams.aspect, 0.1f, 100.0f);
	uvRenderShader.setMat4("view", view);
	uvRenderShader.setMat4("projection", projection);

	for (const WorldObject& object : worldObjects)
	{
		glm::mat4 model = object.getTransform();

		uvRenderShader.setMat4("model", model);

		object.getModel().draw(uvRenderShader);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Main render
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

void generateUVFramebufferAttachements()
{
	// generate texture
	glGenTextures(1, &uvRenderTexture);
	glBindTexture(GL_TEXTURE_2D, uvRenderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, screen_width, screen_height, 0, GL_RGB,
		GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		uvRenderTexture, 0);

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

void openDiffuseTexture()
{
	NFD_Init();

	nfdu8char_t* outPath;
	nfdu8filteritem_t filters[1] = { { "Texture file", "png,jpg" } };
	nfdopendialogu8args_t args = { 0 };
	args.filterList = filters;
	args.filterCount = 1;
	nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
	if (result == NFD_OKAY)
	{
		glDeleteTextures(1, &currentBrushDiffuse);
		currentBrushDiffuse = loadTextureFromFile(outPath, true);

		//Create texture blender.
		const vector<Texture>& textures = worldObjects.front().getModel().textures_loaded;
		for (const auto& texture : textures)
		{
			if (texture.type == "texture_diffuse")
			{
				diffuseBlender = std::make_unique<TextureBlender>(*blendShader, texture.id, currentBrushDiffuse);
				break;
			}
		}

		string path = outPath;
		int backslash = path.find_last_of('\\');
		diffuseName = path.substr(backslash + 1, path.size() - backslash - 1);

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

void openSpecularTexture()
{
	NFD_Init();

	nfdu8char_t* outPath;
	nfdu8filteritem_t filters[1] = { { "Texture file", "png,jpg" } };
	nfdopendialogu8args_t args = { 0 };
	args.filterList = filters;
	args.filterCount = 1;
	nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
	if (result == NFD_OKAY)
	{
		glDeleteTextures(1, &currentBrushSpecular);
		currentBrushSpecular = loadTextureFromFile(outPath, false);

		//Create texture blender.
		const vector<Texture>& textures = worldObjects.front().getModel().textures_loaded;
		for (const auto& texture : textures)
		{
			if (texture.type == "texture_specular")
			{
				specularBlender = std::make_unique<TextureBlender>(*blendShader, texture.id, currentBrushSpecular);
				break;
			}
		}

		string path = outPath;
		int backslash = path.find_last_of('\\');
		specularName = path.substr(backslash + 1, path.size() - backslash - 1);

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

void openNormalTexture()
{
	NFD_Init();

	nfdu8char_t* outPath;
	nfdu8filteritem_t filters[1] = { { "Texture file", "png,jpg" } };
	nfdopendialogu8args_t args = { 0 };
	args.filterList = filters;
	args.filterCount = 1;
	nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
	if (result == NFD_OKAY)
	{
		glDeleteTextures(1, &currentBrushNormal);
		currentBrushNormal = loadTextureFromFile(outPath, false);

		//Create texture blender.
		const vector<Texture>& textures = worldObjects.front().getModel().textures_loaded;
		for (const auto& texture : textures)
		{
			if (texture.type == "texture_normal")
			{
				normalBlender = std::make_unique<TextureBlender>(*blendShader, texture.id, currentBrushNormal);
				break;
			}
		}

		string path = outPath;
		int backslash = path.find_last_of('\\');
		normalName = path.substr(backslash + 1, path.size() - backslash - 1);

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

unsigned int loadTextureFromFile(const char* path, bool useSRGB)
{
	string filename = string(path);

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	//Textures are flipped.
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format = 0;
		GLenum internalFormat = 0;
		if (nrComponents == 1)
		{
			format = GL_RED;
			internalFormat = GL_RED;
		}
		else if (nrComponents == 3)
		{
			format = GL_RGB;
			internalFormat = useSRGB ? GL_SRGB : GL_RGB;
		}
		else if (nrComponents == 4)
		{
			format = GL_RGBA;
			internalFormat = useSRGB ? GL_SRGB_ALPHA : GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, (int)internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(data);

	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void saveDiffuseTexture()
{
	NFD_Init();

	nfdu8char_t* outPath;
	nfdu8filteritem_t filters[1] = { { "Texture file", "jpg" } };
	nfdsavedialogu8args_t args = { 0 };
	args.filterList = filters;
	args.filterCount = 1;
	nfdresult_t result = NFD_SaveDialogU8_With(&outPath, &args);
	if (result == NFD_OKAY)
	{
		const vector<Texture>& textures = worldObjects.front().getModel().textures_loaded;
		for (const auto& texture : textures)
		{
			if (texture.type == "texture_diffuse")
			{
				glBindTexture(GL_TEXTURE_2D, texture.id);
				int w, h;
				int miplevel = 0;
				glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &w);
				glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &h);
				GLubyte* toSave = new GLubyte[w * h * 3];
				glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, toSave);
				stbi_flip_vertically_on_write(true);
				stbi_write_jpg(outPath, w, h, 3, toSave, w * 3);

				glBindTexture(GL_TEXTURE_2D, 0);
				delete[] toSave;
				break;
			}
		}

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

void saveSpecularTexture()
{
	NFD_Init();

	nfdu8char_t* outPath;
	nfdu8filteritem_t filters[1] = { { "Texture file", "jpg" } };
	nfdsavedialogu8args_t args = { 0 };
	args.filterList = filters;
	args.filterCount = 1;
	nfdresult_t result = NFD_SaveDialogU8_With(&outPath, &args);
	if (result == NFD_OKAY)
	{
		const vector<Texture>& textures = worldObjects.front().getModel().textures_loaded;
		for (const auto& texture : textures)
		{
			if (texture.type == "texture_specular")
			{
				glBindTexture(GL_TEXTURE_2D, texture.id);
				int w, h;
				int miplevel = 0;
				glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &w);
				glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &h);
				GLubyte* toSave = new GLubyte[w * h * 3];
				glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, toSave);
				stbi_flip_vertically_on_write(true);
				stbi_write_jpg(outPath, w, h, 3, toSave, w * 3);

				glBindTexture(GL_TEXTURE_2D, 0);
				delete[] toSave;
				break;
			}
		}

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

void saveNormalTexture()
{
	NFD_Init();

	nfdu8char_t* outPath;
	nfdu8filteritem_t filters[1] = { { "Texture file", "jpg" } };
	nfdsavedialogu8args_t args = { 0 };
	args.filterList = filters;
	args.filterCount = 1;
	nfdresult_t result = NFD_SaveDialogU8_With(&outPath, &args);
	if (result == NFD_OKAY)
	{
		const vector<Texture>& textures = worldObjects.front().getModel().textures_loaded;
		for (const auto& texture : textures)
		{
			if (texture.type == "texture_normal")
			{
				glBindTexture(GL_TEXTURE_2D, texture.id);
				int w, h;
				int miplevel = 0;
				glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &w);
				glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &h);
				GLubyte* toSave = new GLubyte[w * h * 3];
				glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, toSave);
				stbi_flip_vertically_on_write(true);
				stbi_write_jpg(outPath, w, h, 3, toSave, w * 3);

				glBindTexture(GL_TEXTURE_2D, 0);
				delete[] toSave;
				break;
			}
		}

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