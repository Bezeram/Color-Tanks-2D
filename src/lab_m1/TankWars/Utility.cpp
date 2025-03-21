#include "lab_m1/TankWars/Utility.h"

float tw::clamp(float value, float min, float max)
{
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

float tw::clampMin(float value, float min)
{
	if (value < min)
		return min;
	return value;
}

float tw::clampMax(float value, float max)
{
	if (value > max)
		return max;
	return value;
}

float tw::clamp01(float x)
{
	if (x > 1)
		return 1;
	if (x < 0)
		return 0;
	return x;
}

float tw::lerp(float a, float b, float t)
{
	return a * (1 - t) + b * t;
}

glm::vec2 tw::lerp(const glm::vec2& a, const glm::vec2& b, float t)
{
	return { lerp(a.x, b.x, t), lerp(a.y, b.y, t) };
}

glm::vec3 tw::lerp(const glm::vec3& a, const glm::vec3& b, float t)
{
	return { lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t) };
}

int tw::sign(float x)
{
	if (x > 0)
		return 1;
	if (x < 0)
		return -1;
	return 0;
}

double tw::RandomDouble(double min, double max)
{
	double t = (double)rand() / RAND_MAX;
	return min * (1 - t) + max * t;
}

tw::TankControls::TankControls(int MoveLeft, int MoveRight, int RotateGunUp, int RotateGunDown, int Accelerate, int Shoot)
	: MoveLeft(MoveLeft), MoveRight(MoveRight), RotateGunUp(RotateGunUp), RotateGunDown(RotateGunDown), Accelerate(Accelerate), Shoot(Shoot) {}

Mesh* tw::CreateSquare(
	const std::string& name,
	glm::vec3 leftBottomCorner,
	float length,
	glm::vec3 color,
	bool fill)
{
	glm::vec3 corner = leftBottomCorner;

	std::vector<VertexFormat> vertices =
	{
		VertexFormat(corner, color),
		VertexFormat(corner + glm::vec3(length, 0, 0), color),
		VertexFormat(corner + glm::vec3(length, length, 0), color),
		VertexFormat(corner + glm::vec3(0, length, 0), color)
	};

	Mesh* square = new Mesh(name);
	std::vector<unsigned int> indices = { 0, 1, 2, 3 };

	if (!fill) {
		square->SetDrawMode(GL_LINE_LOOP);
	}
	else {
		// Draw 2 triangles. Add the remaining 2 indices
		indices.push_back(0);
		indices.push_back(2);
	}

	square->InitFromData(vertices, indices);
	return square;
}
