
// temporary mesh.cpp - requires additions to complete Assignment 3

#include "mesh.h"

using namespace std;
using namespace Eigen;


void mesh::readObjFile(char* fileName)
{
	ifstream infile(fileName);
	if (!infile) {
		cerr << "Error: unable to open obj file: " << fileName << endl;
		exit(1);
	}
	string line;

	while (getline(infile, line)) {
		istringstream stin(line);
		string token;

		if (!(stin >> token)) continue;

		if (token == "v") {
			Vector3f v;
			stin >> v[0] >> v[1] >> v[2];
			vertices.push_back(v);

		 } else if (token == "f") {
			vector<unsigned int> faceV;
			vector<unsigned int> faceT;
			vector<unsigned int> faceN;
			unsigned int i;
			while (stin >> i) {
				//store v
				faceV.push_back(i-1);
				if (stin.get() == '/') {
					//check if t exists
					if (stin.peek() != '/') {
						stin >> i;
						faceT.push_back(i-1);
					}

					if (stin.get() == '/'){
						//store n
						stin >> i;
						faceN.push_back(i-1);
					}
				}
			}
			faceVertices.push_back(faceV);
			faceTextures.push_back(faceT);
			faceNormals.push_back(faceN);
		} else if (token == "vn") {
			Vector3f n;
			stin >> n[0] >> n[1] >> n[2];
			normals.push_back(n);
		} else if (token == "vt") {
			Vector2f t;
			stin >> t[0] >> t[1];
			textures.push_back(t);
		}
	}
	infile.close();
	cout << "done reading\n";
}


void mesh::writeObjFile(char* fileName)
{
	ofstream outfile(fileName);
	if (!outfile) {
		cerr << "Error: unable to open output file: " << fileName << endl;
		exit(1);
	}
	//write v
	for (unsigned int i = 0; i < vertices.size(); ++i)
		outfile << "v " << vertices[i][0] << " " << vertices[i][1] << " " << vertices[i][2] << endl;

	//write vt //tx ty
	for (unsigned int i = 0; i < textures.size(); ++i)
		outfile << "vt " << textures[i][0] << " " << textures[i][1] << endl;

	//write vn //vn nx ny nz
	for (unsigned int i = 0; i < normals.size(); ++i)
		outfile << "vn " << normals[i][0] << " " << normals[i][1] << " " << normals[i][2] << endl;

	//write f //f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
	for (unsigned int i = 0; i < faceVertices.size(); ++i) {
		outfile << "f ";
		for (unsigned int j = 0; j < faceVertices[i].size(); ++j) {
			outfile << faceVertices[i][j] + 1 << "/" << faceTextures[i][j] + 1 << "/" << faceNormals[i][j] +1;
			outfile << " ";
		}
		outfile << endl;
	}
	outfile.close();
	cout << "done writing\n";
}


