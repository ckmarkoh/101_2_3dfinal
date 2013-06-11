// 3DGP_framework.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>

#define PI 3.14159265
vector<GLMmodel*> MODELS;
GLMmodel* OBJ;
int screenWidth = 600;
int screenHeight = 400;

//////////////////////////////////////////////////////////////////////////
// render
//	This is the rendering function
//////////////////////////////////////////////////////////////////////////


struct Vector
{
	GLfloat x, y, z;
	Vector operator- (Vector p)
	{
		Vector ret;
		ret.x = x-p.x;
		ret.y = y-p.y;
		ret.z = z-p.z;
		//MSG(ret.x); MSG(ret.y);	MSG(ret.z);
		return ret;
	}

};

struct Vertex
{
	Vertex() {};
	Vertex (GLMmodel* obj, int index) {
		//cout << index << endl;
		pos.x = obj->vertices[index*3+0];
		pos.y = obj->vertices[index*3+1];
		pos.z = obj->vertices[index*3+2];
		col.r = 255*obj->colors[index*3+0];
		col.g = 255*obj->colors[index*3+1];
		col.b = 255*obj->colors[index*3+2];
	}
	Vector pos;
	Color col;
};


void translate(GLfloat dx, GLfloat dy, GLfloat dz)
{
	for (int i=1; i<=(int)OBJ->numvertices; i++) {
		OBJ->vertices[3 * i + 0] += dx;
		OBJ->vertices[3 * i + 1] += dy;
		OBJ->vertices[3 * i + 2] += dz;
	}
}

void scale(GLfloat sx, GLfloat sy, GLfloat sz)
{
	for (int i=1; i<=(int)OBJ->numvertices; i++) {
		OBJ->vertices[3 * i + 0] *= sx;
		OBJ->vertices[3 * i + 1] *= sy;
		OBJ->vertices[3 * i + 2] *= sz;
	}
}

void rotate(GLfloat r_x, GLfloat r_y, GLfloat r_z)
{
	//GLfloat * R = (GLfloat*) malloc(sizeof(GLfloat)*16);

	GLfloat v0, v1, v2;
	if (r_x != 0)
	{
		GLfloat c = cos(r_x*PI/180);
		GLfloat s = sin(r_x*PI/180);

		/*R[0]  = 1; R[1]  = 0; R[2]  = 0; R[3]  = 0;
		R[4]  = 0; R[5]  = c; R[6]  =-s; R[7]  = 0;
		R[8]  = 0; R[9]  = s; R[10] = c; R[11] = 0;
		R[12] = 0; R[13] = 0; R[14] = 0; R[15] = 1;*/
		for (int i=1; i<=(int)OBJ->numvertices; i++) {
			v1 = c*OBJ->vertices[3*i+1]-s*OBJ->vertices[3*i+2];
			v2 = s*OBJ->vertices[3*i+1]+c*OBJ->vertices[3*i+2];
			OBJ->vertices[3*i+1] = v1;
			OBJ->vertices[3*i+2] = v2;
		}
	}
	if (r_y != 0)
	{
		GLfloat c = cos(r_y*PI/180);
		GLfloat s = sin(r_y*PI/180);

		/*R[0]  = c; R[1]  = 0; R[2]  = s; R[3]  = 0;
		R[4]  = 0; R[5]  = 1; R[6]  = 0; R[7]  = 0;
		R[8]  =-s; R[9]  = 0; R[10] = c; R[11] = 0;
		R[12] = 0; R[13] = 0; R[14] = 0; R[15] = 1;*/
		for (int i=1; i<=(int)OBJ->numvertices; i++) {
			v0 = c*OBJ->vertices[3*i+0]+s*OBJ->vertices[3*i+2];
			v2 = -s*OBJ->vertices[3*i+0]+c*OBJ->vertices[3*i+2];
			OBJ->vertices[3*i+0] = v0;
			OBJ->vertices[3*i+2] = v2;
		}
	}


	if (r_z != 0)
	{
		GLfloat c = cos(r_z*PI/180);	
		GLfloat s = sin(r_z*PI/180);	

		/*R[0]  = c; R[1]  =-s; R[2]  = 0; R[3]  = 0;
		R[4]  = s; R[5]  = c; R[6]  = 0; R[7]  = 0;
		R[8]  = 0; R[9]  = 0; R[10] = 1; R[11] = 0;
		R[12] = 0; R[13] = 0; R[14] = 0; R[15] = 1;*/
		for (int i=1; i<=(int)OBJ->numvertices; i++) {
			v0 = c*OBJ->vertices[3*i+0]-s*OBJ->vertices[3*i+1];
			v1 = s*OBJ->vertices[3*i+0]+c*OBJ->vertices[3*i+1];
			OBJ->vertices[3*i+0] = v0;
			OBJ->vertices[3*i+1] = v1;
		}
	}
}

