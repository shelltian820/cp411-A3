
// modelViewer <mesh.obj>

// modified CMPUT 411/511 Assignment 1 solution

#include <cstdlib>
#include <iostream>
#include <fstream>
//
// #ifndef __APPLE__
// #  include <GL/glew.h>
// #endif
// #include <GL/freeglut.h>


#ifdef __APPLE__
#  include <GL/glew.h>
#  include <GL/freeglut.h>
#  include <OpenGL/glext.h>
#else
#  include <GL/glew.h>
#  include <GL/freeglut.h>
#  include <GL/glext.h>
#pragma comment(lib, "glew32.lib")
#endif

#include "camera.h"
#include "mesh.h"
#include "light.h"
//#include "texture.cpp"

using namespace Eigen;


int main(int, char**);
void setup(char*);
void drawScene(void);
void resize(int, int);
void keyInput(unsigned char, int, int);
void specialKeyInput(int, int, int);


/* global variables */
mesh obj;
camera cam;
Vector3f initialPosition(0.0, 0.0, -1.0);
bool fog = true;
const float fogColor[4] = {0.0, 0.0, 0.0, 0.0};
int view_mode = 1; //1=wire, 2=flat shade, 3=smooth shade, 4=bitmap
string bmpFileName = "../a3files/mesh/skin.bmp";

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitContextVersion(4, 2);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("modelViewer");
	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyInput);
	glutSpecialFunc(specialKeyInput);
#ifndef __APPLE__
	glewExperimental = GL_TRUE;
	glewInit();
#endif
	if (argc != 2) {
		cerr << "Usage: modelViewer <meshfile.obj>" << endl;
		exit(1);
	}
	setup(argv[1]);
	glutMainLoop();
	return 0;
}


void setup(char* fileName)
{
	obj.readObjFile(fileName);
	obj.normalize();
	//obj.createVertexArray();  //called display list instead
	//obj.glEnableVAO();
	cam.initialize(persp, -0.1, 0.1, -0.1, 0.1, 0.1, 100.0);

}


void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	cam.glVolume(); // camera view volume

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//if (view_mode == 4) addTexture(bmpFileName); //add skin to model

	shadeScene(view_mode); //must place light before model transformation


	cam.glPosition(); // camera transformation
	obj.glPosition(initialPosition); // object transformation

	// draw model
	obj.glColor();
	//obj.glDrawVAO(); //call displaylist instead
	obj.glCreateDisplayList();
	obj.glCallDisplayList();
	glutSwapBuffers();
}


void resize(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}


void keyInput(unsigned char key, int x, int y)
{
	switch(key)
	{
		case 'q': exit(0);            break; // quit
		case 'w':
			obj.writeObjFile("meshout.obj");
			//write skeletonout.bvh
			//write attachout.att
			break;
		case 'n': obj.zTransl(-0.1);  break;
		case 'N': obj.zTransl(0.1);   break;
		case 'p': obj.xRotate(-10.0); break;
		case 'P': obj.xRotate(10.0);  break;
		case 'y': obj.yRotate(-10.0); break;
		case 'Y': obj.yRotate(10.0);  break;
		case 'r': obj.zRotate(-10.0); break;
		case 'R': obj.zRotate(10.0);  break;
		case 's':
			cout << "s" <<endl;
			if (view_mode < 4) {
				view_mode++;
			} else {
				view_mode = 1;
			}
			break;
		case 'x': // reset
			obj.reset();
			cam.initialize(persp, -0.1, 0.1, -0.1, 0.1, 0.1, 100.0);
			fog = false;
			break;
		default: break;
	}
	cam.keyInput(key); // camera controls

	glutPostRedisplay();
}


void specialKeyInput(int key, int x, int y)
{
	switch(key)
	{
		case GLUT_KEY_LEFT:  obj.xTransl(-0.1); break;
		case GLUT_KEY_RIGHT: obj.xTransl(0.1);  break;
		case GLUT_KEY_DOWN:  obj.yTransl(-0.1); break;
		case GLUT_KEY_UP:    obj.yTransl(0.1);  break;
		default: break;
	}
	glutPostRedisplay();
}