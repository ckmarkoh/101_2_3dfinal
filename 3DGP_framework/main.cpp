// 3DGP_framework.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>
#include "image.h"
#include <assert.h>
#include <algorithm> 
#include <iostream>
#include <string>
#define PI 3.14159265
vector<GLMmodel*> MODELS;
vector<image*> IMARRAY;
GLMmodel* OBJ;
image* IMG;
int screenWidth = 600;
int screenHeight = 400;
vector<vec3> viewVertices;
vector<vec3> viewFaceNormals;
vector<vec3> viewNormals;
int imgid=0;

//////////////////////////////////////////////////////////////////////////
// render
//	This is the rendering function
//////////////////////////////////////////////////////////////////////////

GLvoid
dimension(vector<vec3>& vertices, GLfloat* dimensions, GLfloat* center)
{
    GLuint i;
    GLfloat maxx, minx, maxy, miny, maxz, minz;
    
    /* get the max/mins */
    maxx = minx = vertices[0][0];
    maxy = miny = vertices[0][1];
    maxz = minz = vertices[0][2];
	for (i = 0; i < vertices.size(); i++) {
        if (maxx < vertices[i][0])
            maxx = vertices[i][0];
        if (minx > vertices[i][0])
            minx = vertices[i][0];
        
        if (maxy < vertices[i][1])
            maxy = vertices[i][1];
        if (miny > vertices[i][1])
            miny = vertices[i][1];
        
        if (maxz < vertices[i][2])
            maxz = vertices[i][2];
        if (minz > vertices[i][2])
            minz = vertices[i][2];
    }

	dimensions[0] = maxx - minx;
	dimensions[1] = maxy - miny;
	dimensions[2] = maxz - minz;
	center[0] = (maxx + minx)/2;
	center[1] = (maxy + miny)/2;
	center[2] = (maxz + minz)/2;
}


class Vertex
{
public:
	Vertex() {};
	Vertex(Vertex* v) {
		pos = v->pos;
		col = v->col;
	}
/*	Vertex (GLMmodel* obj, int index) {
		pos = viewVertices[index-1];
		col = vec3(obj->colors[index*3+0], obj->colors[index*3+1], obj->colors[index*3+2]);
	}*/
	Vertex (GLMmodel* obj, int index,int tid) {
		pos = viewVertices[index-1];
		//pos = vec3(obj->vertices[index*3+0], obj->vertices[index*3+1], obj->vertices[index*3+2]);
		tpos= vec2(obj->texcoords[tid*2],obj->texcoords[tid*2+1]);

		setColor();
	}
	void setColor(){

		double img_width=IMARRAY[imgid]->width;
		double img_height=IMARRAY[imgid]->height;
		unsigned data_id=(unsigned)(tpos[1]*img_width)*img_height+(tpos[0]*img_height);
//		assert(data_id<IMARRAY[imgid]->data.size());
		if(data_id>=IMARRAY[imgid]->data.size()){
			data_id=IMARRAY[imgid]->data.size()-1;
		}
		col = vec3(
				IMARRAY[imgid]->data[data_id].color[0],
				IMARRAY[imgid]->data[data_id].color[1],
				IMARRAY[imgid]->data[data_id].color[2]);
	//	cout<<"end set_color"<<endl;
	}
	vec3 pos;
	vec2 tpos;
	vec3 col;
};

bool clip (FrameBuffer* colorBuff, vec3& point, bool perspective)
{
	int W = colorBuff->getWidth();
	int H = colorBuff->getHeight();
	// if (perspective) apply projection to x,y before comparing to view port
	double p = (perspective)? LViewer::nearPlane/point[2] : 1.0;
	if (point[0]*p < 0 || point[0]*p >= colorBuff->getWidth()) return true;
	if (point[1]*p < 0 || point[1]*p >= colorBuff->getHeight()) return true;
	if (point[2] < LViewer::nearPlane || point[2] >= LViewer::farPlane) return true;
	return false;
}

