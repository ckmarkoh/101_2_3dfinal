#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include <stdio.h>
#include "include/algebra3/algebra3.h"
#include <vector>

using std::vector;
using LLib::Math::vec3;


enum RENDER_MODE
{
	WIRE = 0,
	SOLID= 1,
	NUM
};

enum PROJECT_MODE
{
	PARALLEL = 0,
	PERSPECTIVE= 1,
};

class LCamera
{
protected:
	vec3 _pos;
	vec3 _target;
	vec3 _up;

public:
	LCamera(const vec3 pos, const vec3 tgt, const vec3 up): _pos(pos), _target(tgt), _up(up) {};
	void getVectors( vec3 &pos, vec3 &target, vec3 &up)	const {	pos = _pos; target = _target; up = _up;}
};

class LLight
{
protected:
	vec3 _pos;

public:
	LLight(const vec3 pos): _pos(pos) {};
	inline const vec3& getPos() const { return _pos;}
};

class Color
{
public:
	unsigned char r;
	unsigned char g;
	unsigned char b;

	Color():r(0u), g(0u), b(0u)	{}
	Color(const unsigned char *c) {r=c[0]; g=c[1]; g=c[2];};
	Color(const unsigned char _r, const unsigned char _g, const unsigned char _b):r(_r), g(_g), b(_b) {};
	Color(const Color & c)		{	this->r = c.r; this->g = c.g, this->b = c.b;}
	Color & operator=(const Color &c)	{	this->r = c.r; this->g = c.g, this->b = c.b; return *this;}
	Color & operator+=(const Color &c)	{	this->r += c.r; this->g += c.g, this->b += c.b;return *this;}
	Color operator+(const Color &c)	{ Color ret; ret.r = r+c.r; ret.g = g+c.g; ret.b = b+c.b;return ret;}
	Color & operator-=(const Color &c)	{	this->r -= c.r; this->g -= c.g, this->b -= c.b;return *this;}
	Color operator-(const Color &c)	{ Color ret; ret.r = r-c.r; ret.g = g-c.g; ret.b = b-c.b;return ret;}
	Color operator* (GLfloat f)
	{
		Color ret;
		ret.r = r*f;
		ret.g = g*f;
		ret.b = b*f;
		return ret;
	}
};


class FrameBuffer
{
protected:
	vector<Color> _colors;
	vector<GLfloat> _depth;
	unsigned int _w;
	unsigned int _h;

public:
	FrameBuffer(unsigned int width, unsigned int height): _w(width), _h(height)	{ 
		_colors.resize(_w*_h);
		//_depth.resize(_w*_h);
		//for (int i=0; i<_w*_h; i++) _depth[i]=0;
	}
	inline void setColor(unsigned int x, unsigned int y, Color &color)	{	if(isValidCoord(x, y)) {_colors[x+y*_w] = color;}	};
	inline void setColor(unsigned int x, unsigned int y, vec3 &color)	{	
		if(isValidCoord(x, y)) { _colors[x+y*_w].r = 255*color[0]; _colors[x+y*_w].g = 255*color[1]; _colors[x+y*_w].b = 255*color[2];}	
	};
	//inline GLfloat getDepth(unsigned int x, unsigned int y)	{	if(isValidCoord(x, y)) {return _depth[x+y*_w];}	};
	//inline void setDepth(unsigned int x, unsigned int y, GLfloat depth)	{	if(isValidCoord(x, y)) {_depth[x+y*_w] = depth;}	};
	inline const unsigned char* getUCharArray()	const {return (unsigned char*) &_colors[0]; }

	void clear()	{	memset(&_colors[0], 0u, _w*_h*3); }

	inline unsigned int getWidth() const {return _w;} 
	inline unsigned int getHeight() const {return _h;} 

	bool saveAsPPM(const char * fname)	const
	{
		FILE * fp;
		if(NULL == (fp = fopen(fname, "w")))	{	return false;}

/*/ P6
		fprintf(fp, "P6 %d %d 255 ", _w, _h);
		const unsigned char * bits = getUCharArray();
		for(int y=_h -1; y>=0; y--)
		{
			for (unsigned int x=0; x<_w; x++)
			{
				const unsigned char * pixel = &(bits[(x+y*_w)*3]);		
				fwrite(pixel, sizeof(unsigned char), 3, fp);
			}
		}
		//fwrite(bits, sizeof(unsigned char), _w*_h*3, fp);
// */

// P3
		fprintf(fp, "P3 %d %d 255\n", _w, _h);
		const unsigned char * bits = getUCharArray();
		for(int y=_h -1; y>=0; y--)
		{
			for (unsigned int x=0; x<_w; x++)
			{
				const unsigned char * pixel = &(bits[(x+y*_w)*3]);		
				//fwrite(pixel, sizeof(unsigned char), 3, fp);
				fprintf(fp, "%u %u %u ", pixel[0], pixel[1], pixel[2]);
			}
		}
		//fwrite(bits, sizeof(unsigned char), _w*_h*3, fp);
		// */


		fclose(fp);

		return true;
	}


private:
	inline bool isValidCoord(unsigned int x, unsigned int y)	{	return (x<_w && y<_h);}
};



int initAndRunLViewer( const int winW, const int winH, void (*render)(const LCamera cam, const LLight lit, FrameBuffer * colorBuff, RENDER_MODE rmode, PROJECT_MODE pmode, int modelIndex), void (*initFun)());

#endif