void rasterLine(FrameBuffer * colorBuff, Vertex a, Vertex b)
{
	//cout << "line:(" << a.pos.x << " " << a.pos.y << " " << a.pos.z << ") -> (" <<
	//	b.pos.x << " " << b.pos.y << " " << b.pos.z << ")" << endl;
	Vector p;
	p = b.pos - a.pos;
	Vertex tmp;
	GLfloat x, y;
	//int pnt;
	if (p.x == 0 && p.y == 0)
	{
		//pnt = int(a.pos.y)*W+int(a.pos.x);
		if (colorBuff->getDepth((int)a.pos.x, (int)a.pos.y) < a.pos.z)
		{
			colorBuff->setDepth((int)a.pos.x, (int)a.pos.y, a.pos.z);
			colorBuff->setColor((int)a.pos.x, (int)a.pos.y, a.col);
			//assert(buf[pnt].color.r <= 1);
		}
		// interpolate vetex normal
	}
	else if (p.x == 0) // vertical lines
	{
		if (p.y < 0) swap(a, b);
		x = a.pos.x;
		y = a.pos.y;

		while (y < b.pos.y)
		{
			//pnt = int(y)*W+int(x);
			GLfloat d = a.pos.z	+ (b.pos.z-a.pos.z) * (y-a.pos.y)/(b.pos.y-a.pos.y);
			if (colorBuff->getDepth((int)x, (int)y) < d)
			{
				colorBuff->setDepth((int)x, (int)y, d);
				colorBuff->setColor((int)x, (int)y, a.col + (b.col-a.col) * ((y-a.pos.y)/(b.pos.y-a.pos.y)));
				//assert(buf[pnt].color.r <= 1);
			}
			y++;
		}
	}
	else if (p.y == 0) // horizontal line
	{
		if (p.x < 0) swap(a, b);
		x = a.pos.x;
		y = a.pos.y;
		while (x < b.pos.x)
		{
			//pnt = int(y)*W+int(x);
			GLfloat d = a.pos.z	+ (b.pos.z-a.pos.z) * (x-a.pos.x)/(b.pos.x-a.pos.x);
			if (colorBuff->getDepth((int)x, (int)y) < d)
			{
				colorBuff->setDepth((int)x, (int)y, d);
				colorBuff->setColor((int)x, (int)y, a.col + (b.col-a.col) * ((x-a.pos.x)/(b.pos.x-a.pos.x)));
				//assert(buf[pnt].color.r <= 1);
			}
			x++;
		}
	}
	else
	{
		GLfloat m = p.y / p.x;
		if (m <= 1&& m >= -1)
		{
			if (p.x < 0) swap(a, b);
			x = a.pos.x;
			y = a.pos.y;
			while (x < b.pos.x)
			{
				//pnt = int(y)*W+int(x);
				GLfloat d = a.pos.z	+ (b.pos.z-a.pos.z) * (x-a.pos.x)/(b.pos.x-a.pos.x);
				if (colorBuff->getDepth((int)x, (int)y) < d)
				{
					colorBuff->setDepth((int)x, (int)y, d);
					colorBuff->setColor((int)x, (int)y, a.col + (b.col-a.col) * ((x-a.pos.x)/(b.pos.x-a.pos.x)));
					//assert(buf[pnt].color.r <= 1);
				}
				x++; y = y+m;
			}
		}
		else
		{
			if (p.y < 0) swap(a, b);
			x = a.pos.x;
			y = a.pos.y;
			while (y < b.pos.y)
			{
				//pnt = int(y)*W+int(x);
				GLfloat d = a.pos.z	+ (b.pos.z-a.pos.z) * (y-a.pos.y)/(b.pos.y-a.pos.y);
				if (colorBuff->getDepth((int)x, (int)y) < d)
				{
					colorBuff->setDepth((int)x, (int)y, d);
					colorBuff->setColor((int)x, (int)y, a.col + (b.col-a.col) * ((y-a.pos.y)/(b.pos.y-a.pos.y)));
					//assert(buf[pnt].color.r <= 1);
				}
				y++; x = x+1/m;
			}
		}
	}
}