bool clipLine (FrameBuffer* colorBuff, vec3& point1, vec3& point2, bool perspective)
{
	int W = colorBuff->getWidth();
	int H = colorBuff->getHeight();
	
	double p1 = (perspective)? LViewer::nearPlane/point1[2] : 1.0;
	double p2 = (perspective)? LViewer::nearPlane/point2[2] : 1.0;
	
	if (point1[0]*p1 < 0 && point2[0]*p2 < 0) return true;
	if (point1[0]*p1 >= colorBuff->getWidth() && point2[0]*p2 >= colorBuff->getWidth()) return true;
	if (point1[1]*p1 < 0 && point2[1]*p2 < 0) return true;
	if (point1[1]*p1 >= colorBuff->getHeight() && point2[1]*p2 >= colorBuff->getHeight()) return true;
	if (point1[2] < LViewer::nearPlane && point2[2] < LViewer::nearPlane ) return true;
	if (point1[2] >= LViewer::farPlane && point2[2] >= LViewer::farPlane ) return true;
	return false;
}

void perspectiveInpt(Vertex& ret, Vertex a, Vertex b, GLfloat s, bool perspective)
{
	if (perspective) {
		ret.pos[2] = 1/(1/a.pos[2] + s*(1/b.pos[2]-1/a.pos[2]));
		//GLfloat i1 = a.col * (1/a.pos[2]);
		ret.tpos = ret.pos[2] * ( a.tpos/a.pos[2] + s*(b.tpos/b.pos[2]-a.tpos/a.pos[2]) );
		ret.setColor();
		//ret.col = ret.pos[2] * ( a.col/a.pos[2] + s*(b.col/b.pos[2]-a.col/a.pos[2]) );
	}
	else {
		ret.pos[2] = a.pos[2] + s*(b.pos[2]-a.pos[2]);
		ret.tpos = a.tpos + s*(b.tpos-a.tpos);
		ret.setColor();
		//ret.col = a.col + s*(b.col-a.col);
	}
}

