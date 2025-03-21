#pragma once

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
