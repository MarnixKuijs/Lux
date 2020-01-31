#include "Scene.h"
#include "Ray.h"
#include "Color.h"
#include "Light.h"
#include "Camera.h"

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <cstdint>
#include <array>
#include <string>

constexpr int32_t screenWidth = 512;
constexpr int32_t screenHeight = 512;

std::array<glm::vec3, screenWidth * screenHeight> image;

void processInput(GLFWwindow* window);

int main()
{
	if (!glfwInit())
		return 1;

	auto window = glfwCreateWindow(screenWidth, screenHeight, "Lux", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		return 1;
	}

	glfwMakeContextCurrent(window);

	int framebufferWidth, framebufferHeight;
	glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	const char* vertexShaderSource = R"(#version 400

out vec2 UV;

void main() 
{
    UV = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(UV * 2.0f + -1.0f, 0.0f, 1.0f);
	//UV = vec2(invertedUV.x, 0.0 + (1.0 - invertedUV.y));
})"; 

	const char* fragmentShaderSource = R"(#version 400

in vec2 UV;
uniform sampler2D image;

out vec4 fragColor;

void main() 
{
	fragColor = texture(image, UV);
})";

	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glUseProgram(shaderProgram);

	Object ground
	{
		Sphere{glm::vec3{0.0f, -100.5f, 0.0f}, 100.f},
		new Material{Color::green, 0.0f}
	};

	Object diffuseSphere
	{
		Sphere{glm::vec3{0.0f, 0.0f, 0.0f}, 0.5f},
		new Material{Color::white, 0.0f}
	};


	Object metalicSphere
	{
		Sphere{glm::vec3{1.0f, 0.0f, 0.0f}, 0.5f},
		new Material{glm::vec3{0.8, 0.8, 0.8}, 1.0f}
	};

	PointLight light1
	{
		glm::vec3{-25.0f, 5.0f, 0.0f},
		Color::white * 1000.f
	};

	PointLight light2
	{
		glm::vec3{25.0f, 5.0f, 0.0f},
		Color::white * 1000.f
	};

	Scene scene;

	scene.objects.push_back(ground);
	scene.objects.push_back(diffuseSphere);
	scene.objects.push_back(metalicSphere);
	scene.lights.push_back(light1);
	scene.lights.push_back(light2);


	Camera camera{glm::vec3{0.0f, 0.0f, 1.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, 90 , static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight) };

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		const float cameraSpeed = 0.05f; 
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.position += cameraSpeed * glm::vec3{ 0.0f, 0.0f, -1.0f };
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.position -= cameraSpeed * glm::vec3{ 0.0f, 0.0f, -1.0f };
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.position -= glm::normalize(glm::cross(glm::vec3{ 0.0f, 0.0f, -1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f })) * cameraSpeed;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.position += glm::normalize(glm::cross(glm::vec3{ 0.0f, 0.0f, -1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f })) * cameraSpeed;

		camera = Camera{ camera.position, camera.position - glm::vec3{0.0f, 0.0f, 1.0f}, 90, static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight) };

		for (int y{ 0 }; y < framebufferHeight; ++y)
		{
			for (int x{ 0 }; x < framebufferWidth; ++x)
			{
				auto pixelIndex = x + framebufferWidth * y;

				float u = static_cast<float>(x) / static_cast<float>(framebufferWidth);
				float v = static_cast<float>(y) / static_cast<float>(framebufferHeight);

				glm::vec3 screenPoint = camera.lower_left_corner + u * camera.horizontal + v * camera.vertical;

				Ray ray{ camera.position, glm::normalize(screenPoint - camera.position) };

				image[pixelIndex] = Trace(scene, ray);
			}
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, framebufferWidth, framebufferHeight, 0, GL_RGB, GL_FLOAT, image.data());

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}