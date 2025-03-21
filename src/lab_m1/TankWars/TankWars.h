#pragma once

#include "components/simple_scene.h"
#include "lab_m1/TankWars/TerrainGenerator.h"
#include "lab_m1/TankWars/Tank.h"
#include "lab_m1/TankWars/Utility.h"

#include <cmath>
#include <ctime>
#include <random>
#include <memory>

namespace m1
{
    /**
	 * @brief TankWars class
	 *
	 * This class is the main class for the TankWars game.
	 * Acts as the game scene and handles all the game logic, including the rendering.
    */
    class TankWars : public gfxc::SimpleScene
    {
    public:
        TankWars();

        void Init() override;
		void InitBackgroundMesh();
        void InitTrajectoryPointMesh(const glm::vec3& color, const std::string& meshName);
        void InitProjectileMesh();
        void InitHealthBarMeshes(const glm::vec3& color, const std::string& meshName);
		void InitDeathTransitionMeshes();

		void RenderMesh(const tw::RenderData& data);
		void RenderMesh(const std::string& meshName, const glm::mat3& modelMatrix, const std::string& shaderType);
		void RenderMeshes(const std::vector<tw::RenderData>& data);
        void GenerateTerrainMesh() const;
		void RenderProjectileTrajectory(const Tank& tank, float deltaTime, float tInterpoaltion, const std::string& meshName);
        void ToggleInterpolation();
        void ToggleInterpolation(bool toggle);
    public:
        enum class GameState
        {
            TERRAIN_EDITOR,
            GAMEPLAY_HUMAN_VS_HUMAN,
        };
    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        // Main update functions
        void UpdateGameplay(float deltaTimeSeconds);
        void UpdateTerrainEditor(float deltaTimeSeconds);
        void UpdateAndRenderDeathTransition();

        void UpdateProjectiles(float deltaTime, std::vector<tw::Projectile>& projectiles);
        void CheckProjectilesTerrainCollision(std::vector<tw::Projectile>& projectiles);

		float GetInitialTankPosition(bool onTheLeft) const;

        void OnInputUpdateTerrainEditor(float deltaTimeSeconds);
        void OnInputUpdateGameplay(float deltaTimeSeconds);

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;
    private:
        float m_Accumulator = 0;
		float m_TPS = 20;
        GameState m_GameState = GameState::GAMEPLAY_HUMAN_VS_HUMAN;

        float m_ResolutionTerrain;
		Tank m_TankLeft;
		Tank m_TankRight;
        bool m_TankLeftHasDied = 0;
        bool m_TankRightHasDied = 0;
		int m_ProjectileCountMax = 5;
        TerrainManager m_TerrainManager;

        glm::vec3 m_BackgroundColorBottom = { 0.047, 0.757, 0.871 };
        glm::vec3 m_BackgroundColorTop = { 0, 0.616, 1 };

		float m_TimerDeathTransition = 0;
		float m_DelayDeathTransitionSeconds = 1.f;
        float m_DeathTransitionDurationSeconds = 5.f;
        bool m_DeathTransition = 0;
        bool m_TriggerRestart = 0;

        bool m_interpolation = 1;
        std::vector<tw::Projectile> m_ProjectilesTankLeft;
        std::vector<tw::Projectile> m_ProjectilesTankRight;

        const std::string& m_TerrainMeshName = "terrain";
        const std::string& m_BackgroundMeshName = "sky";
        const std::string& m_ProjectileMeshName = "projectile";
        const std::string& m_TrajectoryLeftMeshName = "projectile_left";
        const std::string& m_TrajectoryRightMeshName = "projectile_right";
        const std::string& m_BlackScreenMeshName = "black_screen_transition";
        const std::string& m_DeathTankMeshName = "death_tank_transition";
        const std::string& m_DeathGunMeshName = "death_gun_transition";
    private:
        class Sun
        {
        public:
            Sun(TankWars* drawRef, glm::ivec2 windowSize) 
                : m_DrawRef(drawRef)
				, m_DistanceBig(windowSize.x * 1.2)
				, m_DistanceSmall(windowSize.y / 1.2f)
                , m_Rotation(45)
				, m_RevolutionRotation(glm::vec2(windowSize.x / 2, -30))
            {}
            void UpdateAndRender(float deltaTimeSeconds);
            void InitSunMesh(Mesh*& sun, Mesh*& sunOutline);

        private:
            double m_SunOutlineCurrentRotation = 0;
            double m_SunCurrentRotation = 0;
            double m_Rotation;
            double m_Speed = 0.05F;
            double m_RotationSpeed = 0.4F;
            double m_DistanceBig;
            double m_DistanceSmall;
            glm::vec2 m_RevolutionRotation;

            const std::string& m_SunMeshName = "sun";
            const std::string& m_SunOutlineMeshName = "sun_outline";
			const float m_SunRadius = 45;
			const float m_SunOutlineRadius = 60;
			TankWars* const m_DrawRef = nullptr;
        } m_Sun;

        class Smoke
        {
        public:
            struct SmokeParticle
            {
                glm::vec2 Position = { 0, 0 };
                glm::vec2 Speed = { 0, 0 };
                float Lifetime = 0;
                float SizeScalar = 0;
                float GrowthSpeed = 0;
                float FadeSpeed = 0;
            };

            Smoke(TankWars* tankWarsRef) : m_TankWarsRef(tankWarsRef) {}
            void UpdateAndRender(float deltaTimeSeconds);
            void GenerateSmokeParticles(float deltaTimeSeconds, const glm::vec2& tankPosition);
            void InitSmokeMesh();
            void Clear();

        private:
            const int m_NumParticles = 50; // Number of smoke particles
            const float m_MinLifetime = 1.0f; // Minimum lifetime of a particle
            const float m_MaxLifetime = 3.0f; // Maximum lifetime of a particle
            const float m_MinSpeed = 10.0f; // Minimum velocity magnitude
            const float m_MaxSpeed = 50.0f; // Maximum velocity magnitude
            const float m_MinSize = 2.0f; 
            const float m_MaxSize = 8.0f; 
            const float m_MinGrowthSpeed = 2.0f;
            const float m_MaxGrowthSpeed = 5.0f;
            const float m_MinFadeSpeed = 0.5f; 
            const float m_MaxFadeSpeed = 1.5f;

            const std::string& m_SmokeMeshName = "smoke";
            TankWars* const m_TankWarsRef = nullptr;
            std::vector<SmokeParticle> m_SmokeParticles;
        } m_SmokeGenerator;

        void PrintStats();
    };
}   // namespace m1