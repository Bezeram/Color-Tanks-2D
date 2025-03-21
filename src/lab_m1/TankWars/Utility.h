#pragma once

#include "components/simple_scene.h"
#include "utils/glm_utils.h"

namespace transform2D
{
	// Translate matrix
	inline glm::mat3 Translate(float translateX, float translateY)
	{
		return glm::transpose
		(
			glm::mat3
			(
				1, 0, translateX,
				0, 1, translateY,
				0, 0, 1
			)
		);

	}

	inline glm::mat3 Translate(glm::vec2 translate)
	{
		return glm::transpose
		(
			glm::mat3
			(
				1, 0, translate.x,
				0, 1, translate.y,
				0, 0, 1
			)
		);

	}

	// Scale matrix
	inline glm::mat3 Scale(float scaleX, float scaleY)
	{
		return glm::transpose
		(
			glm::mat3
			(
				scaleX, 0, 0,
				0, scaleY, 0,
				0, 0, 1
			)
		);
	}

	inline glm::mat3 Scale(glm::vec2 scale)
	{
		return glm::transpose
		(
			glm::mat3
			(
				scale.x, 0, 0,
				0, scale.y, 0,
				0, 0, 1
			)
		);
	}

	// Rotate matrix
	inline glm::mat3 Rotate(float radians)
	{
		return glm::transpose
		(
			glm::mat3
			(
				cos(radians), -sin(radians), 0,
				sin(radians), cos(radians), 0,
				0, 0, 1
			)
		);
	}
}   // namespace transform2D

namespace tw
{
	struct RenderData
	{
		RenderData(const char* meshName, const char* shaderType, const glm::mat3& modelMatrix)
			: meshName(meshName), shaderType(shaderType), modelMatrix(modelMatrix)
		{}
		const char* meshName;
		const char* shaderType;
		glm::mat3 modelMatrix;
	};

	struct TankControls
	{
		TankControls() = default;
		TankControls(int MoveLeft, int MoveRight, int RotateGunUp, int RotateGunDown, int Accelerate, int Shoot);
		int MoveLeft = -1, MoveRight = -1, RotateGunUp = -1, RotateGunDown = -1, Accelerate = -1, Shoot = -1;
	};

	namespace TankConstants
	{
		inline const glm::vec3 COLOR_HEALTH_BAR_BACKGROUND = glm::vec3(0.361, 0.176, 0.008);
		inline const glm::vec3 COLOR_HEALTH_BAR_FOREGROUND = glm::vec3(0.039, 0.51, 0.016);
		inline const glm::vec3 COLOR_DEFAULT_MAIN = glm::vec3(0.79, 0.68, 0.54);
		inline const glm::vec3 COLOR_DEFAULT_SECONDARY = glm::vec3(0.45, 0.39, 0.3);
		inline const glm::vec3 COLOR_DEFAULT_GUN = glm::vec3(0.23, 0.23, 0.23);
		inline const glm::vec3 COLOR_RED_MAIN = glm::vec3(0.79, 0.3, 0.31);
		inline const glm::vec3 COLOR_RED_SECONDARY = glm::vec3(0.21, 0.08, 0.08);
		inline const glm::vec3 COLOR_BLUE_MAIN = glm::vec3(0.31, 0.30, 0.79);
		inline const glm::vec3 COLOR_BLUE_SECONDARY = glm::vec3(0.08, 0.08, 0.21);
		inline const float BODY_HEIGHT = 20.f;
		inline const float BODY_LENGTH = 80.f;
		inline const float SLOPE_WIDTH = BODY_HEIGHT / 1.5f;

		inline const float GUN_HEIGHT = BODY_HEIGHT / 2.5f;
		inline const float GUN_LENGTH = BODY_LENGTH / 1.75f;
		inline const float TURRET_RADIUS = BODY_LENGTH / 5.f;
	};

	struct Projectile
	{
		static inline const float INITIAL_PROJECTILE_SPEED = 400.f;
		static inline const float PROJECTILE_RADIUS = TankConstants::GUN_HEIGHT / 2.f - 1;
		static inline const size_t MAX_PROJECTILE_COUNT = 2;

		glm::vec2 PreviousPosition = { 0, 0 };
		glm::vec2 Position = { 0, 0 };
		glm::vec2 Speed = { INITIAL_PROJECTILE_SPEED, INITIAL_PROJECTILE_SPEED };
	};

	struct Healthbar
	{
		static inline const int MAX_HEALTH_DEFAULT = 3;

		Healthbar() = default;
		Healthbar(int MaxHealth) : MaxHealth(MaxHealth), Health(MaxHealth) {}
		int MaxHealth = MAX_HEALTH_DEFAULT;
		int Health = MAX_HEALTH_DEFAULT;
	};

	float clamp(float value, float min, float max);
	float clampMin(float value, float min);
	float clampMax(float value, float max);
	float clamp01(float x);
	float lerp(float a, float b, float t);
	glm::vec2 lerp(const glm::vec2& a, const glm::vec2& b, float t);
	glm::vec3 lerp(const glm::vec3& a, const glm::vec3& b, float t);
	int sign(float x);

	double RandomDouble(double min, double max);

	Mesh* CreateSquare(const std::string& name, glm::vec3 leftBottomCorner, float length, glm::vec3 color, bool fill = false);

	inline const float Gravity = 300;
	inline const TankControls s_ControlsLeft = TankControls(GLFW_KEY_A, GLFW_KEY_D, GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_SPACE);
	inline const TankControls s_ControlsRight = TankControls(GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_DOWN, GLFW_KEY_UP, GLFW_KEY_RIGHT_SHIFT, GLFW_KEY_ENTER);
} // namespace tw