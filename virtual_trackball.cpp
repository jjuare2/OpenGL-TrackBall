#include <iostream>
#include <fstream>
using namespace std;

#include <stdlib.h>
#include <math.h>

#include <sys/types.h>
#include <GL/freeglut.h>

#include "pch.h"
#include "objLoader.h"

#define KEY_LEFT 100
#define KEY_UP 101
#define KEY_RIGHT 102
#define KEY_DOWN 103

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

int winWidth = 1024;
int winHeight = 1024;
bool firstTime = true;

WavefrontObj *obj_data;

// Trackball parameters initialization 
float angle = 0.0, axis[3], trans[3];

bool trackingMouse = false;
bool redrawContinue = false;
bool trackballMove = false;

float lastPos[3] = { 0.0, 0.0, 0.0 };
int curX, curY;
int startX, startY;


// Translation & Rotation
float x_trans = 0.0f; // translate object in x direction
float y_trans = 0.0f; // translate object in y direction
float zoom = 1.0f; // zoom for scaling



void Init(int w, int h)
{
	glViewport(0, 0, w, h);
	glShadeModel(GL_SMOOTH);								// Set Smooth Shading 
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);			    	// Background Color 
	glClearDepth(1.0f);										// Depth buffer setup 
	glEnable(GL_DEPTH_TEST);								// Enables Depth Testing 
	glDepthFunc(GL_LEQUAL);									// The Type Of Depth Test To Do 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);		// Use perspective correct interpolation if available

	glMatrixMode(GL_PROJECTION);							// Select The Projection Matrix
	glLoadIdentity();										// Reset The Projection Matrix
	double aspect = (double)h / w;
	glFrustum(-5, 5, -5 * aspect, 5 * aspect, 10, 500);          // Define perspective projection frustum
	//gluPerspective(30, w/h, 10, 74);
	glTranslated(0.0, 0.0, -24);                          // Viewing transformation

	glMatrixMode(GL_MODELVIEW);								// Select The Modelview Matrix
	glLoadIdentity();										// Reset The Modelview Matrix

	if (firstTime)
	{
		glEnable(GL_LIGHTING);
		float frontColor[] = { 0.2f, 0.7f, 0.7f, 1.0f };

		glMaterialfv(GL_FRONT, GL_AMBIENT, frontColor);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, frontColor);
		glMaterialfv(GL_FRONT, GL_SPECULAR, frontColor);
		glMaterialf(GL_FRONT, GL_SHININESS, 100);

		float lightDirection[] = { 2.0f, 0.0f, 1.0f, 0 };
		float ambientIntensity[] = { 0.1f, 0.1f, 0.1f, 1.0f };
		float lightIntensity[] = { 0.9f, 0.9f, 0.9f, 1.0f };
		float lightSpec[] = { 1.0f, 1.0f, 1.0f, 1 };
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambientIntensity);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);
		glLightfv(GL_LIGHT0, GL_POSITION, lightDirection);
		glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);
		glEnable(GL_LIGHT0);
		firstTime = false;
	}
}


void Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
	glLoadIdentity();
	glTranslatef(x_trans, 0.0, 0.0);
	glTranslatef(0.0, y_trans, 1.0);
	glScalef(zoom, zoom, 1.0f);

	if (trackballMove) {
		glRotatef(angle, axis[0], axis[1], axis[2]);
	}

	if (obj_data != NULL)
		obj_data->Draw();
	else
		glutSolidTeapot(1.0);	//draw a teapot when no argument is provided
	glFlush();
	glutSwapBuffers();
	glutPostRedisplay();
}

void trackball_proj(int x, int y, int w, int h, float v[3]) {
	float a, d;
	v[0] = (2.0*x - w) / w;
	v[1] = (h - 2.0F*y) / h;
	d = sqrt(v[0] * v[0] + v[1] * v[1]);
	v[2] = cos((M_PI / 2.0) * ((d < 1.0) ? d : 1.0));
	a = 1.0 / sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] *= a;
	v[1] *= a;
	v[2] *= a;
}
void start(int x, int y) {
	trackingMouse = true;
	redrawContinue = false;
	startX = x;
	startY = y;
	curX = x;
	curY = y;
	trackball_proj(x, y, winWidth, winHeight, lastPos);
	trackballMove = true;
}

void stop(int x, int y) {
	trackingMouse = false;
	if (startX != startY) {
		redrawContinue = true;
	}
	else {
		angle = 0.0;
		redrawContinue = false;
		trackballMove = false;
	}
}

void Specialkey(int key, int x, int y)
{
	if (key == KEY_UP) {
		y_trans += 0.5;
	}
	else if (key == KEY_DOWN) {
		y_trans -= 0.5;
	}
	else if (key == KEY_LEFT) {
		x_trans -= 0.5;
	}
	else if (key == KEY_RIGHT) {
		x_trans += 0.5;
	}
	glutPostRedisplay();
}

void onKeyPress(unsigned char key, int x, int y)
{

	if (key == 'p')
	{
		obj_data->mode = GL_LINE_LOOP;
	}
	else if (key == 's')
	{
		glShadeModel(GL_SMOOTH);								// Set Smooth Shading 
		obj_data->mode = GL_POLYGON;
	}
	else if (key == 'f')
	{
		glShadeModel(GL_FLAT);								// Set Smooth Shading 
		obj_data->mode = GL_POLYGON;
	}
	else if (key == 'q')
	{
		delete obj_data;
		exit(0);
	}

	glutPostRedisplay();
}

void MouseWheel(int wheel, int direction, int x, int y)
{
	if (direction > 0) {
		zoom += 0.5;
	}
	else {
		zoom -= 0.5;
	}

	glutPostRedisplay();
}

void mouseMotion(int x, int y)
{
	float curPos[3], deltaX, deltaY, deltaZ;
	trackball_proj(x, y, winWidth, winHeight, curPos);
	if (trackingMouse) {
		deltaX = curPos[0] - lastPos[0];
		deltaY = curPos[1] - lastPos[1];
		deltaZ = curPos[2] - lastPos[2];
	}
	if (deltaX || deltaY || deltaZ) {
		angle = 90.0 * sqrt(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);
		axis[0] = lastPos[1] * curPos[2] - lastPos[2] * curPos[1];
		axis[1] = lastPos[2] * curPos[0] - lastPos[0] * curPos[2];
		axis[2] = lastPos[0] * curPos[1] - lastPos[1] * curPos[0];
	}
	glutPostRedisplay();
}

void mouseButton(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON) switch (state) {
	case GLUT_DOWN:
		y = winHeight - y;
		start(x, y);
		break;
	case GLUT_UP:
		stop(x, y);
		break;
	}
}

int main(int argc, char *argv[])
{
	// glut initialization functions:
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(winWidth, winHeight);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("ImageViewer");

	Init(winWidth, winHeight);

	// display, onMouseButton, mouse_motion, onKeyPress, and resize are functions defined above
	glutDisplayFunc(Draw);
	glutKeyboardFunc(onKeyPress);
	glutSpecialFunc(Specialkey);
	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMotion);
	glutMouseWheelFunc(MouseWheel);
	glutReshapeFunc(Init);

	if (argc >= 2)
		obj_data = new WavefrontObj(argv[1]);
	else
		obj_data = NULL;

	// start glutMainLoop -- infinite loop
	glutMainLoop();

	return 0;
}
