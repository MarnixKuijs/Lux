#include "Scene.h"
#include "Ray.h"

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <cstdint>
#include <array>
#include <iostream>

constexpr int32_t screenWidth = 1280;
constexpr int32_t screenHeight = 720;

std::array<glm::vec3, screenWidth * screenHeight> image;

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

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	const char* vertexShaderSource = R"(#version 400

out vec2 UV;

void main() 
{
    vec2 invertedUV = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(invertedUV * 2.0f + -1.0f, 0.0f, 1.0f);
	UV = vec2(invertedUV.x, 0.0 + (1.0 - invertedUV.y));
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

	Scene scene;

	Object object1
	{
		Sphere{glm::vec3{0.0, 0.0, 2.0}, 0.5f}
	};

	Object object2
	{
		Sphere{glm::vec3{-1.0, -1.0, 1.0}, 0.5f}
	};

	scene.objects.push_back(object1);
	scene.objects.push_back(object2);

	float fov = 1;
	glm::vec3 cameraPosition{ 0.0f, 0.0f, 0.0f };
	glm::vec3 viewDirection{ 0.0f, 0.0f, 1.0f };
	glm::vec3 screenCenter{ cameraPosition + fov * viewDirection };
	glm::vec3 corner1 = screenCenter + glm::vec3{-1.0, 1.0, 0.0};
	glm::vec3 corner2 = screenCenter + glm::vec3{1.0, 1.0, 0.0};
	glm::vec3 corner3 = screenCenter + glm::vec3{-1.0, -1.0, 0.0};


	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		for (int y{ 0 }; y < framebufferHeight; ++y)
		{
			for (int x{ 0 }; x < framebufferWidth; ++x)
			{
				auto pixelIndex = x + framebufferWidth * y;
				auto& currentPixel = image[pixelIndex];

				float u = static_cast<float>(x) / static_cast<float>(framebufferWidth);
				float v = static_cast<float>(y) / static_cast<float>(framebufferHeight);

				glm::vec3 screenPoint = corner1 + u * (corner2 - corner1) + v * (corner3 - corner1);

				Ray ray{ cameraPosition, glm::normalize(screenPoint - cameraPosition) };

				image[pixelIndex] = Trace(scene, ray);
			}
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, framebufferWidth, framebufferHeight, 0, GL_RGB, GL_FLOAT, image.data());

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
	}
}