void perspectiveLine(FrameBuffer * colorBuff, vector<GLfloat> &zBuff, Vertex a, Vertex b, bool perspective)
{
	if (clipLine(colorBuff, a.pos, b.pos, false)) return;

	vec3 p = b.pos - a.pos;
	GLfloat x, y, m, minv;
	int pnt;
	int W = colorBuff->getWidth();
	int H = colorBuff->getHeight();
	Vertex tmp;

	if (p[0] == 0 && p[1] == 0) {
		if (clip(colorBuff, a.pos, false)) return;
		pnt = int(a.pos[1])*W+int(a.pos[0]);
		if (zBuff[pnt] > a.pos[2]) {
			zBuff[pnt] = a.pos[2];
			vec3 color = a.col;
			color *= (LViewer::farPlane-zBuff[pnt])/(LViewer::farPlane-LViewer::nearPlane);
			colorBuff->setColor((int)a.pos[0], (int)a.pos[1], color);
		}
	}
	else if (abs(p[0]) <  abs(p[1])) {// x < y including vertical lines (y++)
		//x = minv*y + B
		minv = p[0]/p[1];
		GLfloat B = a.pos[0] - minv * a.pos[1];
		if (p[1] < 0) swap(a, b);
		y = floor(a.pos[1] + 0.5);
		x = minv * y + B;
		
		tmp.pos[0] = floor(x + 0.5); tmp.pos[1] = y; 
		perspectiveInpt(tmp, a, b, (y-a.pos[1])/(b.pos[1]-a.pos[1]), perspective);
		while (y <= floor(b.pos[1]+0.5) && clip(colorBuff, tmp.pos, false)) {
			x = x + minv; y = y + 1;
			tmp.pos[0] = floor(x+0.5); tmp.pos[1] = y; 
			perspectiveInpt(tmp, a, b, (y-a.pos[1])/(b.pos[1]-a.pos[1]), perspective);
		}
		while (y <= floor(b.pos[1]+0.5) && !clip(colorBuff, tmp.pos, false))
		{
			pnt = int(tmp.pos[1])*W+int(tmp.pos[0]);
			if (zBuff[pnt] > tmp.pos[2]) {
				zBuff[pnt] = tmp.pos[2];
				vec3 color = tmp.col;
				color *= (LViewer::farPlane-zBuff[pnt])/(LViewer::farPlane-LViewer::nearPlane);
				colorBuff->setColor((int)x, (int)y, color);
			}
			x = x + minv; y = y + 1;
			tmp.pos[0] = floor(x+0.5); tmp.pos[1] = y; 
			perspectiveInpt(tmp, a, b, (y-a.pos[1])/(b.pos[1]-a.pos[1]), perspective);
		}
	}
	else {// x > y including horizontal lines (x++)
		//y = mx + B
		m = p[1]/p[0];
		GLfloat B = a.pos[1] - m * a.pos[0];
		if (p[0] < 0) swap(a, b);
		//x = (a.pos[0]-floor(a.pos[0]) < 0.5)? floor(a.pos[0]):ceil(a.pos[0]);
		x = floor(a.pos[0] + 0.5);
		y = m * x + B;
		
		tmp.pos[0] = x; tmp.pos[1] = floor(y + 0.5); 
		perspectiveInpt(tmp, a, b, (x-a.pos[0])/(b.pos[0]-a.pos[0]), perspective);
		while (x <= floor(b.pos[0]+0.5) && clip(colorBuff, tmp.pos, false)) {
			x = x + 1; y = y + m;
			tmp.pos[0] = x; tmp.pos[1] = floor(y+0.5); 
			perspectiveInpt(tmp, a, b, (x-a.pos[0])/(b.pos[0]-a.pos[0]), perspective);
		}
		while (x <= floor(b.pos[0]+0.5) && !clip(colorBuff, tmp.pos, false))
		{
			pnt = int(tmp.pos[1])*W+int(tmp.pos[0]);
			if (zBuff[pnt] > tmp.pos[2]) {
				zBuff[pnt] = tmp.pos[2];
				vec3 color = tmp.col;
				color *= (LViewer::farPlane-zBuff[pnt])/(LViewer::farPlane-LViewer::nearPlane);
				colorBuff->setColor((int)x, (int)y, color);
			}
			x = x + 1; y = y + m;
			tmp.pos[0] = x; tmp.pos[1] = floor(y+0.5); 
			perspectiveInpt(tmp, a, b, (x-a.pos[0])/(b.pos[0]-a.pos[0]), perspective);
		}
	}
}

