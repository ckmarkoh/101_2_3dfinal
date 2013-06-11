#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <stdio.h>
#include <string>

class pixelColor{
public:
	pixelColor(){
		this->color[0] = this->color[1] = this->color[2] = 0.0f;
	}
	float color[3];
};

class image
{
    public:
		image();
        virtual ~image();
		
		vector<pixelColor> data;

		//the width and the height of the image
		int width;
		int height;

		//read the image and store it at the data
		bool readImg( const char* fileName );
		
		//modify here
		//-------------------------------------------------------------------------
		//get the pixel color at (u,v)
		//u,v are between 0.0 and 1.0
		//....
		//-------------------------------------------------------------------------
};

#endif // IMAGE_H
