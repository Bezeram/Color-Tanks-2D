#pragma once

#include <vector>
#include <string>
#include <iostream>

#include "core/gpu/mesh.h"
#include "components/simple_scene.h"
#include "lab_m1/TankWars/TerrainGenerator.h"
#include "lab_m1/TankWars/RenderData.h"
#include "lab_m1/TankWars/Utility.h"

namespace m1
{
	class Tank
	{
	public:
		struct UpdateStatus
		{
		};
		struct Hitbox
		{
			glm::vec2 Center = { 0, 0 };
			float Radius = 0;
		};
	public:
		Tank(std::string_view name, const tw::TankControls& controls, int health);
		Tank(std::string_view name, const tw::TankControls& controls, int health, float initialPositionX, const glm::vec3& bodyColor, const glm::vec3& tracksColor, const glm::vec3& gunColor);

		void Init(float initialPositionX, float initialGunRotation, glm::vec3 bodyColor, glm::vec3 tracksColor, const TerrainManager& terrain, const glm::ivec2& windowSize);
		void Move(float distance);
		UpdateStatus UpdateGameplay(float deltaTime, TerrainManager& terrain, const WindowObject* window, std::vector<tw::Projectile>& projectilesLeft, std::vector<tw::Projectile>& projectilesRight);
		void UpdateTitleScreen(float deltaTime, const TerrainManager& terrain, const WindowObject* window);
		
		tw::Projectile Shoot() const;
		void UpdateHeightAndRotation(const TerrainManager& terrain, const glm::ivec2& windowSize);
		void CheckProjectilesCollision(std::vector<tw::Projectile>& projectiles);
		bool IntersectsWithHitbox(const tw::Projectile& projectile, const Hitbox& hitbox);

		std::vector<tw::RenderData> RenderTank(float tInterpolation) const;
		std::vector<tw::RenderData> RenderHealthbar(float tInterpolation) const;
		std::vector<tw::RenderData> Render(float tInterpolation) const;
		
		bool IsAlive() const;

		void OnInputUpdateGameplay(float deltaTime, const WindowObject* window);
		bool OnKeyPress(int key, tw::Projectile& potentialProjectile);

		Mesh* GetBodyMesh() const;
		Mesh* GetGunMesh() const;

		const std::string& GetMeshBodyName() const;
		const std::string& GetMeshGunName() const;

		Hitbox GetHitbox(int idx) const;

		void SetRotation(float rotation);
		void SetGunRotation(float rotation);
		void ResetHealth(int health);

		const glm::vec2& GetTankPosition() const;
		const glm::vec2& GetPreviousPosition() const;
		float GetTankRotation() const;
		float GetPreviousTankRotation() const;
		static glm::vec2 GetTankOrigin();
		static glm::vec2 GetGunOrigin();
		glm::vec2 GetProjectileOrigin() const;
		glm::vec2 GetGunOriginRotation() const;
		glm::vec2 GetProjectileOriginRotation() const;
		float GetGunRotation() const;
		float GetPreviousGunRotation() const;
		glm::vec2 GetHealthbarOrigin() const;
		void SetPositionX(float position);

		int GetHealth() const;
		static Mesh* InitTankMesh(const std::string& nameMesh, glm::vec3 colorTracks, glm::vec3 colorBody);
		static Mesh* InitGunMesh(const std::string& nameMesh, glm::vec3 colorGun);
	public:
		const static inline std::string& s_HealthbarBackgroundMeshName = "healthbar_background";
		const static inline std::string& s_HealthbarForegroundMeshName = "healthbar_foreground";
	private:
		// Relative to an untransformed tank
		Hitbox m_HitboxRelative[3]{};
		tw::Healthbar m_Healthbar;
		int m_Health;
		int m_MaxHealth;

		tw::TankControls controls;

		// Transforms
		glm::vec2 m_PreviousTankPosition = { 0, 0 };
		glm::vec2 m_TankPosition = { 0, 0 };
		float m_PreviousTankRotation = 0;
		float m_TankRotation = 0;

		float m_PrevGunRotation = 0;
		float m_GunRotation = 0;
		float m_GunRotationSpeed = 1.5f;

		// The moment a gun rotation key is pressed, it takes note which direction it should rotate
		// It then keeps rotating in that same direction until the key is released
		float m_CurrentRotationDirection = 0;
		
		Mesh* m_BodyMesh = nullptr;
		Mesh* m_GunMesh = nullptr;
		const std::string m_TankMeshName = "tank";
		const std::string m_GunMeshName = "tank_gun";

		const float m_TankSpeed = 150;
		const float m_TankFakeGravityScalar = 80;

		glm::vec3 m_colorBody	= tw::TankConstants::COLOR_DEFAULT_MAIN;
		glm::vec3 m_colorTracks = tw::TankConstants::COLOR_DEFAULT_SECONDARY;
		glm::vec3 m_colorGun	= tw::TankConstants::COLOR_DEFAULT_GUN;
	};
}
