#include "lab_m1/TankWars/TerrainGenerator.h"

using namespace std;
using namespace m1;

TerrainManager::TerrainManager(size_t resolution, const glm::vec2& windowSize)
	: m_Resolution(resolution)
{
}

/**
* Regenerate height map with the current local settings.
* Does NOT recalculate random values, such as wave offsets.
*/
const std::vector<double>& m1::TerrainManager::RegenerateHeightMap()
{
	return m_GeneratePoints();
}

const std::vector<double>& m1::TerrainManager::RegenerateRandomHeightMap()
{
	// Only recalculate the random waves
	m_RandomBumps = m_GenerateRandomWaves(m_RandomBumps.size(), m_BaseBump);
	m_RandomHills = m_GenerateRandomWaves(m_RandomHills.size(), m_BaseHill);

	return m_GeneratePoints();
}

const std::vector<double>& TerrainManager::RegenerateHeightMap(std::size_t resolution)
{
	m_Resolution = resolution;
	return m_GeneratePoints();
}

const std::vector<double>& TerrainManager::RegenerateHeightMap(std::size_t resolution, double functionStart, double functionEnd)
{
	m_Resolution = resolution;
	m_FunctionStart = functionStart;
	m_FunctionEnd = functionEnd;
	return m_GeneratePoints();
}

/**
* Implicitly, the height map will be generated randomly.
* Generate random amount of bumps and hills.
* Bumps compared to hills:
* - more common (higher frequency and number of waves)
* - smaller amplitude
* - higher frequency per wave
*/
const vector<double>& TerrainManager::GenerateRandomHeightMap()
{
	// Bumps are more likely to have an average count, whilst hills are rarely plentiful
	// Harcoded to have at least 2 bumps and 2 hills
	size_t bumpsCount = m_GenerateBumpsCount();
	size_t hillsCount = m_GenerateHillsCount();
	// Values are sorted by bumps and then hills
	m_RandomBumps = m_GenerateRandomWaves(bumpsCount, m_BaseBump);
	m_RandomHills = m_GenerateRandomWaves(hillsCount, m_BaseHill);

	return m_GeneratePoints();
}

/**
* Calculate the height and angle of the terrain at a given x position, where the position is in between 0 and 1
*/
void TerrainManager::CalculateHeightAndAngle(float positionX, float windowLength, float& outHeight, float& outAngle) const
{
	float positionX01 = tw::clamp(positionX / windowLength, 0, 1); // clamp position within terrain size

	size_t maxIndexHeightMap = SizeHeightMap() - 1;
	float tIndex = positionX01 * maxIndexHeightMap; // (between 0 and maxIndexHeightMap)
	size_t indexLeft = tw::clamp((size_t)tIndex, 0, maxIndexHeightMap - 1);
	size_t indexRight = indexLeft + 1;
	// Calculate the real x values
	float xLeft01 = indexLeft / (float)maxIndexHeightMap;
	float xRight01 = indexRight / (float)maxIndexHeightMap;
	float xLeft = xLeft01 * windowLength;
	float xRight = xRight01 * windowLength;

	// Calculate height
	float yLeft = GetHeight(indexLeft);
	float yRight = GetHeight(indexRight);

	float yInterpolate = (positionX01 - xLeft01) / (xRight01 - xLeft01);
	// Output
	outHeight = yLeft * (1 - yInterpolate) + yRight * yInterpolate;
	outAngle = atan2(yRight - yLeft, xRight - xLeft);

	// Convert radiands to degrees
	float outAngleDegrees = glm::degrees(outAngle);
}

float TerrainManager::CalculateHeight(float positionX, float windowLength) const
{
	float positionX01 = tw::clamp(positionX / windowLength, 0, 1); // clamp position within terrain size

	size_t maxIndexHeightMap = SizeHeightMap() - 1;
	float tIndex = positionX01 * maxIndexHeightMap; // (between 0 and maxIndexHeightMap)
	size_t indexLeft = tw::clamp((size_t)tIndex, 0, maxIndexHeightMap - 1);
	size_t indexRight = indexLeft + 1;
	// Calculate the real x values
	float xLeft01 = indexLeft / (float)maxIndexHeightMap;
	float xRight01 = indexRight / (float)maxIndexHeightMap;
	float xLeft = xLeft01 * windowLength;
	float xRight = xRight01 * windowLength;

	// Calculate height
	float yLeft = GetHeight(indexLeft);
	float yRight = GetHeight(indexRight);

	float yInterpolate = (positionX01 - xLeft01) / (xRight01 - xLeft01);
	// Output
	return lerp(yLeft, yRight, yInterpolate);
}

