// HedgeHog
#include <iostream>

//GLM
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include <SOIL/SOIL.h>

// Other includes
#include "Shader.h"
#include <vector>
#include <random>
#include <ctime>

const GLchar* grass_pic = "grass.jpg";
const GLchar* hedgehog_pic = "hedgehog.png";
const GLchar* apple_pic = "apple.png";
const GLchar* congrats_pic = "congrats.png";

// Java Style
struct Ratios {
	static const double GRASS;
	static const double HEDGEHOG;
	static const double APPLE;
};

const double Ratios::GRASS = 1.0;
const double Ratios::HEDGEHOG = 0.15;
const double Ratios::APPLE = 0.075;

enum DIRECTION {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

const auto DEF_POS = glm::vec3();
const auto SPEED = 2.0f;
const auto APPLE_NUM = 20;
const glm::vec3 front = glm::vec3(0.0, 0.5, 0.0);
const glm::vec3 right = glm::vec3(0.5, 0.0, 0.0);

auto deltaTime = 0.0f;
auto lastFrame = 0.0f;
auto left = true;
auto win = false;

std::vector<glm::vec3> applePositions;

// Window dimensions
const GLuint WIDTH = 1000, HEIGHT = 750;
glm::vec3 origin = glm::vec3();
glm::vec3 hedgehogOrigin = glm::vec3();

GLFWwindow* initialize();
GLuint bindTexture(const GLchar* path);
void useTexture(GLuint& texture, Shader& shader);
void renderRectangle();
void drawRectangle(glm::vec3 &position, const GLint transLoc, double ratio);
void keyboard(DIRECTION direction, GLfloat deltaTime);
void move();
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// The MAIN function, from here we start the application and run the game loop
int main() {
	auto window = initialize();

	// Build and compile our shader program
	Shader shader("vertex.txt", "fragment.txt");

	// Load and create a textures
	auto apple = bindTexture(apple_pic);
	auto grass = bindTexture(grass_pic);
	auto hedgehog = bindTexture(hedgehog_pic);
	auto congrats = bindTexture(congrats_pic);

	std::default_random_engine dre;
	dre.seed(time(nullptr));
	std::uniform_real_distribution<double> dis(-0.9, 0.9);

	// Generate apples
	for (auto i = 0; i < APPLE_NUM; ++i) {
		auto position = glm::vec3(dis(dre), dis(dre), 0.0);
		
		// Check if apples are not too close to each other and do not overlap
		for (auto j = applePositions.begin(); j != applePositions.end(); ++j) {
			if (distance(position, *j) < 0.2) {
				position = glm::vec3(dis(dre), dis(dre), 0.0);
				j = applePositions.begin();
			}
		}

		applePositions.push_back(position);
	}
	
	// Game loop
	while (!glfwWindowShouldClose(window)) {

		// Count time of each frame to keep FPS stable
		auto currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		move();

		// Render
		// Clear the colorbuffer
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		auto transLoc = glGetUniformLocation(shader.Program, "transform");

		// Draw background
		useTexture(grass, shader);
		drawRectangle(origin, transLoc, Ratios::GRASS);

		// Draw apples 
		useTexture(apple, shader);

		std::vector<glm::vec3> newPositions;
		for (auto i : applePositions) {
			// Check if hedgehog is close to apple, if so, remove
			if (distance(i, hedgehogOrigin) > 0.15)
				newPositions.push_back(i);
			drawRectangle(i, transLoc, Ratios::APPLE);
		}

		// Update apple vector
		applePositions = newPositions;

		if (applePositions.empty())
			win = true;

		// Draw hedgehog
		useTexture(hedgehog, shader);
		drawRectangle(hedgehogOrigin, transLoc, Ratios::HEDGEHOG);

		// Victory picture
		if (win == true) {
			useTexture(congrats, shader);
			drawRectangle(origin, transLoc, Ratios::GRASS);
		}

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}
	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
}

// Keys array to keep track of pressed keys
bool keys[1024] = {};

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (action == GLFW_PRESS)
		keys[key] = true;
	if (action == GLFW_RELEASE)
		keys[key] = false;
}

GLFWwindow* initialize() {
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	auto window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, WIDTH, HEIGHT);

	return window;
}

GLuint bindTexture(const GLchar* path) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height;
	auto image = SOIL_load_image(path, &width, &height, nullptr, SOIL_LOAD_RGBA);

	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);

	return textureID;
}

void useTexture(GLuint& texture, Shader& shader) {
	// Bind Textures using texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(shader.Program, "ourTexture"), 0);
	
	// Activate shader
	shader.Use();
}

// Set up vertex data (and buffer(s)) and attribute pointers
GLuint rectangleVAO = 0;
GLuint rectangleVBO = 0;
GLuint rectangleEBO = 0;

void renderRectangle() {
	if (rectangleVAO == 0) {
		GLfloat vertices[] = {
			// Positions          // Colors           // Texture Coords
			1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Top Right
			1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // Bottom Right
			-1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Bottom Left
			-1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // Top Left 
		};

		GLuint indices[] = {  // Note that we start from 0!
			0, 1, 3,		  // First Triangle
			1, 2, 3           // Second Triangle
		};

		glGenVertexArrays(1, &rectangleVAO);
		glGenBuffers(1, &rectangleVBO);
		glGenBuffers(1, &rectangleEBO);

		glBindVertexArray(rectangleVAO);

		glBindBuffer(GL_ARRAY_BUFFER, rectangleVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectangleEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), static_cast<GLvoid*>(0));
		glEnableVertexAttribArray(0);
		// Color attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<GLvoid*>(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		// TexCoord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<GLvoid*>(6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0); // Unbind VAO
	}

	// Render Rectangle
	glBindVertexArray(rectangleVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void move() {
	if (keys[GLFW_KEY_W] == true)
		keyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S] == true)
		keyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A] == true)
		keyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D] == true)
		keyboard(RIGHT, deltaTime);
}

void keyboard(DIRECTION direction, GLfloat deltaTime) {
	auto velocity = SPEED * deltaTime;

	if (direction == FORWARD)
		hedgehogOrigin += front * velocity;
	if (direction == BACKWARD)
		hedgehogOrigin -= front * velocity;

	// if hedgehog changes direction from left to right
	// or vice versa flip the texture horizontally
	if (direction == LEFT) {
		hedgehogOrigin -= right * velocity;
		left = true;
	}
	if (direction == RIGHT) {
		left = false;
		hedgehogOrigin += right * velocity;
	}
}

void drawRectangle(glm::vec3 &position, const GLint transLoc, double ratio) {

	// Wrap the screen
	if (position.y > 1 || position.y < -1 )
		position.y = -position.y;
	if (position.x > 1 || position.x < -1)
		position.x = -position.x;

	// Transform matrix for changing position of a hedgehog
	glm::mat4 transform;
	transform = translate(transform, position);

	// If hedgehog changes direction change size of texture respectively
	if (ratio == Ratios::HEDGEHOG && left == false)
		transform = scale(transform, glm::vec3(-ratio, ratio, 0.0));
	else 
		transform = scale(transform, glm::vec3(ratio));	

	glUniformMatrix4fv(transLoc, 1, GL_FALSE, value_ptr(transform));
	renderRectangle();
}