void perspectiveTri(FrameBuffer* colorBuff, vector<GLfloat> &zBuff, GLMtriangle t, bool perspective)
{
	Vertex a(OBJ, t.vindices[0], t.tindices[0]);
	Vertex b(OBJ, t.vindices[1], t.tindices[1]);
	Vertex c(OBJ, t.vindices[2], t.tindices[2]);

	// clip triangle

	// sort in descending order using y
	if (b.pos[1] > a.pos[1]) swap(a, b);
	if (c.pos[1] > a.pos[1]) swap(a, c);
	if (c.pos[1] > b.pos[1]) swap(b, c);

	if (a.pos[1] == c.pos[1]) {
		if (b.pos[0] < a.pos[0]) swap(a, b);
		if (c.pos[0] < a.pos[0]) swap(a, c);
		if (c.pos[0] < b.pos[0]) swap(b, c);
		perspectiveLine(colorBuff, zBuff, a, c, perspective);
		return;
	}

	GLfloat Y = floor(a.pos[1] + 0.5);
	Vertex left, right;
	GLfloat minv1 = (a.pos[0]-c.pos[0]) / (a.pos[1]-c.pos[1]);
	GLfloat minv2 = (a.pos[0]-b.pos[0]) / (a.pos[1]-b.pos[1]);
	GLfloat B1 = a.pos[0] - minv1 * a.pos[1];
	GLfloat B2 = a.pos[0] - minv2 * a.pos[1];
	
	if (a.pos[1] == b.pos[1]) {
		perspectiveLine(colorBuff, zBuff, a, b, perspective);
		Y--;
	}
	if (Y > a.pos[1]) {
		perspectiveLine(colorBuff, zBuff, a, a, perspective);
		Y--;
	}
	while (Y >= ceil(b.pos[1])) {
		left.pos[1] = Y;
		left.pos[0] = floor(minv1*Y + B1 + 0.5);
		right.pos[1] = Y;
		right.pos[0] = floor(minv2*Y + B2 + 0.5);
		perspectiveInpt(left, a, c, (Y-a.pos[1])/(c.pos[1]-a.pos[1]), perspective);
		perspectiveInpt(right, a, b, (Y-a.pos[1])/(b.pos[1]-a.pos[1]), perspective);
		perspectiveLine(colorBuff, zBuff, left, right, perspective);
		Y--;
	}
	minv2 = (c.pos[0]-b.pos[0]) / (c.pos[1]-b.pos[1]);
	B2 = c.pos[0] - minv2 * c.pos[1];
	
	if (c.pos[1] == b.pos[1]) {
		perspectiveLine(colorBuff, zBuff, c, b, perspective);
		Y--;
	}
	while (Y >= ceil(c.pos[1])) {
		left.pos[1] = Y;
		left.pos[0] = floor(minv1*Y + B1 + 0.5);
		right.pos[1] = Y;
		right.pos[0] = floor(minv2*Y + B2 + 0.5);
		perspectiveInpt(left, a, c, (Y-a.pos[1])/(c.pos[1]-a.pos[1]), perspective);
		perspectiveInpt(right, b, c, (Y-b.pos[1])/(c.pos[1]-b.pos[1]), perspective);
		perspectiveLine(colorBuff, zBuff, left, right, perspective);
		Y--;
	}
	if (floor(c.pos[1]+0.5) == Y) perspectiveLine(colorBuff, zBuff, c, c, perspective);
}

void drawLine2(FrameBuffer * colorBuff, vector<GLfloat> &zBuff, bool perspective)
{
	for (int i=0; i<(int)OBJ->numtriangles; ++i)
	{

		Vertex a(OBJ, OBJ->triangles[i].vindices[0], OBJ->triangles[i].tindices[0]);
		Vertex b(OBJ, OBJ->triangles[i].vindices[1], OBJ->triangles[i].tindices[1]);
		Vertex c(OBJ, OBJ->triangles[i].vindices[2], OBJ->triangles[i].tindices[2]);
		perspectiveLine(colorBuff, zBuff, a, b, perspective);
		perspectiveLine(colorBuff, zBuff, a, c, perspective);
		perspectiveLine(colorBuff, zBuff, b, c, perspective);
	}
}


void drawPlane2(FrameBuffer* colorBuff, vector<GLfloat> &zBuff, bool perspective)
{
	GLfloat scale = ( (colorBuff->getHeight() < colorBuff->getWidth())?colorBuff->getHeight(): colorBuff->getWidth() )/2;
	for (int i=0; i<(int)OBJ->numtriangles; ++i)
	{
		double theta1 = acos( (viewFaceNormals[i]*vec3(0,0,1))/(viewFaceNormals[i].length()) );
		double theta2 = atan( scale/LViewer::nearPlane );
		//cout << (theta1+theta2)/PI*180 << endl;
		if (theta1 + theta2 <  PI/2) continue;
		perspectiveTri(colorBuff, zBuff, OBJ->triangles[i], perspective);
	}
}

void scalingTrans(FrameBuffer * colorBuff)
{
	viewVertices.resize(OBJ->numvertices);

	// from [-1, 1] box to appropriate size
	GLfloat scale = ( (colorBuff->getHeight() < colorBuff->getWidth())?colorBuff->getHeight(): colorBuff->getWidth() )/2;

	mat4 S = 
		mat4(scale, 0, 0, 0,
		0, scale, 0, 0,
		0, 0, scale, 0,
		0, 0, 0, 1);

	for (int i=1; i<=(int)OBJ->numvertices; i++)
	{
		viewVertices[i-1] = S*vec3(OBJ->vertices[3*i], OBJ->vertices[3*i+1], OBJ->vertices[3*i+2]);
	}
}


