
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
#include "texture.h"
#include "skeleton.h"
#include "motion.h"
#include "timer.h"
#include "attachment.h"


using namespace Eigen;


int main(int, char**);
void setup(char*, char*, char*);
void drawScene(void);
void resize(int, int);
void keyInput(unsigned char, int, int);
void specialKeyInput(int, int, int);

void loadTexture();
void addTexture(string bmpFileName);
void removeTexture();
void idle(void);
BitMapFile *getbmp(string filename);


/* global variables */
mesh obj;
camera cam;
skeleton skel;
motion mot;
timer tim;
attachment att(&obj, &skel);
Vector3f initialPosition(0.0, 0.0, -1.0);
int view_mode = 1; //1=wire, 2=flat shade, 3=smooth shade, 4=bitmap
string bmpFileName = "../a3files/mesh/skin.bmp";
static unsigned int texture[1]; // Array of texture ids.
float boneRadii[] = {
	2.0, 2.0, 1.0, 0.5, 0.25, 0.0,
	2.0, 2.0, 1.0, 0.5, 0.25, 0.0,
	2.0, 2.0, 2.5, 1.5, 0.5, 0.5, 0.5,
	2.0, 1.0, 1.0, 0.5, 0.5, 0.5, 0.25, 0.0, 0.0,
	2.0, 1.0, 1.0, 0.5, 0.5, 0.5, 0.25, 0.0, 0.0
};
bool interpolate(true);





int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitContextVersion(4, 2);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("personViewer");
	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyInput);
	glutSpecialFunc(specialKeyInput);
  glutIdleFunc(idle);
#ifndef __APPLE__
	glewExperimental = GL_TRUE;
	glewInit();
#endif
	if (argc != 4) {
		cerr << "Usage: modelViewer <meshfile.obj> <motion.bvh> <attachment.att>" << endl;
		exit(1);
	}
	setup(argv[1], argv[2], argv[3]);
	glutMainLoop();
	return 0;
}


void setup(char* objFile, char* motionFile, char* attachFile)
{
  //OBJ
	obj.readObjFile(objFile);
  obj.normalize();
	//obj.createVertexArray();  //called display list instead
	//obj.glEnableVAO();
	//cam.initialize(persp, -0.1, 0.1, -0.1, 0.1, 0.1, 100.0);

  //BVH
  // skeleton
	skel.readBvhSkeleton(motionFile);
	skel.determineRadius();
  skel.recoverBones();
	// motion
	mot.readBvhMotion(motionFile, skel);
	mot.determineRange(skel.translationIndices);
	// timer
	tim.initialize(true, mot.sequence.size(), mot.defaultGapTime);
	// camera
	cam.initialize(persp, -0.1, 0.1, -0.1, 0.1, 0.1, 200.0);
  cam.transl[2] = -1.0;
	cam.positionMotion(mot.range, skel.radius);
	// camera view volume
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	cam.glVolume();

  //ATT
  //do something with attach file
  att.distancesVisibility(boneRadii);
	att.readW(attachFile);
  cout << "done reading attachments" << endl;



  glClearColor(0.0, 0.0, 0.0, 0.0);
}


void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
  cam.glPosition(); // camera transformation

	if (view_mode == 4){
    addTexture(bmpFileName);//add skin to model
	} else {
		removeTexture();
	}
	shadeScene(view_mode); //must place light before model transformation

  //draw skeleton & mesh
  // skel.glColor();
	skel.interpolatePose(&mot, tim.loopFrac, interpolate);
	skel.glDraw(att.W, obj.vertices, obj.normals, obj.textures);

  //draw mesh
  // obj.glColor();
	// obj.glCreateDisplayList(); //instead of obj.glDrawVAO();
	// obj.glCallDisplayList();

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
      skel.writeBvhSkeleton("skeletonout.bvh");
      mot.writeBvhMotion("skeletonout.bvh");
      att.writeMatrixXfSparse(att.W, "attachout.att");
			break;
		case 's':
			if (view_mode < 4) view_mode++; else view_mode = 1; break;
		case 'x': // reset
			//obj.reset();
      tim.initialize(true, mot.sequence.size(), mot.defaultGapTime);
			cam.initialize(persp, -0.1, 0.1, -0.1, 0.1, 0.1, 100.0);
      cam.positionMotion(mot.range, skel.radius);
			break;

		default: break;
	}
	cam.keyInput(key); // camera controls
  tim.keyInput(key); //timer controls  //p, P, +, -

	glutPostRedisplay();
}


