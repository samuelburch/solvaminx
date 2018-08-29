// Include standard headers

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

using namespace std;

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow *window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"
#include "shader.hpp"
#include "text2D.hpp"

int num;

vec3 colors[13] = {
	{0.890, 0.050, 0.050}, {0.890, 0.396, 0.050}, {0.968, 0.858, 0.211}, {0.211, 0.968, 0.270}, {0.211, 0.254, 0.968}, {1, 1, 1}, {0.890, 0.050, 0.050}, {0.890, 0.396, 0.050}, {0.968, 0.858, 0.211}, {0.211, 0.968, 0.270}, {0.211, 0.254, 0.968}, {0, 0, 0}, {.5, .5, .5}};

int edgemap[12 * 5] = {
	0, 1, 0, 3, 4,  // 0
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

typedef struct tie
{
	vec3 color;
	int icolor;
	vector<int> color_indices;
	vector<GLfloat> *color_buff;
	tile(){};
	vector<GLfloat> exportcolorindices()
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
		// switch (e)
		// {
		// case 0:
		// 	tiles[0]->icolor = c[0];
		// 	tiles[1]->icolor = c[1];
		// 	tiles[2]->icolor = c[2];
		// 	break;
		// case 1:
		// 	tiles[2]->icolor = c[0];
		// 	tiles[3]->icolor = c[1];
		// 	tiles[4]->icolor = c[2];
		// 	break;
		// case 2:
		// 	tiles[4]->icolor = c[0];
		// 	tiles[5]->icolor = c[1];
		// 	tiles[6]->icolor = c[2];
		// 	break;
		// case 3:
		// 	tiles[6]->icolor = c[0];
		// 	tiles[7]->icolor = c[1];
		// 	tiles[8]->icolor = c[2];
		// 	break;
		// case 4:
		// 	tiles[8]->icolor = c[0];
		// 	tiles[9]->icolor = c[1];
		// 	tiles[0]->icolor = c[2];
		// 	break;
		// }
	}

	void cc()
	{
		vector<int> temp;
		temp = getedge(0);
		setedge(0, getedge(4));
		setedge(1, getedge(0));
		setedge(2, getedge(1));
		setedge(3, getedge(2));
		setedge(4, temp);

		// temp = adjacent[0]->getedge(edgemap[f*5 + 0]);
		// adjacent[0]->setedge(edgemap[f*5 + 0],adjacent->getedge);
		temp = adjacent[0]->getedge(edgemap[f*5]);
		for (int i = 3; i > 0; i--){
			adjacent[i]->setedge(edgemap[f*5+i],adjacent[i-1]->getedge(edgemap[f*5 + i - 1]));
		}
		adjacent[4]->setedge(edgemap[f*5 + 4],temp);

		vector<int> temp2;

		// temp2 = adjacent[4]->getedge(edgemap[f * 5 + 4][1]);

		// adjacent[0]->setedge(edgemap[f * 5 + 3][1], temp2);
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
		// for (auto &t : tiles)
		// {
		// 	t->icolor = f;
		// }
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
		// faces = {face(tiles, 0), face(tiles, 1), face(tiles, 2), face(tiles, 3),
		// 		 face(tiles, 4), face(tiles, 5), face(tiles, 6), face(tiles, 7),
		// 		 face(tiles, 8), face(tiles, 9), face(tiles, 10), face(tiles,
		// 11)};
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
	ifstream file1("../resources/models/tiles.dat");
	vector<string> lines;
	if (file1.is_open())
	{
		while (getline(file1, line))
		{
			lines.push_back(line);
		}
		file1.close();
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
		for (int i = 0; ss >> a;)
		{
			t.color_indices.push_back(a);
			t.color = colors[cc];
		}
		cc++;
		tiles.push_back(t);
	}

	megaminx m(tiles);

	ifstream file2("../resources/models/megaminx.v");
	lines = {};
	line = "";
	if (file2.is_open())
	{
		while (getline(file2, line))
		{
			lines.push_back(line);
		}
		file2.close();
	}
	else
		cout << "Unable to open file";

	vector<GLfloat> vertex_buff;

	for (auto l : lines)
	{
		stringstream ss(l);
		float f;
		for (int i = 0; ss >> f;)
		{
			vertex_buff.push_back(f);
		}
	}

	// for (auto f : vertex_buff)
	// 	cout << f << endl;

	cout << tiles.size() << endl;
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,
				   GL_TRUE); // To make MacOS happy;
							 // should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Megaminx", NULL, NULL);
	if (window == NULL)
	{
		fprintf(stderr, "Failed to open GLFW window. If you "
						"have an Intel GPU, they are "
						"not 3.3 compatible. Try the 2.1 "
						"version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being
	// pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera
	// than the former one
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
	glm::mat4 Projection =
		glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	glm::mat4 View = glm::lookAt(glm::vec3(0, 0,
										   0), // Camera is at (4,3,-3), in
											   // World Space
								 glm::vec3(0, 0,
										   0), // and looks at the origin
								 glm::vec3(0, 1,
										   0) // Head is up (set to 0,-1,0 to
											  // look upside-down)
	);
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication
											   // is the other way around

	// One color for each vertex. They were
	// generated randomly.

	vector<GLfloat> color_buff(vertex_buff.size());

	// m.faces[0].setedge(2, m.faces[5].getedge(0));

	// m.faces[0].cc();

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

	int a = 0;
	int b = 0;

	int ca = -1;
	int fa = 10;
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

		// m.faces[center].tiles[10]->icolor = rand() % 10;
		// color_buff = m.exportcolorbuffer();

		// for (int i = 0; i <
		// sizeof(g_vertex_buffer_data) / 12
		// * 3; i++)
		// {
		// 	for (int j = 0; j < 3; j++)
		// 	{
		// 		float col = (float)(rand() % 1000)
		// / 1000; 		color_buff[i + j] = col;
		// 	}
		// }

		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);

		static int oldSpace = GLFW_RELEASE;
		int newSpace = glfwGetKey(window, GLFW_KEY_SPACE);
		if (newSpace == GLFW_RELEASE && oldSpace == GLFW_PRESS)
		{
			// if (ca == 4)
			// {
			// 	m.reset();
			// 	fa++;
			// 	ca = 0;
			// }
			// else
			// {
			// 	ca++;
			// }

			// m.faces[fa].adjacent[ca]->setedge(edgemap[fa * 5 + ca], {12, 12, 12});
			// color_buff = m.exportcolorbuffer();

			m.faces[0].cc();
			color_buff = m.exportcolorbuffer();

			// center++;
			// for (b = 0; b < 9; b++)
			// {
			// 	color_buff[a * 9 + b] = 0.0f;
			// }
			// a++;
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
		computeMatricesFromInputs();
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

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertex_buff.size() / 3);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		char text[256];
		sprintf(text, "%.2f sec", glfwGetTime());
		printText2D(text, 10, 500, 60);
		sprintf(text, "triangle %i", a);
		printText2D(text, 10, 300, 40);
		sprintf(text, "vertex %i", a * 3 + b);
		printText2D(text, 10, 200, 40);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the
	  // window was
	// closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &colorbuffer);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	cleanupText2D();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
