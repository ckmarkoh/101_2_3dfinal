#include "stdafx.h"
#include "image.h"

image::image(){
	this->data.clear();
}

image::~image(){
	this->data.clear();
}

bool image::readImg( const char* fileName ){
	FILE * file;

	//open the file
	file = fopen( fileName, "r" );
	if(string(fileName)==""){
		return false;
	}
	if( !file ){
		return false;
	}

	//read the image information
	fscanf( file, "%*s" );
	fscanf( file, "%d %d", &this->width, &this->height );
	fscanf( file, "%*d");

	this->data.resize( this->width * this->height );

	for( int i=this->height-1; i>=0; i-- ){
		for( int j=0; j<this->width; j++ ){
			for( int k=0; k<3; k++ ){
				int tmp;
				fscanf( file, "%d", &tmp );
				this->data[ i*this->width + j ].color[k] = (float)tmp / 255.0f;
			}
		}
	}

	fclose(file);
	return true;
}
