// Delaunay triangulation
#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <vector>

// GLEW - OpenGL Extension Wrangler
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW - OpenGL FrameWork
#include <GLFW/glfw3.h>
#include "shader.h"

const GLuint WIDTH = 800, HEIGHT = 600;
const float ORANGE[4] = { 1.0f, 0.549f, 0.0f, 1.0f };

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
GLFWwindow* initialize();


class Triangle {
public:
	int p1, p2, p3;

	Triangle() { }
};

class Edge {
public:
	int p1, p2;
	Edge() { p1 = -1; p2 = -1; }
};

class XYZ {
public:
	double x, y, z;

	XYZ() { }

	XYZ(double _x, double _y, double _z) : x(_x), y(_y), z(_z)
	{}
};

class Triangulate {

public:
	static double EPSILON;

	/*
	return TRUE if a point (xPoint,yPoint) is inside the circumcircle made up
	of the points (x1, y1), (x2, y2), (x3, y3)
	the circumcircle centre is returned in (xC,yC) and the radius
	(a point on the edge is inside the circumcircle)
	*/
	static bool isInCircle(double xPoint, double yPoint, double x1, double y1,
		double x2, double y2, double x3, double y3, XYZ *circle) {

		double m1, m2, mx1, mx2, my1, my2;
		double dx, dy, rsqr, drsqr;
		double xC, yC, radius;

		/* Check for coincident points */

		if (abs(y1 - y2) < EPSILON && abs(y2 - y3) < EPSILON) {
			std::cout << "isInCircle: points are coincident" << std::endl;
			return false;
		}

		if (abs(y2 - y1) < EPSILON) {
			m2 = -(x3 - x2) / (y3 - y2);
			mx2 = (x2 + x3) / 2.0;
			my2 = (y2 + y3) / 2.0;
			xC = (x2 + x1) / 2.0;
			yC = m2 * (xC - mx2) + my2;
		} else
			if (abs(y3 - y2) < EPSILON) {
				m1 = -(x2 - x1) / (y2 - y1);
				mx1 = (x1 + x2) / 2.0;
				my1 = (y1 + y2) / 2.0;
				xC = (x3 + x2) / 2.0;
				yC = m1 * (xC - mx1) + my1;
			}
			else {
				m1 = -(x2 - x1) / (y2 - y1);
				m2 = -(x3 - x2) / (y3 - y2);
				mx1 = (x1 + x2) / 2.0;
				mx2 = (x2 + x3) / 2.0;
				my1 = (y1 + y2) / 2.0;
				my2 = (y2 + y3) / 2.0;
				xC = (m1 * mx1 - m2 * mx2 + my2 - my1) / (m1 - m2);
				yC = m1 * (xC - mx1) + my1;
			}

			dx = x2 - xC;
			dy = y2 - yC;
			rsqr = dx * dx + dy * dy;
			radius = sqrt(rsqr);

			dx = xPoint - xC;
			dy = yPoint - yC;
			drsqr = dx * dx + dy * dy;

			circle->x = xC;
			circle->y = yC;
			circle->z = radius;

			return (drsqr <= rsqr ? true : false);
	}

	/*
	Triangulation subroutine
	Takes as input pointNumb vertices in array xyz
	These triangles are arranged in a consistent clockwise order.
	The triangle array "triangle" should be malloced to 3 * pointNumb
	The vertex array xyz must be big enough to hold 3 more points
	*/

