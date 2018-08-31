#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

using namespace std;

#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow *window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"
#include "shader.hpp"
#include "text2D.hpp"

const vec3 colors[12] = {{0.968, 0.900, 0.111},  // yellow
						 {0.364, 0.019, 0.501},  // purple
						 {0.078, 0.301, 0.078},  // green
						 {0.858, 0, 0.086},		 // red
						 {0, 0.019, 0.521},		 // blue
						 {1, 1, 1},				 // white
						 {0.137, 0.670, 0.882},  // light blue
						 {1, 0.458, 0.078},		 // orange
						 {0.078, 1, 0.168},		 // lime green
						 {1, 0.980, 0.580},		 // pastel yellow
						 {0.917, 0.364, 0.784},  // pink
						 {0.820, 0.820, 0.820}}; // grey

const int edgemap[12 * 5] = {0, 1, 0, 3, 4,  // 0
							 0, 1, 1, 3, 4,  // 1
							 0, 1, 2, 3, 4,  // 2
							 0, 1, 3, 3, 4,  // 3
							 0, 1, 4, 3, 4,  // 4
							 2, 2, 2, 2, 2,  // 5
							 4, 3, 4, 1, 0,  // 6
							 4, 3, 3, 1, 0,  // 7
							 4, 3, 2, 1, 0,  // 8
							 4, 3, 1, 1, 0,  // 9
							 4, 3, 0, 1, 0,  // 10
							 2, 2, 2, 2, 2}; // 11

typedef struct tile
{
	vec3 color;
	int icolor;
	vector<int> color_indices;
	vector<GLfloat> *color_buff;
	tile(){};
	void exportcolorindices()
	{
		color = colors[icolor];
		for (auto &i : color_indices)
		{
			for (int a = 0; a < 3; a++)
			{
				(*color_buff)[(i - 1) * 9 + a * 3] = color.r;
				(*color_buff)[(i - 1) * 9 + a * 3 + 1] = color.g;
				(*color_buff)[(i - 1) * 9 + a * 3 + 2] = color.b;
			}
		}
	}
} tile;

typedef vector<tile> _tiles;

typedef struct face
{
	vector<face *> adjacent;
	vector<tile *> tiles;
	int f;
	face(vector<tile> &_tiles, int c)
	{
		f = c;
		for (int i = 0; i < 11; i++)
		{
			tiles.push_back(&_tiles[c * 11 + i]);
		}
		for (int i = 0; i < 11; i++)
			tiles[i]->icolor = c;
	}

	vector<int> getedge(int e)
	{
		vector<int> temp;
		temp.push_back(tiles[e * 2]->icolor);
		temp.push_back(tiles[e * 2 + 1]->icolor);
		if (e == 4)
			temp.push_back(tiles[0]->icolor);
		else
			temp.push_back(tiles[e * 2 + 2]->icolor);
		return temp;
	}

	void setedge(int e, vector<int> c)
	{

		tiles[e * 2]->icolor = c[0];
		tiles[e * 2 + 1]->icolor = c[1];
		if (e == 4)
			tiles[0]->icolor = c[2];
		else
			tiles[e * 2 + 2]->icolor = c[2];
	}

	void cc()
	{
		vector<int> temp;
		temp = getedge(0);
		setedge(0, getedge(1));
		setedge(1, getedge(2));
		setedge(2, getedge(3));
		setedge(3, getedge(4));
		setedge(4, temp);

		temp = adjacent[4]->getedge(edgemap[f * 5 + 4]);

		for (int z = 4; z > 0; z--)
			adjacent[z]->setedge(edgemap[f * 5 + z],
								 adjacent[z - 1]->getedge(edgemap[f * 5 + (z - 1)]));
		adjacent[0]->setedge(edgemap[f * 5 + 0], temp);
	}

	void connectfaces(face *face1, face *face2, face *face3, face *face4,
					  face *face5)
	{
		adjacent.push_back(face1);
		adjacent.push_back(face2);
		adjacent.push_back(face3);
		adjacent.push_back(face4);
		adjacent.push_back(face5);
	}

	void reset()
	{
		for (int i = 0; i < 11; i++)
		{
			tiles[i]->icolor = f;
		}
	}

} face;