void mesh::normalize()
{
	Vector3f totals(0, 0, 0);
	Vector3f maxs(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	Vector3f mins(FLT_MAX, FLT_MAX, FLT_MAX);

	for (unsigned int v = 0; v < vertices.size(); ++v)
		for (unsigned int j = 0; j < 3; ++j) {
			maxs[j] = max(maxs[j], vertices[v][j]);
			mins[j] = min(mins[j], vertices[v][j]);
			totals[j] += vertices[v][j];
		}
	center = totals / (float)vertices.size();
	Vector3f scales = maxs - mins;
	float scale = (float)scales.maxCoeff();
  meshScale = 1.0/scale;
}


void mesh::glCreateDisplayList()
{
	displayList = glGenLists(1);
	glNewList(displayList, GL_COMPILE);
		for (unsigned int f = 0; f < faceVertices.size(); ++f) { //for each face
			glBegin(GL_TRIANGLE_FAN);
				for (unsigned int j = 0; j < faceVertices[f].size(); ++j) { //for each face vertex index
					//DEFINE NORMAL, THEN TEXTURE, THEN VERTEX
					unsigned int n(faceNormals[f][j]); ///get normal index
					glNormal3fv(normals[n].data()); //get normal

					unsigned int t(faceTextures[f][j]); //get texture index
					glTexCoord2f(textures[t][0],textures[t][1]); //get texture coordinate

					unsigned int v(faceVertices[f][j]); //get vertex index // same as: unsigned int v = faceVertices[f][j]
					glVertex3fv(vertices[v].data()); //get vertex coordinate
				}
			glEnd();
		}
	glEndList();
}


void mesh::glCallDisplayList()
{
	if (displayList)
		glCallList(displayList);
	else {
		cerr << "Error: display list incorrectly initialized" << endl;
		exit(1);
	}
}


void mesh::createVertexArray()
{
	numVertices = vertices.size();
	vertexArray = new float[3*numVertices];
	// normalArray = new float[3*numVertices];

	for (unsigned int v = 0; v < numVertices; ++v) {
		vertexArray[3*v + 0] = vertices[v][0];
		vertexArray[3*v + 1] = vertices[v][1];
		vertexArray[3*v + 2] = vertices[v][2];
		// normalArray[3*v + 0] = normals[v][0];
		// normalArray[3*v + 1] = normals[v][1];
		// normalArray[3*v + 2] = normals[v][2];
	}

	numTriangles = 0;
	for (unsigned int f = 0; f < faceVertices.size(); ++f)
		numTriangles += faceVertices[f].size() - 2;

	triangleIndices = new unsigned int[3*numTriangles];
	diameters = new float[numTriangles];
	int t(0);

	for (unsigned int f = 0; f < faceVertices.size(); ++f) {
		// trianglulate each face
		vector<unsigned int> face(faceVertices[f]);
		unsigned int i0(face[0]);
		unsigned int i1;
		unsigned int i2;

		for (unsigned int i = 1; i < face.size() - 1; ++i) {
			// triangle fan
			i1 = face[i];
			i2 = face[i + 1];
			triangleIndices[3*t + 0] = i0;
			triangleIndices[3*t + 1] = i1;
			triangleIndices[3*t + 2] = i2;

			float diameter(0);
			Vector3f p0(vertices[i0]);
			Vector3f p1(vertices[i1]);
			Vector3f p2(vertices[i2]);
			diameter = max(diameter, sqrt((p1 - p0).dot(p1 - p0)));
			diameter = max(diameter, sqrt((p2 - p1).dot(p2 - p1)));
			diameter = max(diameter, sqrt((p0 - p2).dot(p0 - p2)));
			diameters[t] = diameter;

			++t;
		}
	}
}


void mesh::glEnableVertexArray()
{
	if (vertexArray == NULL) {
		cerr << "Error: vertex array incorrectly initialized" << endl;
		exit(1);
	}
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertexArray);
}


void mesh::glDrawVertexArray()
{
	if (triangleIndices == NULL) {
		cerr << "Error: triangle indices incorrectly initialized" << endl;
		exit(1);
	} else
		glDrawElements(GL_TRIANGLES, 3*numTriangles, GL_UNSIGNED_INT,
		               triangleIndices);
}


void mesh::glEnableVBO()
{
	unsigned int bufferIds[2];
	glGenBuffers(2, bufferIds);

	// vertices
	glBindBuffer(GL_ARRAY_BUFFER, bufferIds[0]);
	glBufferData(GL_ARRAY_BUFFER, 3*numVertices*sizeof(float),
	             vertexArray, GL_STATIC_DRAW);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	// triangleIndices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*numTriangles*sizeof(unsigned int),
	             triangleIndices, GL_STATIC_DRAW);
}


void mesh::glDrawVBO()
{
	glDrawElements(GL_TRIANGLES, 3*numTriangles, GL_UNSIGNED_INT, 0);
}


void mesh::glEnableVAO()
{

#ifdef __APPLE__
	glGenVertexArraysAPPLE(1, vaoId);
	glBindVertexArrayAPPLE(vaoId[0]);
#else
	glGenVertexArrays(1, vaoId);
	glBindVertexArray(vaoId[0]);
#endif
	glEnableVBO();
}


void mesh::glDrawVAO()
{
#ifdef __APPLE__
	glBindVertexArrayAPPLE(vaoId[0]);
#else
	glBindVertexArray(vaoId[0]);
#endif
	glDrawVBO();
}
