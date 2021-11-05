#pragma once
#include <math.h>
#include <stdlib.h>
#define PI 3.14159265358979323846

typedef struct
{
	float *verts;
	float *norms;
	float *texCoords;
	unsigned int *elements;
	unsigned int numVertices;
	unsigned int numElements;
	unsigned int maxElements;
}mesh;


void makeSphere(mesh *sphere, int slicesCount, int stacksCount, float radius);