void TerrainManager::Explode(glm::vec2 center, float radius, float windowLength)
{
	float positionCenterX01 = tw::clamp(center.x / windowLength, 0, 1); // clamp position within terrain size
	float positionLeftX01 = tw::clamp((center.x - radius) / windowLength, 0, 1); // clamp position within terrain size
	float positionRightX01 = tw::clamp((center.x + radius) / windowLength, 0, 1); // clamp position within terrain size

	size_t maxIndexHeightMap = SizeHeightMap() - 1;
	float tIndexCenter = positionCenterX01 * maxIndexHeightMap; // (between 0 and maxIndexHeightMap)
	float tIndexLeft = positionLeftX01 * maxIndexHeightMap; // (between 0 and maxIndexHeightMap)
	float tIndexRight = positionRightX01 * maxIndexHeightMap; // (between 0 and maxIndexHeightMap)
	size_t indexLeft = (size_t)tIndexLeft;
	size_t indexRight = (size_t)tIndexRight;
	// Calculate the real x values
	float xLeft01 = indexLeft / (float)maxIndexHeightMap;
	float xRight01 = indexRight / (float)maxIndexHeightMap;
	float xLeft = xLeft01 * windowLength;
	float xRight = xRight01 * windowLength;

	for (size_t i = indexLeft; i <= indexRight; i++)
	{
		float x = i / (float)maxIndexHeightMap * windowLength - center.x;
		if (radius - x < 0 || radius + x < 0) 
			continue; // Skip if outside of the circle
		float newY = center.y + 0.5 * -sqrtf((radius - x) * (radius + x));

		// Update height map
		if (newY < m_HeightMap[i])
			m_HeightMap[i] = newY;
	}
}

void TerrainManager::LevelTerrainRegion(float deltaTime, float windowLength)
{
	float mapScale = SizeHeightMap() / windowLength;
	int mapDx = m_LandslideDepth * mapScale;

	float extremeLeftX = 0;
	float extremeRightX = 0;

	auto findLocalExtreme = [&](int idxMap, bool left) -> float
	{
		int j;
		int signTrend = 0;
		int side = left ? -1 : 1;
		for (j = idxMap; j > 0 && j < SizeHeightMap() - 1 && j >= idxMap - mapDx && j <= idxMap + mapDx; j += side)
		{
			signTrend = tw::sign(m_HeightMap[j] - m_HeightMap[j + side]);
			if (signTrend != 0)
				break;
		}
		if (signTrend != 0)
		{
			// Find the local ceiling or minima
			while (j > 0 && j < SizeHeightMap() - 1 && j >= idxMap - mapDx && j <= idxMap + mapDx)
			{
				int sign = tw::sign(m_HeightMap[j] - m_HeightMap[j + side]);

				// When the trend is broken, stop advancing
				if (sign * signTrend < 0)
					break;
				j += side;
			}

			if (left)
				extremeLeftX = j;
			else
				extremeRightX = j;
			return m_HeightMap[j];
		}

		return m_HeightMap[idxMap];
	};

	for (int i = 0; i < SizeHeightMap(); i++)
	{
		// Given the trend on the terrain of the left and right, 
		// find either the local ceiling or minima
		float leftExtreme = findLocalExtreme(i, true);
		float rightExtreme = findLocalExtreme(i, false);
		float displacement = (leftExtreme - m_HeightMap[i] + rightExtreme - m_HeightMap[i]) / 2.f;

		if (abs(displacement) > m_LandslideMinimumThreshold)
		{
			m_HeightMap[i] += displacement * m_LandslideSpeed * deltaTime;
		}

		if (i != 0 && i != SizeHeightMap() - 1 && m_HeightMap[i - 1] < m_HeightMap[i] && m_HeightMap[i] > m_HeightMap[i + 1])
		{
			m_HeightMap[i] += m_LandslideSpeed * deltaTime;
		}
	}
}

bool TerrainManager::CollidesWithProjectile(const tw::Projectile projectile, float windowLength)
{
	if (projectile.Position.y < CalculateHeight(projectile.Position.x, windowLength))
	{
		Explode(projectile.Position, tw::TankConstants::BODY_LENGTH / 2.f, windowLength);
		return 1;
	}

	return 0;
}

double TerrainManager::GetMinHeight() const
{
	return m_MinHeight;
}

double TerrainManager::GetHeight(size_t index) const
{
	assert(index >= 0 && index < m_HeightMap.size());
	return m_HeightMap[index];
}

double TerrainManager::GetHeight(float x, float windowLength) const
{
	size_t tIndex = x / windowLength * (SizeHeightMap() - 1);
	return GetHeight(tIndex);
}

double TerrainManager::GetMaxHeight() const
{
	return m_MaxHeight;
}

glm::vec2 TerrainManager::GetPointOnScreen(float x, float windowLength) const
{
	int mapX = (float)(x / windowLength);
	float mapXOnScreen = mapX * windowLength;
	return { mapXOnScreen, GetHeight(x, windowLength) };
}

size_t TerrainManager::GetResolution() const
{
	return m_Resolution;
}

std::size_t m1::TerrainManager::SizeHeightMap() const
{
	return m_Resolution + 2;
}

size_t TerrainManager::GetFunctionStart() const
{
	return m_FunctionStart;
}

std::size_t TerrainManager::GetFunctionEnd() const
{
	return m_FunctionEnd;
}

