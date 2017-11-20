
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <Eigen/Dense>

#ifndef __APPLE__
#  include <GL/glew.h>
#endif
#include <GL/freeglut.h>

#include "bvh_util.h"
#include "motion.h"
#include "timer.h"


#ifndef SKELETON_H
#define SKELETON_H

#define DEG2RAD_SKE 0.017453292519943 /* rad = deg*PI/180 = deg*DEG2RAD */
#define RAD2DEG_SKE 57.29577951308232 /* deg = rad*180/PI = rad*RAD2DEG */

using namespace std;
using namespace Eigen;


// joint class

enum jointType {rootJoint, regularJoint, endSite};

struct joint
{
	string name;
	jointType type;
	unsigned int id;

	Vector3f offset;
	Vector3f rotate, rotateB;
	Vector3f transl, translB;
	Quaternionf quaternion, quaternionB;

	vector<joint*> children;
	vector<string> channelNames;

	// constructor
	joint(string nm, jointType tp)
	{
		name = nm;
		type = tp;
		offset = Vector3f::Zero();
		rotate = Vector3f::Zero();
		transl = Vector3f::Zero();
		quaternion = Quaternionf::Identity();
		rotateB = Vector3f::Zero();
		translB = Vector3f::Zero();
		quaternionB = Quaternionf::Identity();
	}
};


// skeleton class

struct skeleton
{
	joint* root;
	float radius;
	vector<float*> channels, channelsB;
	Vector3i translationIndices;
	float skel_color[4], mesh_color[4];

	// vertices and bones (used for vertex arrays)
	vector<Vector3f> vertices;
	vector<Vector2i> bones;

	//frame info
	vector<Vector3f> timeVertices; //3d position of vertex j at frame i
	vector<Vector3f> timeNormals; //normal of vertex j at frame i

	// vertex array
	float* vertexArray;
	unsigned int numVertices;
	unsigned int vaoId[1];

	// constructor
	skeleton()
	{
		// color[1] = color[2] = 0.0;
		// color[0] = color[3] = 1.0;
		skel_color[0] = 1.0, skel_color[1] = 0.0;
		skel_color[2] = 0.0, skel_color[3] = 1.0;

		mesh_color[0] = 1.0, mesh_color[1] = 1.0;
		mesh_color[2] = 1.0, mesh_color[3] = 1.0;

		vertexArray = NULL;
		numVertices = 0;
	}


	// member functions


	void readBvhSkeleton(char*);
	joint* acquire_SubTree(ifstream&, string, jointType);
	void writeBvhSkeleton(char*);
	void output_SubTree(ofstream&, joint*, unsigned int);

	void determineRadius() { radius = maxBranchLength(root); }
	float maxBranchLength(joint*);
	void nullPose();
	void interpolatePose(motion*, double, bool);
	void interpolatePose_SubTree(joint*, float);
	Quaternionf quaternionFromEulers(Vector3f, vector<string>);

	void glColorSkel() { glColor4fv(skel_color); }
	void glColorMesh() { glColor4fv(mesh_color); }





	void glDraw(MatrixXf* W, vector<Vector3f> meshVertices, vector<Vector3f> meshNormals, vector<Vector2f> meshTextures) {
		glColorSkel();
		glDrawSubTreeSkel(root);
		glColorMesh();
		glDrawSubTreeMesh(root, W, meshVertices, meshNormals, meshTextures);
	}

	void glDrawSubTreeSkel(joint*);
	void glDrawSubTreeMesh(joint*, MatrixXf*, vector<Vector3f>,vector<Vector3f>, vector<Vector2f>);
	void compileSubtree();
	void glDrawFrame();






	// vertices and bones (used for vertex arrays)
	void recoverBones();
	void recoverBones_SubTree(joint*, Vector3f, int);

	void glDrawBones();
	void glDrawBonesHighlight(int);

	// vertex array
	void createVertexArray();
	void glEnableVertexArray();
	void glDrawVertexArray();
	void updateVertexArray();
	void updateVA_SubTree(joint*, Affine3f, Vector3f);

	// VBO
	void glEnableVBO();
	void glUpdateVBO();

	// VAO
	void glEnableVAO();
	void glDrawVAO();
	void glUpdateVAO();


};

#endif