Vertex interpolate(Vertex a, Vertex b, float x, float y)
{
	Vertex ret;
	ret.pos.x = x;
	ret.pos.y = y;
	if (a.pos.x > b.pos.x) swap(a,b);
	//assert(x>=a.pos.x && x<=b.pos.x);

	Vector p = b.pos-a.pos;
	if (p.x == 0 && p.y == 0)
	{
		if (a.pos.z > b.pos.z) swap(a, b);
		ret.pos.z = a.pos.z;
		ret.col = a.col;
	}
	else if (p.x == 0)
	{
		ret.pos.z = a.pos.z	+ (b.pos.z-a.pos.z) * (y-a.pos.y)/(b.pos.y-a.pos.y);
		ret.col = a.col	+ (b.col-a.col) * ((y-a.pos.y)/(b.pos.y-a.pos.y));
	}
	else
	{
		float m = p.y/p.x;
		if (m<=1 && m>=-1)
		{
			ret.pos.z = a.pos.z	+ (b.pos.z-a.pos.z) * (x-a.pos.x)/(b.pos.x-a.pos.x);
			ret.col = a.col	+ (b.col-a.col) * ((x-a.pos.x)/(b.pos.x-a.pos.x));
		}
		else
		{
			ret.pos.z = a.pos.z	+ (b.pos.z-a.pos.z) * (y-a.pos.y)/(b.pos.y-a.pos.y);
			ret.col = a.col	+ (b.col-a.col) * ((y-a.pos.y)/(b.pos.y-a.pos.y));
		}
	}
	return ret;
}

void rasterTriangle(FrameBuffer* buf, GLMtriangle t)
{
	Vertex a(OBJ, t.vindices[0]);
	Vertex b(OBJ, t.vindices[1]);
	Vertex c(OBJ, t.vindices[2]);

	if (b.pos.y > a.pos.y) swap(a, b);
	if (c.pos.y > a.pos.y) swap(a, c);
	if (c.pos.y > b.pos.y) swap(b, c);

	Vector ab = b.pos-a.pos;
	Vector bc = c.pos-b.pos;
	Vector ca = a.pos-c.pos;

	Vertex left, right;
	GLfloat y, x1, x2;
	GLfloat m1_inv, m2_inv;

	left = a;
	right = a;
	x1 = a.pos.x;
	x2 = a.pos.x;
	y = a.pos.y;
	if (ab.y == 0) // top
	{
		right = b;
		x2 = b.pos.x;
		m1_inv = ca.x/ca.y;
		m2_inv = bc.x/bc.y;
		rasterLine(buf, left, right);
		y--;
		while (y > c.pos.y)
		{
			x1=x1-m1_inv; x2=x2-m2_inv;
			left = interpolate(a, c, x1, y); 
			right = interpolate(b, c, x2, y); 
			rasterLine(buf, left, right);
			y--;
		}
	}
	else if (bc.y == 0) // bottom
	{
		m1_inv = ab.x/ab.y;
		m2_inv = ca.x/ca.y;
		rasterLine(buf, left, right);
		y--;
		while (y > c.pos.y)
		{
			x1=x1-m1_inv; x2=x2-m2_inv;
			left = interpolate(a, b, x1, y); 
			right = interpolate(c, a, x2, y); 
			rasterLine(buf, left, right);
			y--;
		}
	}
	else
	{
		m1_inv = ab.x/ab.y;
		m2_inv = ca.x/ca.y;
		rasterLine(buf, left, right);
		y--;
		while (y > b.pos.y)
		{
			x1=x1-m1_inv; x2=x2-m2_inv;
			left = interpolate(a, b, x1, y); 
			right = interpolate(c, a, x2, y); 
			rasterLine(buf, left, right);
			y--;
		}

		m1_inv = bc.x/bc.y;
		y = b.pos.y;
		x1 = b.pos.x;
		x2 = a.pos.x + m2_inv*(y-a.pos.y);
		left = b;
		right = interpolate(c, a, x2, y);
		rasterLine(buf, left, right);
		y--;
		while (y > c.pos.y)
		{
			x1=x1-m1_inv; x2=x2-m2_inv;
			left = interpolate(b, c, x1, y); 
			right = interpolate(c, a, x2, y); 
			rasterLine(buf, left, right);
			y--;
		}
	}
}