void specialKeyInput(int key, int x, int y)
{
	// switch(key)
	// {
	// 	case GLUT_KEY_LEFT:  obj.xTransl(-0.1); break;
	// 	case GLUT_KEY_RIGHT: obj.xTransl(0.1);  break;
	// 	case GLUT_KEY_DOWN:  obj.yTransl(-0.1); break;
	// 	case GLUT_KEY_UP:    obj.yTransl(0.1);  break;
	// 	default: break;
	// }
	// glutPostRedisplay();
}















//FROM Textbook Code
//read BMP
BitMapFile *getbmp(string filename)
{
    int offset, headerSize;

	// Initialize bitmap files for RGB (input) and RGBA (output).
	BitMapFile *bmpRGB = new BitMapFile;
	BitMapFile *bmpRGBA = new BitMapFile;

	// Read input bmp file name.
	ifstream infile(filename.c_str(), ios::binary);

	// Get starting point of image data in bmp file.
	infile.seekg(10);
	infile.read((char *)&offset, 4);

	// Get header size of bmp file.
	infile.read((char *)&headerSize,4);

	// Get image width and height values from bmp file header.
	infile.seekg(18);
	infile.read((char *)&bmpRGB->sizeX, 4);
	infile.read((char *)&bmpRGB->sizeY, 4);

	// Determine the length of zero-byte padding of the scanlines
	// (each scanline of a bmp file is 4-byte aligned by padding with zeros).
	int padding = (3 * bmpRGB->sizeX) % 4 ? 4 - (3 * bmpRGB->sizeX) % 4 : 0;

	// Add the padding to determine size of each scanline.
	int sizeScanline = 3 * bmpRGB->sizeX + padding;

	// Allocate storage for image in input bitmap file.
	int sizeStorage = sizeScanline * bmpRGB->sizeY;
	bmpRGB->data = new unsigned char[sizeStorage];

	// Read bmp file image data into input bitmap file.
	infile.seekg(offset);
	infile.read((char *) bmpRGB->data , sizeStorage);

	// Reverse color values from BGR (bmp storage format) to RGB.
	int startScanline, endScanlineImageData, temp;
	for (int y = 0; y < bmpRGB->sizeY; y++)
	{
       startScanline = y * sizeScanline; // Start position of y'th scanline.
	   endScanlineImageData = startScanline + 3 * bmpRGB->sizeX; // Image data excludes padding.
       for (int x = startScanline; x < endScanlineImageData; x += 3)
	   {
	      temp = bmpRGB->data[x];
		  bmpRGB->data[x] = bmpRGB->data[x+2];
		  bmpRGB->data[x+2] = temp;
	   }
	}

	// Set image width and height values and allocate storage for image in output bitmap file.
	bmpRGBA->sizeX = bmpRGB->sizeX;
	bmpRGBA->sizeY = bmpRGB->sizeY;
	bmpRGBA->data = new unsigned char[4*bmpRGB->sizeX*bmpRGB->sizeY];

	// Copy RGB data from input to output bitmap files, set output A to 1.
	for(int j = 0; j < 4*bmpRGB->sizeY * bmpRGB->sizeX; j+=4)
	{
		bmpRGBA->data[j] = bmpRGB->data[(j/4)*3];
		bmpRGBA->data[j+1] = bmpRGB->data[(j/4)*3+1];
		bmpRGBA->data[j+2] = bmpRGB->data[(j/4)*3+2];
		bmpRGBA->data[j+3] = 0xFF;
	}

	return bmpRGBA;
}


void loadTexture(string bmpFileName)
{
   // Local storage for bmp image data.
   BitMapFile *image[1];

   // Load the image.
   image[0] = getbmp(bmpFileName);

   // Create texture object texture[0].
   glBindTexture(GL_TEXTURE_2D, texture[0]);

   // Specify image data for currently active texture object.
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[0]->sizeX, image[0]->sizeY, 0,
	            GL_RGBA, GL_UNSIGNED_BYTE, image[0]->data);

   // Set texture parameters for wrapping.
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

   // Set texture parameters for filtering.
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void addTexture(string bmpFileName){
  // Create texture ids.
  glGenTextures(1, texture);

  // Load external texture.
  loadTexture(bmpFileName);

  // Specify how texture values combine with current surface color values.
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  // Turn on OpenGL texturing.
  glEnable(GL_TEXTURE_2D);

  // Activate texture object.
  glBindTexture(GL_TEXTURE_2D, texture[0]);
}

void removeTexture(){
	glDisable(GL_TEXTURE_2D);
}

void idle(void){
	tim.glIdle();
}