	static int triangulate(int pointNumb, XYZ *xyz, Triangle *triangles) {
		bool *complete;
		Edge *edges;
		int edgeNumber = 0;
		int trianglesMax, edgesMax = pointNumb * 10;

		bool inside;
		double 	xPoint, yPoint, x1, y1, x2, y2, x3, y3, xC, yC, radius;
		double 	xMin, xMax, yMin, yMax, xMid, yMid;
		double 	dx, dy, dmax;

		int	triangleNumber = 0;

		// allocate memory for the completeness list, flag for each triangle
		trianglesMax = 4 * pointNumb;

		complete = new bool[trianglesMax];
		for (int i = 0; i < trianglesMax; i++)
			complete[i] = false;


		edges = new Edge[edgesMax];


		// find the maximum and minimum vertex bounds
		// this is to allow calculation of the bounding triangle

		xMin = xyz[0].x;
		yMin = xyz[0].y;
		xMax = xMin;
		yMax = yMin;

		for (int i = 1; i < pointNumb; i++) {
			if (xyz[i].x < xMin) xMin = xyz[i].x;
			if (xyz[i].x > xMax) xMax = xyz[i].x;
			if (xyz[i].y < yMin) yMin = xyz[i].y;
			if (xyz[i].y > yMax) yMax = xyz[i].y;
		}

		dx = xMax - xMin;
		dy = yMax - yMin;
		dmax = (dx > dy) ? dx : dy;
		xMid = (xMax + xMin) / 2.0;
		yMid = (yMax + yMin) / 2.0;

		/*
		Create the supertriangle
		This is a triangle which encompasses all the sample points
		The supertriangle coordinates are added to the end of the vertex list
		The supertriangle is the first triangle in the triangle list
		*/
		xyz[pointNumb + 0].x = xMid - 2.0 * dmax;
		xyz[pointNumb + 0].y = yMid - dmax;
		xyz[pointNumb + 0].z = 0.0;
		xyz[pointNumb + 1].x = xMid;
		xyz[pointNumb + 1].y = yMid + 2.0 * dmax;
		xyz[pointNumb + 1].z = 0.0;
		xyz[pointNumb + 2].x = xMid + 2.0 * dmax;
		xyz[pointNumb + 2].y = yMid - dmax;
		xyz[pointNumb + 2].z = 0.0;
		triangles[0].p1 = pointNumb;
		triangles[0].p2 = pointNumb + 1;
		triangles[0].p3 = pointNumb + 2;

		complete[0] = false;
		triangleNumber = 1;



		// include each point one at a time into the existing mesh
		for (int i = 0; i < pointNumb; i++) {

			xPoint = xyz[i].x;
			yPoint = xyz[i].y;
			edgeNumber = 0;


			/*
			Set up the edge buffer.
			If the point (xP, yP) lies inside the circumcircle then the
			three edges of that triangle are added to the edge buffer
			and that triangle is removed.
			*/
			XYZ *circle = new XYZ;
			for (int j = 0; j < triangleNumber; j++) {
				if (complete[j])
					continue;

				x1 = xyz[triangles[j].p1].x;
				y1 = xyz[triangles[j].p1].y;
				x2 = xyz[triangles[j].p2].x;
				y2 = xyz[triangles[j].p2].y;
				x3 = xyz[triangles[j].p3].x;
				y3 = xyz[triangles[j].p3].y;

				inside = isInCircle(xPoint, yPoint, x1, y1, x2, y2, x3, y3, circle);

				xC = circle->x; yC = circle->y; radius = circle->z;

				if (xC + radius < xPoint) complete[j] = true;

				if (inside) {
					// Check that we haven't exceeded the edge list size
					if (edgeNumber + 3 >= edgesMax) {
						edgesMax += 100;
						Edge *edges_n = new Edge[edgesMax];

						edges = edges_n;
					}

					edges[edgeNumber + 0].p1 = triangles[j].p1;
					edges[edgeNumber + 0].p2 = triangles[j].p2;
					edges[edgeNumber + 1].p1 = triangles[j].p2;
					edges[edgeNumber + 1].p2 = triangles[j].p3;
					edges[edgeNumber + 2].p1 = triangles[j].p3;
					edges[edgeNumber + 2].p2 = triangles[j].p1;
					edgeNumber += 3;
					triangles[j].p1 = triangles[triangleNumber - 1].p1;
					triangles[j].p2 = triangles[triangleNumber - 1].p2;
					triangles[j].p3 = triangles[triangleNumber - 1].p3;
					complete[j] = complete[triangleNumber - 1];
					triangleNumber--;
					j--;
				}
			}


			// tag multiple edges
			for (int j = 0; j < edgeNumber - 1; j++) {
				for (int k = j + 1; k < edgeNumber; k++) {
					if ((edges[j].p1 == edges[k].p2) && (edges[j].p2 == edges[k].p1)) {
						edges[j].p1 = -1;
						edges[j].p2 = -1;
						edges[k].p1 = -1;
						edges[k].p2 = -1;
					}
				}
			}

			/*
			Form new triangles for the current point
			skip any tagged edges.
			all edges are arranged in clockwise order.
			*/
			for (int j = 0; j < edgeNumber; j++) {
				if (edges[j].p1 == -1 || edges[j].p2 == -1)
					continue;

				if (triangleNumber >= trianglesMax) return -1;
				triangles[triangleNumber].p1 = edges[j].p1;
				triangles[triangleNumber].p2 = edges[j].p2;
				triangles[triangleNumber].p3 = i;

				complete[triangleNumber] = false;
				triangleNumber++;
			}
		}


		/*
		Remove triangles with supertriangle vertices
		These are triangles which have a vertex number greater than pointNumb
		*/
		for (int i = 0; i < triangleNumber; i++) {
			if (triangles[i].p1 >= pointNumb || triangles[i].p2 >= pointNumb || triangles[i].p3 >= pointNumb) {
				triangles[i] = triangles[triangleNumber - 1];
				triangleNumber--;
				i--;
			}
		}

		return triangleNumber;
	}
};

