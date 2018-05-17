#define GLEW_STATIC
#include <GL/glew.h>
#include "shprogram.h"
#include <GLFW/glfw3.h>
#include <SOIL.h>
#include <iostream>
#include <vector>
using namespace std;
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "CubeModel.h"

const GLuint WIDTH = 800, HEIGHT = 600;

// key buffer for camera movement
bool keys[1024];

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	cout << key << endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

// Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
GLfloat pitch = 0.0f; // elewacja
const GLfloat pitch_max = 80.0f, pitch_min = -80.0f;

GLfloat yaw = 0.0f; // azymut
const GLfloat yaw_max = 360.0f, yaw_min = 0.0f;

glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

void do_movement()
{
	// Camera controls
	GLfloat cameraSpeed = 25.0f * deltaTime;
	if (keys[GLFW_KEY_UP])
		if ((pitch += cameraSpeed) > pitch_max)
			pitch = pitch_max;
	if (keys[GLFW_KEY_DOWN])
		if ((pitch -= cameraSpeed) < pitch_min)
			pitch = pitch_min;
	if (keys[GLFW_KEY_LEFT])
		if ((yaw -= cameraSpeed) < yaw_min)
			yaw += yaw_max;
	if (keys[GLFW_KEY_RIGHT])
		if ((yaw += cameraSpeed) > yaw_max)
			yaw -= yaw_max;
	// cout << "Yaw: " << yaw << "  pitch: " << pitch << endl;
}

GLuint LoadMipmapTexture(GLuint texId, const char* fname)
{
	int width, height;
	unsigned char* image = SOIL_load_image(fname, &width, &height, 0, SOIL_LOAD_RGB);
	if (image == nullptr)
		throw exception("Failed to load texture file");

	GLuint texture;
	glGenTextures(1, &texture);

	glActiveTexture(texId);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

int main()
{
	if (glfwInit() != GL_TRUE)
	{
		cout << "GLFW initialization failed" << endl;
		return -1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	try
	{
		GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "GKOM - OpenGL 06", nullptr, nullptr);
		if (window == nullptr)
			throw exception("GLFW window not created");
		glfwMakeContextCurrent(window);
		glfwSetKeyCallback(window, key_callback);

		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK)
			throw exception("GLEW Initialization failed");

		glViewport(0, 0, WIDTH, HEIGHT);

		// Build, compile and link shader program
		ShaderProgram theProgram("gl_06.vert", "gl_06.frag");

		// Set up cube model
		CubeModel cube(1.0f);

		// Set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT (usually basic wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// Set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// prepare textures
		GLuint texture0 = LoadMipmapTexture(GL_TEXTURE0, "iipw.png");
		GLuint texture1 = LoadMipmapTexture(GL_TEXTURE1, "weiti.png");

		glEnable(GL_DEPTH_TEST); // let's use z-buffer

		// main event loop
		while (!glfwWindowShouldClose(window))
		{
			// Calculate deltatime of current frame
			GLfloat currentFrame = (GLfloat) glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
			glfwPollEvents();
			do_movement();

			// Clear the colorbuffer
			glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Bind Textures using texture units
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture0);
			glUniform1i(glGetUniformLocation(theProgram.get_programID(), "Texture0"), 0);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, texture1);
			glUniform1i(glGetUniformLocation(theProgram.get_programID(), "Texture1"), 1);

			theProgram.Use();
			GLuint modelLoc = glGetUniformLocation(theProgram.get_programID(), "model");
			GLuint viewLoc = glGetUniformLocation(theProgram.get_programID(), "view");
			GLuint projLoc = glGetUniformLocation(theProgram.get_programID(), "projection");

			// set perspective projection & pass it to GPU
			glm::mat4 projection = glm::perspective(45.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
			glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

			// set view transformation & pass it to GPU
			glm::mat4 view;
			view = glm::lookAt(cameraPos, glm::vec3(0.0, 0.0, 0.0), cameraUp);
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

			glm::mat4 trans;
			cube.Draw(trans, modelLoc);

			// Swap the screen buffers
			glfwSwapBuffers(window);
		}
	}
	catch (exception ex)
	{
		cout << ex.what() << endl;

		getchar(); getchar();
	}
	glfwTerminate();

	return 0;
}