void drawLine(FrameBuffer * colorBuff)
{
	for (int i=0; i<(int)OBJ->numtriangles; ++i)
	{
		//cout << "tri:" << i << endl;
		//if (tri[i].N.z < 0) continue; // backface
		Vertex a(OBJ, OBJ->triangles[i].vindices[0]);
		Vertex b(OBJ, OBJ->triangles[i].vindices[1]);
		Vertex c(OBJ, OBJ->triangles[i].vindices[2]);
		rasterLine(colorBuff, a, b);
		rasterLine(colorBuff, a, c);
		rasterLine(colorBuff, b, c);
	}
}

void drawPlane(FrameBuffer* colorBuff)
{
	for (int i=0; i<(int)OBJ->numtriangles; ++i)
	{
		int indfn = OBJ->triangles[i].findex;
		if (OBJ->facetnorms[indfn*3+2] >= 0) continue;
		rasterTriangle(colorBuff, OBJ->triangles[i]);
	}
}

void scale2Screen(GLMmodel* model)
{
	GLfloat s = -1 + ((screenHeight > screenWidth)? screenWidth: screenHeight);
	//GLfloat scale = -1 + ((LViewer::getHeight > LViewer::getWidth)? LViewer::getWidth: LViewer::getHeight);
	translate(1, 1, 1);
	scale(s/2, s/2, s/2);
	/*for (int i = 1; i <= model->numvertices; i++) {
		model->vertices[3 * i + 0] += 1;
		model->vertices[3 * i + 1] += 1;
		model->vertices[3 * i + 2] += 1;
		model->vertices[3 * i + 0] *= s/2;
		model->vertices[3 * i + 1] *= s/2;
		model->vertices[3 * i + 2] *= s/2;
	}*/
}
void render(const LCamera cam, const LLight lit, FrameBuffer * colorBuff, RENDER_MODE renderingMode, int modelIndex)
{	

	colorBuff->clear();

	OBJ = MODELS[modelIndex%MODELS.size()];
	modelIndex;
	//model index
	//integer
	//-2, -1, 0, 1, 2....initial value:0

	vec3 pos,target,up;
	cam.getVectors(pos, target, up);
	
	//cout << pos.n[0] << " " << pos.n[1] << " " << pos.n[2] << endl;
	// how to use: just like the input of gluLookAt
	// 	camera position (   pos.n[0],    pos.n[1],   pos.n[2] )   initial value: (0,250,0)
	// 	camera target   (target.n[0], target.n[1],target.n[2] )   initial value: (0,249,0)
	// 	camera upVector (    up.n[0],     up.n[1],    up.n[2] )   initial value: (1,0,0)
	
	vec3 lightS;
	lightS = lit.getPos();
	// how to use: it's just a position coordinate
	// 	light position ( lightS.n[0], lightS.n[1], lightS.n[2] )   initial value: (100,100,0)
	
	//to assign pixel color
	//cout << colorBuff->getHeight() << " " << colorBuff->getWidth() << endl;
	//how to use: here is an example
	/*Color cc;
	for(int i=0;i<colorBuff->getWidth();i++){
		for(int j=0;j<colorBuff->getHeight();j++){
			cc.r = rand()%256;
			cc.g = rand()%256;
			cc.b = rand()%256;
			cc.r = 255*i/colorBuff->getWidth();
			cc.g = 255*i/colorBuff->getWidth();
			cc.b = 255*i/colorBuff->getWidth();
			colorBuff->setColor(i,j,cc);
		}
	}*/
	
	renderingMode;
	//how to use: here is an example
	if(renderingMode==WIRE){
		//the rendering mode is WIRE
		drawLine(colorBuff);
	}
	if(renderingMode==SOLID){
		//the rendering mode is SOLID
		drawPlane(colorBuff);
	}

	//how to use: here is an example
	//traverse the triangle of the model (and render them urself)
	/*for(int i=0;i<(int)OBJ->numtriangles;i++){

		//the index of each vertices
		int indv1 = OBJ->triangles[i].vindices[0];
		int indv2 = OBJ->triangles[i].vindices[1];
		int indv3 = OBJ->triangles[i].vindices[2];

		//the index of each color (same as the index of vertex
		int indc1 = indv1;
		int indc2 = indv2;
		int indc3 = indv3;

		//the index of each normals
		int indn1 = OBJ->triangles[i].nindices[0];
		int indn2 = OBJ->triangles[i].nindices[1];
		int indn3 = OBJ->triangles[i].nindices[2];

		//the index of the face normal of the triangle i
		int indfn = OBJ->triangles[i].findex;
		
		//the index of each texture coordinates
		int indt1 = OBJ->triangles[i].tindices[0];
		int indt2 = OBJ->triangles[i].tindices[1];
		int indt3 = OBJ->triangles[i].tindices[2];

		//for each channel (x,y,z)
		for(int j=0;j<3;j++){
			//vertex data			
			cout << j << ": ";
			cout << OBJ->vertices[indv1*3+j] << " ";
			cout << OBJ->vertices[indv2*3+j] << " ";
			cout << OBJ->vertices[indv3*3+j] << endl;
			OBJ->vertices[indv1*3+j];
			OBJ->vertices[indv2*3+j];
			OBJ->vertices[indv3*3+j];

			//color data

			OBJ->colors[indc1*3+j];
			OBJ->colors[indc2*3+j];
			OBJ->colors[indc3*3+j];

			//normal data
			OBJ->normals[indn1*3+j];
			OBJ->normals[indn2*3+j];
			OBJ->normals[indn3*3+j];

			//face normal data
			OBJ->facetnorms[indfn*3+j];
		}
		system("pause");
		//for each channel of texture coordinates
		for(int j=0;j<2;j++){
			OBJ->texcoords[indt1*2+j];
			OBJ->texcoords[indt2*2+j];
			OBJ->texcoords[indt3*2+j];
		}

		//render this triangle
		//bla bla bla bla bla bla bla bla bla bla bla bla....
	}*/
	//system("pause");
}