void TerrainManager::TranslateResolution(double value)
{
	m_Resolution = tw::clampMin(m_Resolution + value, 0);
	RegenerateHeightMap();
}

void TerrainManager::TranslateFunctionStart(double value)
{
	m_FunctionStart += value;
	RegenerateHeightMap();
}

void TerrainManager::TranslateFunctionEnd(double value)
{
	m_FunctionEnd += value;
	RegenerateHeightMap();
}

void TerrainManager::TranslateFloor(double value)
{
	m_Floor += value;
	RegenerateHeightMap();
}

void TerrainManager::TranslateHeightScalar(double value)
{
	m_HeightScalar += value;
	RegenerateHeightMap();
}

void TerrainManager::TranslateWavesOffset(double value)
{
	m_BaseBump.offset += value;
	m_BaseHill.offset += value;
	RegenerateHeightMap();
}

void TerrainManager::TranslateBumpMaxCount(double value)
{
	m_BumpsMax = tw::clampMin(m_BumpsMax + value, 3);
	RegenerateHeightMap();
}

void TerrainManager::TranslateHillMaxCount(double value)
{
	m_HillsMax = tw::clampMin(m_HillsMax + value, 3);
	RegenerateHeightMap();
}

void TerrainManager::TranslateBumpAmplitude(double value)
{
	m_BaseBump.amplitude += value;
	RegenerateHeightMap();
}

void TerrainManager::TranslateHillAmplitude(double value)
{
	m_BaseHill.amplitude += value;
	RegenerateHeightMap();
}

void TerrainManager::TranslateBumpFrequency(double value)
{
	m_BaseBump.frequency += value;
	RegenerateHeightMap();
}

void TerrainManager::TranslateHillFrequency(double value)
{
	m_BaseHill.frequency += value;
	RegenerateHeightMap();
}

void TerrainManager::TranslateRandomness(double value)
{
	m_RandomnessMargin += value;
	RegenerateRandomHeightMap();
}

double TerrainManager::GenerateRandomScalar() const
{
	return tw::RandomDouble(1 - m_RandomnessMargin, 1 + m_RandomnessMargin);
}

TerrainManager::Stats TerrainManager::GetStats() const
{
	return
	{
		(size_t)m_Resolution,
		m_FunctionStart, m_FunctionEnd,
		m_Floor, m_HeightScalar,
		m_RandomnessMargin,
		m_BaseBump, m_BaseHill,
		m_BumpsMax, m_HillsMax,
		m_RandomBumps, m_RandomHills
	};
}

double TerrainManager::m_GenerateBumpsCount() const
{
	const double minimumBumps = 2;
	return minimumBumps + tw::RandomDouble(0.f, 1.f) * m_BumpsMax;
}

double TerrainManager::m_GenerateHillsCount() const 
{
	const double minimumHills = 2;
	return minimumHills + tw::RandomDouble(0.f, 1.f) * m_HillsMax;
}

vector<TerrainManager::Wave> TerrainManager::m_GenerateRandomWaves(size_t count, const TerrainManager::Wave& baseWave) const
{
	vector<TerrainManager::Wave> waves;
	waves.reserve(count);

	for (int i = 0; i < count; i++)
	{
		double offset = GenerateRandomScalar();
		double amplitude = GenerateRandomScalar();
		double frequency = GenerateRandomScalar();
		waves.emplace_back(offset, amplitude, frequency);
	}

	// Calculate for bumps and hills
	return waves;
}

std::vector<double>& TerrainManager::m_GeneratePoints()
{
	// Generate points in the function interval scaled with the resolution
	m_HeightMap.resize(SizeHeightMap());

	m_MaxHeight = 0;
	m_MinHeight = numeric_limits<double>::max();
	for (int i = 0; i < SizeHeightMap(); i++)
	{
		// Generate x coordinate
		double t = i / (double)(SizeHeightMap() - 1); // Normalise point
		double x = tw::lerp(m_FunctionStart, m_FunctionEnd, t); // Interpolate for function abscissa
		
		// Calculate y coordinate (height)
		double y = 0; 
		for (const Wave& randomBump : m_RandomBumps) // Apply bump sines
		{
			float amplitude = randomBump.amplitude * m_BaseBump.amplitude;
			float frequency = randomBump.frequency * m_BaseBump.frequency;
			float offset = randomBump.offset * m_BaseBump.offset;
			y += amplitude * sinf(frequency * (x + offset));
		}
		for (const Wave& randomHill : m_RandomHills) // Apply hill sines
		{
			float amplitude = randomHill.amplitude * m_BaseHill.amplitude;
			float frequency = randomHill.frequency * m_BaseHill.frequency;
			float offset = randomHill.offset * m_BaseHill.offset;
			y += amplitude * sinf(frequency * (x + offset));
		}

		y = y * m_HeightScalar + m_Floor;
		m_HeightMap[i] = y;

		m_MaxHeight = max(m_MaxHeight, y);
		m_MinHeight = min(m_MinHeight, y);
	}

	return m_HeightMap;
}