double Triangulate::EPSILON = 0.000001;

int main() {
	int pointNumber = 10;
	float max = FLT_MIN;

	std::vector<float> coord;

	std::cout << "Creating " << pointNumber << " random points" << std::endl;

	XYZ *points = new XYZ[pointNumber + 3];

	srand(time(0));

	for (int i = 0; i < pointNumber + 3; i++) {
		points[i].x = i * 20.0;
		points[i].y = rand() % 100 * 2;
		points[i].z = 0.0;
	}

	Triangle *triangles = new Triangle[pointNumber * 3];

	int triangleNumber = Triangulate::triangulate(pointNumber, points, triangles);


	for (int i = 0; i < triangleNumber; i++) {
		coord.push_back(static_cast<float>(points[triangles[i].p1].x));
		coord.push_back(static_cast<float>(points[triangles[i].p1].y));
		coord.push_back(static_cast<float>(0.0f));

		coord.push_back(static_cast<float>(points[triangles[i].p2].x));
		coord.push_back(static_cast<float>(points[triangles[i].p2].y));
		coord.push_back(static_cast<float>(0.0f));

		coord.push_back(static_cast<float>(points[triangles[i].p3].x));
		coord.push_back(static_cast<float>(points[triangles[i].p3].y));
		coord.push_back(static_cast<float>(0.0f));
	}

	for (auto iter = coord.begin(); iter != coord.end(); ++iter)
		if (*iter > max)
			max = *iter;

	for (auto i = 0; i < coord.size(); i++) {
		coord[i] /= max;
		coord[i] -= 0.5;
		coord[i] *= 2;
	}

	auto window = initialize();

	Shader shader("vertex.txt", "fragment.txt");

	GLuint VAO;
	GLuint VBO;

	while (!glfwWindowShouldClose(window)) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glfwPollEvents();

		glClearColor(0.7529f, 0.7529f, 0.7529f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		auto colorLocation = glGetUniformLocation(shader.Program, "color");
		glUniform4fv(colorLocation, 1, ORANGE);

		shader.Use();

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, coord.size() * sizeof(GLfloat), &coord[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), static_cast<GLvoid*>(nullptr));
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, coord.size());
		glBindVertexArray(0);

		glBindVertexArray(0);
		glfwSwapBuffers(window);
	}

	return 0;
}

GLFWwindow* initialize() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	auto window = glfwCreateWindow(WIDTH, HEIGHT, "Polygon Intersection", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);

	glewExperimental = GL_TRUE;
	glewInit();
	glViewport(0, 0, WIDTH, HEIGHT);

	return window;
}

void key_callback(GLFWwindow* window, int key, int, int action, int) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}