//////////////////////////////////////////////////////////////////////////
// init()
//	This function will be called once when the program starts.
//////////////////////////////////////////////////////////////////////////
void init(){
	//read an obj model here
	//GLMmodel* OBJ;
	OBJ = glmReadOBJ("bunny5KN.obj");
	MODELS.push_back(OBJ);
	OBJ = glmReadOBJ("duck4KN.obj");
	MODELS.push_back(OBJ);
	OBJ = glmReadOBJ("Nala6KN.obj");
	MODELS.push_back(OBJ);
	//OBJ = glmReadOBJ("cube.obj");
	
	for (int i=0; i<MODELS.size(); ++i) {
		OBJ = MODELS[i];
		//rotate(0, 45.0, 45.0);
		//move the object to the origin
		//scale to -1~+1
		glmUnitize(OBJ);
		scale2Screen(OBJ);
		//calculate the vertex normals
		glmFacetNormals(OBJ);	
		glmVertexNormals(OBJ,90.0);
	}
	//the math library that can be used
	vec3;
	vec4;
	mat3;
	mat4;
	identity3D;
	translation3D;
	rotation3D;
	scaling3D;
	//......
	//cout << OBJ->position[0] << " " << OBJ->position[1] << " " << OBJ->position[2] << endl;

	//maybe u can do some initialization here....
}




int main(int argc, char ** argv)
{
	//get into a rendering loop
	initAndRunLViewer(screenWidth, screenHeight, render, init);
	
	//free the model obj
	glmDelete(OBJ);

	return 0;
}