typedef struct megaminx
{
	vector<face> faces;
	vector<tile> tiles;
	vector<GLfloat> color_buff;
	megaminx(vector<tile> _tiles)
	{
		tiles = _tiles;
		for (auto &t : tiles)
			t.color_buff = &color_buff;
		for (int i = 0; i < 12; i++)
		{
			for (int j = 0; j < 11; j++)
			{
				tiles[i * 11 + j].color = colors[i];
			}
		}
		color_buff = vector<GLfloat>(2160);
		for (int i = 0; i < 12; i++)
			faces.push_back(face(tiles, i));
		faces[0].connectfaces(&faces[8], &faces[4], &faces[5], &faces[1],
							  &faces[7]);
		faces[1].connectfaces(&faces[7], &faces[0], &faces[5], &faces[2],
							  &faces[6]);
		faces[2].connectfaces(&faces[6], &faces[1], &faces[5], &faces[3],
							  &faces[10]);
		faces[3].connectfaces(&faces[10], &faces[2], &faces[5], &faces[4],
							  &faces[9]);
		faces[4].connectfaces(&faces[9], &faces[3], &faces[5], &faces[0],
							  &faces[8]);
		faces[5].connectfaces(&faces[4], &faces[3], &faces[2], &faces[1],
							  &faces[0]);
		faces[6].connectfaces(&faces[2], &faces[10], &faces[11], &faces[7],
							  &faces[1]);
		faces[7].connectfaces(&faces[1], &faces[6], &faces[11], &faces[8],
							  &faces[0]);
		faces[8].connectfaces(&faces[0], &faces[7], &faces[11], &faces[9],
							  &faces[4]);
		faces[9].connectfaces(&faces[4], &faces[8], &faces[11], &faces[10],
							  &faces[3]);
		faces[10].connectfaces(&faces[3], &faces[9], &faces[11], &faces[6],
							   &faces[2]);
		faces[11].connectfaces(&faces[10], &faces[9], &faces[8], &faces[7],
							   &faces[6]);
	}

	vector<GLfloat> exportcolorbuffer()
	{
		for (auto &t : tiles)
			t.exportcolorindices();
		return color_buff;
	}

	void reset()
	{
		for (auto &f : faces)
			f.reset();
	}

} megaminx;

int main(void)
{

	string line;
	ifstream file("../resources/models/tiles.dat");
	vector<string> lines;
	if (file.is_open())
	{
		while (getline(file, line))
		{
			lines.push_back(line);
		}
		file.close();
	}
	else
		cout << "Unable to open file";

	_tiles tiles;
	int cc = 0;
	for (auto l : lines)
	{
		stringstream ss(l);
		tile t;
		int a;
		for (; ss >> a;)
		{
			t.color_indices.push_back(a);
			t.color = colors[cc];
		}
		cc++;
		tiles.push_back(t);
	}

	megaminx m(tiles);

	file.open("../resources/models/megaminx2.v");
	lines = {};
	line = "";
	if (file.is_open())
	{
		while (getline(file, line))
		{
			lines.push_back(line);
		}
		file.close();
	}
	else
		cout << "Unable to open file";

	vector<GLfloat> vertex_buff;

	for (auto l : lines)
	{
		stringstream ss(l);
		float f;
		for (; ss >> f;)
		{
			vertex_buff.push_back(f);
		}
	}

	cout << tiles.size() << endl;

	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 16);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Megaminx", NULL, NULL);
	if (window == NULL)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	//vsync
	glfwSwapInterval(1);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glClearColor(0.0f, 0.0f, 0.4f, 0.0f); // dark blue
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the
	// shaders
	GLuint programID = LoadShaders("../resources/shaders/"
								   "TransformVertexShader.vertexshader",
								   "../resources/shaders/"
								   "ColorFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	vector<GLfloat> color_buff(vertex_buff.size());
	vector<GLfloat> color_buff_black(vertex_buff.size(), 0);

	color_buff = m.exportcolorbuffer();

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buff.size() * sizeof(GLfloat),
				 &vertex_buff[0], GL_STATIC_DRAW);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, color_buff.size() * sizeof(GLfloat),
				 &color_buff[0], GL_STATIC_DRAW);

	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	initText2D("../resources/fonts/Ariel.DDS");

	double lastTime = glfwGetTime();
	int nbFrames = 0;

	int currentface;

	vec3 camerapos;

#if 0
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
#else
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

	do
	{
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0)
		{
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);

		static int oldSpace = GLFW_RELEASE;
		int newSpace = glfwGetKey(window, GLFW_KEY_SPACE);
		if (newSpace == GLFW_RELEASE && oldSpace == GLFW_PRESS)
		{
			for (int i = 0; i < 1000; i++)
				m.faces[rand() % 12].cc();
			color_buff = m.exportcolorbuffer();
		}
		oldSpace = newSpace;

		static int oldAlt = GLFW_RELEASE;
		int newAlt = glfwGetKey(window, GLFW_KEY_RIGHT_ALT);
		if (newAlt == GLFW_RELEASE && oldAlt == GLFW_PRESS)
		{
			m.reset();
			color_buff = m.exportcolorbuffer();
		}
		oldAlt = newAlt;

		glBufferData(GL_ARRAY_BUFFER, color_buff.size() * sizeof(GLfloat),
					 &color_buff[0], GL_STATIC_DRAW);

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and
		// mouse input
		computeMatricesFromInputs(&currentface, &camerapos);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently
		// bound shader, in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		// 2nd attribute buffer : colors
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		//TRIANGLES
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glDrawArrays(GL_TRIANGLES, 0, vertex_buff.size() / 3);

		//WIRES
		// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// glDrawArrays(GL_TRIANGLES, 0, vertex_buff.size() / 3);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		char text[256];
		sprintf(text, "%.2f sec", glfwGetTime());
		printText2D(text, 10, 500, 60);
		sprintf(text, "Current face: %i", currentface);
		printText2D(text, 10, 440, 30);
		sprintf(text, "x: %f", camerapos.x);
		printText2D(text, 10, 410, 30);
		sprintf(text, "y: %f", camerapos.y);
		printText2D(text, 10, 380, 30);
		sprintf(text, "z: %f", camerapos.z);
		printText2D(text, 10, 350, 30);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
			 glfwWindowShouldClose(window) == 0);

	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &colorbuffer);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	cleanupText2D();

	glfwTerminate();

	return 0;
}
