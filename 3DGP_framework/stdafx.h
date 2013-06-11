// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

//standard library
#include <iostream>
#include <cstdlib>
using namespace std;

#include "targetver.h"

//openGL related library
#include "GL\GLAux.h"
#include "GL\glut.h"
#pragma comment( lib, "glut32.lib")		
#pragma comment( lib, "glAux.lib")

//the framework library
#include "framework\framework.h"

//the OBJ model library
#include "framework\GLM.h"

//the math library
using namespace LLib::Math;

// TODO: reference additional headers your program requires here

#include "framework\include\LLib.h"
using LLib::Viewer::LViewer;