mat4 modelviewTrans(const LCamera cam)
{
	viewFaceNormals.resize(OBJ->numfacetnorms);
	viewNormals.resize(OBJ->numnormals);
	
	// Model View Transform
	vec3 pos, target, up;
	cam.getVectors(pos, target, up);
	
	vec3 zaxis = (target-pos).normalize();
	vec3 yaxis = up.normalize();
	vec3 xaxis = (yaxis^zaxis).normalize();
	mat4 invT = 
	//mat4(vec4(xaxis,0),vec4(yaxis,0),vec4(zaxis,0),vec4(pos,1));
		mat4(xaxis[0], yaxis[0], zaxis[0], pos[0],
		xaxis[1], yaxis[1], zaxis[1], pos[1],
		xaxis[2], yaxis[2], zaxis[2], pos[2],
		0, 0, 0, 1);
	
	mat4 T = invT.inverse();
	/*cout << T[0][0] << " " << T[0][1] << " " << T[0][2] << " " << T[0][3] << "\n" ;
	cout << T[1][0] << " " << T[1][1] << " " << T[1][2] << " " << T[1][3] << "\n" ;
	cout << T[2][0] << " " << T[2][1] << " " << T[2][2] << " " << T[2][3] << "\n" ;
	cout << T[3][0] << " " << T[3][1] << " " << T[3][2] << " " << T[3][3] << "\n" ;*/
	for (int i=1; i<=(int)OBJ->numvertices; i++)
	{
		viewVertices[i-1] = T*viewVertices[i-1];
	}
	for (int i=1; i<=(int)OBJ->numfacetnorms; i++)
	{
		viewFaceNormals[i-1] = vec3(T*vec4(OBJ->facetnorms[i*3],OBJ->facetnorms[i*3+1],OBJ->facetnorms[i*3+2],0.0), 3);
	}
	for (int i=1; i<=(int)OBJ->numnormals; i++)
	{
		viewNormals[i-1] = vec3(T*vec4(OBJ->normals[i*3],OBJ->normals[i*3+1],OBJ->normals[i*3+2],0.0), 3);
	}
	return T;
}


void projectTrans(FrameBuffer * colorBuff)
{
	// Projection
	double Znear = LViewer::nearPlane;
	double Zfar = LViewer::farPlane;
	//GLfloat dmax = (d[0] > d[1])? d[0]:d[1];
	GLfloat scale = ( (colorBuff->getHeight() < colorBuff->getWidth())?colorBuff->getHeight(): colorBuff->getWidth() )/2;
	
	for (int i=1; i<=(int)OBJ->numvertices; i++)
	{
		viewVertices[i-1][0] = viewVertices[i-1][0] * Znear / viewVertices[i-1][2];
		viewVertices[i-1][1] = viewVertices[i-1][1] * Znear / viewVertices[i-1][2];
	}
	GLfloat* d = new GLfloat(3);
	GLfloat* c = new GLfloat(3);
	//dimension(viewVertices, d, c);
	//cout << "P*T*S*V: " << d[0] << "x" << d[1] << "x" << d[2] << endl;
	//cout << "center: (" << c[0] << "," << c[1] << "," << c[2] << ")" << endl;
}

void viewportTrans(FrameBuffer * colorBuff)
{	
	GLfloat scale = (colorBuff->getHeight() > colorBuff->getWidth())?colorBuff->getHeight(): colorBuff->getWidth();
	scale = scale / 2;
	mat4 portCenter =
		mat4(1, 0, 0, colorBuff->getWidth()/2,
		0, 1, 0, colorBuff->getHeight()/2,
		0, 0, 1, 0,
		0, 0, 0, 1);

	for (int i=1; i<=(int)OBJ->numvertices; i++)
	{
		viewVertices[i-1] = portCenter*viewVertices[i-1];
	}
}

