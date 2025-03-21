#pragma once

#include "lab_m1/TankWars/Utility.h"

#include <vector>
#include <random>
#include <iostream>
#include <limits>
#include <cmath>
#include <algorithm>

namespace m1
{
	class TerrainManager
	{
	public:
		struct Wave
		{
			Wave(double offset, double amplitude, double scalar) : offset(offset), amplitude(amplitude), frequency(scalar) {}
			double offset;
			double amplitude;
			double frequency;
		};
		struct Stats
		{
			std::size_t Resolution;
			double FunctionStart, FunctionEnd;
			double Floor, HeightScalar;
			double RandomnessMargin;
			Wave BaseBump, BaseHill;
			double BumpsMax, HillsMax;
			std::vector<Wave> RandomBumps;
			std::vector<Wave> RandomHills;
		};
	public:
		TerrainManager(std::size_t resolution, const glm::vec2& windowSize);

		void CalculateHeightAndAngle(float positionX, float windowLength, float& outHeight, float& outAngle) const;
		float CalculateHeight(float positionX, float windowLength) const;
		void Explode(glm::vec2 center, float radius, float windowLength);
		void LevelTerrainRegion(float deltaTime, float windowLength);

		bool CollidesWithProjectile(const tw::Projectile projectile, float windowLength);

		double GetMinHeight() const;

		double GetHeight(size_t index) const;
		double GetHeight(float x, float windowLength) const;
		double GetMaxHeight() const;
		glm::vec2 GetPointOnScreen(float x, float windowLength) const;
		std::size_t GetResolution() const;
		std::size_t SizeHeightMap() const;
		std::size_t GetFunctionStart() const;
		std::size_t GetFunctionEnd() const;

		/**
		* Translate the a parameter of the terrain.
		* Will trigger a regeneration of the height map.
		*/
		void TranslateResolution(double value);
		void TranslateFunctionStart(double value);
		void TranslateFunctionEnd(double value);
		void TranslateFloor(double value);
		void TranslateHeightScalar(double value);
		void TranslateWavesOffset(double value);
		void TranslateBumpMaxCount(double value);
		void TranslateHillMaxCount(double value);
		void TranslateBumpAmplitude(double value);
		void TranslateHillAmplitude(double value);
		void TranslateBumpFrequency(double value);
		void TranslateHillFrequency(double value);
		void TranslateRandomness(double value);

		double GenerateRandomScalar() const;

		const std::vector<double>& GenerateRandomHeightMap();
		const std::vector<double>& RegenerateHeightMap();
		const std::vector<double>& RegenerateRandomHeightMap();
		const std::vector<double>& RegenerateHeightMap(std::size_t resolution);
		const std::vector<double>& RegenerateHeightMap(std::size_t resolution, double functionStart, double functionEnd);

		Stats GetStats() const;
	private:
		double m_Resolution;
		std::vector<double> m_HeightMap;
		
		double m_FunctionStart = 0.f;
		double m_FunctionEnd = 9.1f;
		double m_Floor = 230.8f;
		double m_HeightScalar = 125.4f;
		std::vector<Wave> m_RandomBumps;
		std::vector<Wave> m_RandomHills;
		double m_BumpsMax = 10;
		double m_HillsMax = 10;

		// The randomness margin dictates how much the scalar for a parameter is influenced by the randomness
		// For example a margin of 0.2 mean the scalar can vary from 0.8 to 1.2 of the base value
		double m_RandomnessMargin = 0.2f;

		// From these, the randomness will scale up or down the generated waves
		Wave m_BaseBump = Wave(0.5, 0.1, 2.3);
		Wave m_BaseHill = Wave(0.5, 0.1, 0.95);
		
		double m_MaxHeight = -1.f;
		double m_MinHeight = std::numeric_limits<double>::max();

		float m_LandslideDepth = 40.f;
		float m_LandslideMaximumThreshold = 50.f;
		float m_LandslideSpeed = 0.2f;
		float m_LandslideMinimumThreshold = 1.f;

		std::vector<Wave> m_GenerateRandomWaves(size_t count, const Wave& baseWave) const;
		double m_GenerateBumpsCount() const;
		double m_GenerateHillsCount() const;
		std::vector<double>& m_GeneratePoints();
	};
}
