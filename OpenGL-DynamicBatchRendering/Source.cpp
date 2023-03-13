#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

struct Vec2
{
	float x, y;
};

struct Vec3
{
	float x, y, z;
};

struct Vec4
{
	float x, y, z, w;
};

struct Vertex {
	Vec3 position;
	Vec4 color;
	Vec2 texCoords;
	float texID;
};

struct ShaderProgramSource
{
	std::string VertexSource;
	std::string FragmentSource;
};

ShaderProgramSource ParseShader(const std::string& filepath)
{
	std::ifstream stream(filepath);

	enum class ShaderType
	{
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

	std::string line;
	//1 tane vertex için 1 tane de fragment için stringstream oluþturduk.
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;

	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
				type = ShaderType::VERTEX;
			else if (line.find("fragment") != std::string::npos)
				type = ShaderType::FRAGMENT;
		}
		else
		{
			ss[(int)type] << line << '\n';
		}
	}
	return { ss[0].str(), ss[1].str() };
}

unsigned int CompileShader(unsigned int type, const std::string& source)
{
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << "Failed to compile" << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "shader!" << std::endl;
		std::cout << message << std::endl;
		glDeleteShader(id);
		return 0;
	}
	return id;
}

unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	unsigned int program = glCreateProgram();
	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

static std::array<Vertex, 4> CreateQuad(float x, float y, float textureID)
{
	float size = 1.0f;

	Vertex v0;
	v0.position = { x, y, 0.0f };
	v0.color = { 0.18f, 0.6f, 0.96f, 1.0f };
	v0.texCoords = { 0.0f, 0.0f };
	v0.texID = textureID;

	Vertex v1;
	v1.position = { x+size, y, 0.0f };
	v1.color = { 0.18f, 0.6f, 0.96f, 1.0f };
	v1.texCoords = { 1.0f, 0.0f };
	v1.texID = textureID;

	Vertex v2;
	v2.position = { x+size,  y+size, 0.0f };
	v2.color = { 0.18f, 0.6f, 0.96f, 1.0f };
	v2.texCoords = { 1.0f, 1.0f };
	v2.texID = textureID;

	Vertex v3;
	v3.position = { x, y+size, 0.0f };
	v3.color = { 0.18f, 0.6f, 0.96f, 1.0f };
	v3.texCoords = { 0.0f, 1.0f };
	v3.texID = textureID;

	return { v0, v1, v2, v3 };
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(800, 600, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		std::cout << "SORUN WAR LA" << std::endl;
	}

	unsigned int VAO;
	glCreateVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	unsigned int VBO;
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 1000, nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexArrayAttrib(VBO, 0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

	glEnableVertexArrayAttrib(VBO, 1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, color));

	glEnableVertexArrayAttrib(VBO, 2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, texCoords));

	glEnableVertexArrayAttrib(VBO, 3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, texID));
	
	unsigned int indices[] =
	{
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
	};

	unsigned int IBO;
	glCreateBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//TEXTURE BUSINESS

	ShaderProgramSource source = ParseShader("Shader.glsl");
	unsigned int shaderID = CreateShader(source.VertexSource, source.FragmentSource);

	

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		//Set dynamic vertex buffer

		auto q0 = CreateQuad(-1.5f, -0.5f, 0.0f);
		auto q1 = CreateQuad( 0.5f, -0.5f, 1.0f);

		Vertex vertices[8];
		memcpy(vertices, q0.data(), q0.size() * sizeof(Vertex));
		memcpy(vertices + q0.size(), q1.data(), q1.size() * sizeof(Vertex));

		//YOU CAN LOOK INTO GLMAPBUFFER AND GLUNMAPBUFFER- for dynamicly loading data.
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shaderID);
		//glBindTextureUnit()
		glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, nullptr);
		
		glfwSwapBuffers(window);

		
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}