void render(const LCamera cam, const LLight lit, FrameBuffer * colorBuff, RENDER_MODE renderingMode, PROJECT_MODE projectingMode, int modelIndex)
{	
	bool perspective = ((int)projectingMode +1) % 2 == 0;
	vector<GLfloat> zBuff(colorBuff->getWidth()*colorBuff->getHeight(), LViewer::farPlane);
	colorBuff->clear();

	OBJ = MODELS[modelIndex%MODELS.size()];
	imgid=modelIndex%MODELS.size();

	scalingTrans(colorBuff);
	modelviewTrans(cam);
	if (perspective) projectTrans(colorBuff);
	viewportTrans(colorBuff);


	vec3 lightS;
	lightS = lit.getPos();
	// how to use: it's just a position coordinate
	// 	light position ( lightS.n[0], lightS.n[1], lightS.n[2] )   initial value: (100,100,0)
	
	renderingMode;
	//how to use: here is an example
	if(renderingMode==WIRE){
		drawLine2(colorBuff, zBuff, perspective);
	}
	if(renderingMode==SOLID){
		drawPlane2(colorBuff, zBuff, perspective);
	}
	//for (unsigned i=0; i<colorBuff->getWidth()*colrBuff->getHeight(); ++i)
	//	if (zBuff[i] != LViewer::farPlane) cout << zBuff[i] << " ";

	//system("pause");
}

//////////////////////////////////////////////////////////////////////////
// init()
//	This function will be called once when the program starts.
//////////////////////////////////////////////////////////////////////////
void init(){
	//read an obj model here
	//GLMmodel* OBJ;
	/*
	OBJ = glmReadOBJ("cube.obj");
	MODELS.push_back(OBJ);
	OBJ = glmReadOBJ("santa7KN.obj");
	MODELS.push_back(OBJ);
	OBJ = glmReadOBJ("duck4KN.obj");
	MODELS.push_back(OBJ);
	OBJ = glmReadOBJ("Nala6KN.obj");
	MODELS.push_back(OBJ);
	*/
	//cout << "near:" << LViewer::nearPlane << " far:" << LViewer::farPlane << endl;*/
	//IMARRAY.clear();
	
	MODELS.clear();
	const char * const objfile[3]={"cube.obj","duck.obj","laurana500.obj"};
	const char * const texturefile[3]={"checker.ppm","duckCM.ppm","laurana500.ppm"};
	IMARRAY.clear();// = new image*[10];
	//OBJ = glmReadOBJ("duck.obj");
	//MODELS.push_back(OBJ);

	for(int i=0;i<=2;i++){
		OBJ = glmReadOBJ(objfile[i]);
		MODELS.push_back(OBJ);

		IMG = new image();
		string name1(texturefile[i]);
	//	string name2=".ppm";
		string name3=name1;//+name2;
		bool isread=IMG->readImg(name3.c_str());

		if(isread){
			cout<< IMG->width<<endl;
			cout<< IMG->height<<endl;
			IMARRAY.push_back(IMG);
		}
		else{
			cout<<"cannot read image"<<endl;
		}
	}
	cout<< IMARRAY.size()<<endl;
	cout<<MODELS.size()<<endl;
	IMG=IMARRAY[0];
//	ducktexture=*IMG;
	for (int i=0; i<MODELS.size(); ++i) {
		glmUnitize(MODELS[i]);
		//calculate the vertex normals
		glmFacetNormals(MODELS[i]);	
		glmVertexNormals(MODELS[i],90.0);
	}
	//the math library that can be used
}

int main(int argc, char ** argv)
{
	//get into a rendering loop
	initAndRunLViewer(screenWidth, screenHeight, render, init);
	
	//free the model obj
	glmDelete(OBJ);

	return 0;
}

