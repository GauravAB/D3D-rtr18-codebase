#include "sphere.h"



void makeSphere(mesh *sphere, int slicesCount, int stacksCount, float radius)
{
	void addTriangle(float[4][3], float[4][3], float[4][2], mesh*);

	unsigned int totalVertices = slicesCount * stacksCount * 6;

	//point the pointer to a memory

	sphere->maxElements = totalVertices;
	sphere->numElements = 0;
	sphere->numVertices = 0;

	unsigned int numTriangles = totalVertices / 3;


	sphere->verts = (float*)calloc(totalVertices * 3, sizeof(float));
	sphere->norms = (float*)malloc(sizeof(float)*numTriangles * 3 * 3);
	sphere->texCoords = (float*)malloc(sizeof(float)*numTriangles * 3 * 2);
	sphere->elements = (unsigned int*)malloc(sizeof(unsigned int)*totalVertices);
	//on to the sphere algorithm
	float rhoStep = (float)(PI / stacksCount);
	float thetaStep = (float)((2.0* PI) / slicesCount);
	float ds = 1.0f / slicesCount;
	float dt = 1.0f / stacksCount;
	float t = 1.0f;
	float s = 0.0f;

	int i = 0;
	int j = 0;

	for (i = 0; i < stacksCount; i++)
	{
		float rho = (i * rhoStep);
		float srho = (float)sin(rho);
		float crho = (float)cos(rho);
		float srhorhoStep = (float)sin(rho + rhoStep);
		float crhorhoStep = (float)cos(rho + rhoStep);

		s = 0.0f;

		//initialization of arrays for a single quad patch

		float vertex[4][3];
		float normal[4][3];
		float texture[4][2];

		for (int p = 0; p < 4; p++)
		{
			for (int q = 0; q < 3; q++)
			{
				vertex[p][q] = 0;
				normal[p][q] = 0;
				texture[p][0] = 0;
				texture[p][1] = 0;
			}
		}

		for (j = 0; j < slicesCount; j++)
		{
			float theta = (j == slicesCount) ? 0.0f : j * thetaStep;
			float stheta = (float)-sin(theta);
			float ctheta = (float)cos(theta);

			//spherical to cartesian conversion

			float x = stheta * srho;
			float y = ctheta * srho;
			float z = crho;

			texture[0][0] = s;
			texture[0][1] = t;
			normal[0][0] = x;
			normal[0][1] = y;
			normal[0][2] = z;
			vertex[0][0] = x * radius;
			vertex[0][1] = y * radius;
			vertex[0][2] = z * radius;

			x = stheta * srhorhoStep;
			y = ctheta * srhorhoStep;
			z = crhorhoStep;

			texture[1][0] = s;
			texture[1][1] = t - dt;
			normal[1][0] = x;
			normal[1][1] = y;
			normal[1][2] = z;
			vertex[1][0] = x * radius;
			vertex[1][1] = y * radius;
			vertex[1][2] = z * radius;

			theta = ((j + 1) == slicesCount) ? 0.0f : (j + 1) * thetaStep;
			stheta = (float)-sin(theta);
			ctheta = (float)cos(theta);

			x = stheta * srho;
			y = ctheta * srho;
			z = crho;

			s += ds;
			texture[2][0] = s;
			texture[2][1] = t;
			normal[2][0] = x;
			normal[2][1] = y;
			normal[2][2] = z;
			vertex[2][0] = x * radius;
			vertex[2][1] = y * radius;
			vertex[2][2] = z * radius;

			x = stheta * srhorhoStep;
			y = ctheta * srhorhoStep;
			z = crhorhoStep;

			texture[3][0] = s;
			texture[3][1] = t - dt;
			normal[3][0] = x;
			normal[3][1] = y;
			normal[3][2] = z;
			vertex[3][0] = x * radius;
			vertex[3][1] = y * radius;
			vertex[3][2] = z * radius;

			addTriangle(vertex, normal, texture, sphere);

			vertex[0][0] = vertex[1][0];
			vertex[0][1] = vertex[1][1];
			vertex[0][2] = vertex[1][2];
			normal[0][0] = normal[1][0];
			normal[0][1] = normal[1][1];
			normal[0][2] = normal[1][2];
			texture[0][0] = texture[1][0];
			texture[0][1] = texture[1][1];

			vertex[1][0] = vertex[3][0];
			vertex[1][1] = vertex[3][1];
			vertex[1][2] = vertex[3][2];
			normal[1][0] = normal[3][0];
			normal[1][1] = normal[3][1];
			normal[1][2] = normal[3][2];
			texture[1][0] = texture[3][0];
			texture[1][1] = texture[3][1];

			addTriangle(vertex, normal, texture, sphere);

		}
		t -= dt;
	}
}


void addTriangle(float verts[4][3], float norms[4][3], float tex[4][2], mesh* m)
{
	int isFoundIdentical(float, float, float);
	void normalizeVector(float* v);


	unsigned int i, j;
	const float diff = 0.00001f;

	normalizeVector(norms[0]);
	normalizeVector(norms[1]);
	normalizeVector(norms[2]);


	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < m->numVertices; j++)
		{
			if (isFoundIdentical(m->verts[j * 3], verts[i][0], diff) &&
				isFoundIdentical(m->verts[(j * 3) + 1], verts[i][1], diff) &&
				isFoundIdentical(m->verts[(j * 3) + 2], verts[i][2], diff) &&

				isFoundIdentical(m->norms[j * 3], norms[i][0], diff) &&
				isFoundIdentical(m->norms[(j * 3) + 1], norms[i][1], diff) &&
				isFoundIdentical(m->norms[(j * 3) + 2], norms[i][2], diff) &&

				isFoundIdentical(m->texCoords[(j * 2)], tex[i][0], diff) &&
				isFoundIdentical(m->texCoords[(j * 2) + 1], tex[i][1], diff)
				)
			{
				m->elements[m->numElements] = j;
				m->numElements += 1;
				break;
			}
		}

		//non repeated then append
		if (j == m->numVertices && m->numVertices < m->maxElements && m->numElements < m->maxElements)
		{
			m->verts[(m->numVertices * 3)] = verts[i][0];
			m->verts[(m->numVertices * 3) + 1] = verts[i][1];
			m->verts[(m->numVertices * 3) + 2] = verts[i][2];

			m->norms[(m->numVertices * 3)] = norms[i][0];
			m->norms[(m->numVertices * 3) + 1] = norms[i][1];
			m->norms[(m->numVertices * 3) + 2] = norms[i][2];

			m->texCoords[(m->numVertices * 2)] = tex[i][0];
			m->texCoords[(m->numVertices * 2) + 1] = tex[i][1];


			m->elements[m->numElements] = m->numVertices;
			m->numElements += 1;
			m->numVertices += 1;
		}
	}
}

int isFoundIdentical(float v1, float v2, float diff)
{
	if (fabs((v1 - v2)) < diff)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


void normalizeVector(float* v)
{
	float sqLength = (v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]);

	float sqrtOfsqLength = (float)sqrt(sqLength);

	v[0] = v[0] * 1.0f / sqrtOfsqLength;
	v[1] = v[1] * 1.0f / sqrtOfsqLength;
	v[2] = v[2] * 1.0f / sqrtOfsqLength;
}
