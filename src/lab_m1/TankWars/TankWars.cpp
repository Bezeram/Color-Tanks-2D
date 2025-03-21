#include "lab_m1/TankWars/TankWars.h"

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace m1;

/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */
TankWars::TankWars() 
    : m_ResolutionTerrain(window->GetResolution().x / 2)
    , m_TerrainManager((size_t)m_ResolutionTerrain, window->GetResolution())
    , m_TankLeft("tank_left", tw::s_ControlsLeft, tw::Healthbar::MAX_HEALTH_DEFAULT)
    , m_TankRight("tank_right", tw::s_ControlsRight, tw::Healthbar::MAX_HEALTH_DEFAULT)
    , m_Sun(this, window->GetResolution())
	, m_SmokeGenerator(this)
{}

void TankWars::Init()
{
    glm::ivec2 resolution = window->GetResolution();
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);

    // Sun
    {
        Mesh* sun;
        Mesh* sunOutline;
	    m_Sun.InitSunMesh(sun, sunOutline);
        AddMeshToList(sun);
        AddMeshToList(sunOutline);
    }
    
    // Tanks
	{
		float tankPositionLeft = GetInitialTankPosition(true);
		float tankPositionRight = GetInitialTankPosition(false);
        m_TankLeft.Init(tankPositionLeft, M_PI / 6, tw::TankConstants::COLOR_RED_MAIN, tw::TankConstants::COLOR_RED_SECONDARY, m_TerrainManager, window->GetResolution());
        m_TankRight.Init(tankPositionRight, M_PI - M_PI / 6, tw::TankConstants::COLOR_BLUE_MAIN, tw::TankConstants::COLOR_BLUE_SECONDARY, m_TerrainManager, window->GetResolution());

		Mesh* tankLeft = m_TankLeft.GetBodyMesh();
		Mesh* tankRight = m_TankRight.GetBodyMesh();
		Mesh* tankGunLeft = m_TankLeft.GetGunMesh();
		Mesh* tankGunRight = m_TankRight.GetGunMesh();
		AddMeshToList(tankLeft);
		AddMeshToList(tankRight);
		AddMeshToList(tankGunLeft);
		AddMeshToList(tankGunRight);
	}

    Mesh* terrain = new Mesh(m_TerrainMeshName);
    AddMeshToList(terrain);
    m_TerrainManager.GenerateRandomHeightMap();
    GenerateTerrainMesh();

    m_SmokeGenerator.InitSmokeMesh();

    InitBackgroundMesh();

	InitDeathTransitionMeshes();

	InitTrajectoryPointMesh(tw::TankConstants::COLOR_RED_MAIN, m_TrajectoryLeftMeshName);
	InitTrajectoryPointMesh(tw::TankConstants::COLOR_BLUE_MAIN, m_TrajectoryRightMeshName);

	InitHealthBarMeshes(tw::TankConstants::COLOR_HEALTH_BAR_BACKGROUND, Tank::s_HealthbarBackgroundMeshName);
	InitHealthBarMeshes(tw::TankConstants::COLOR_HEALTH_BAR_FOREGROUND, Tank::s_HealthbarForegroundMeshName);
    
    InitProjectileMesh();
}

void TankWars::InitBackgroundMesh()
{
	glm::vec2 resolution = window->GetResolution();

    vector<VertexFormat> vertices
    {
        VertexFormat(glm::vec3(0, 0, 0), m_BackgroundColorBottom),
        VertexFormat(glm::vec3(resolution.x, 0, 0), m_BackgroundColorBottom),
        VertexFormat(glm::vec3(resolution.x, resolution.y, 0), m_BackgroundColorTop),
        VertexFormat(glm::vec3(0, resolution.y, 0), m_BackgroundColorTop),
    };

    vector<unsigned int> indices =
    {
        0, 1, 2,
        0, 2, 3
    };

    Mesh* background = new Mesh(m_BackgroundMeshName);
    background->InitFromData(vertices, indices);
    AddMeshToList(background);
}

void TankWars::InitTrajectoryPointMesh(const glm::vec3& color, const std::string& meshName)
{
    const size_t pointsCircle = 40;
    const float radius = tw::Projectile::PROJECTILE_RADIUS * 1.5f;
    vector<VertexFormat> vertices;
    vertices.reserve(pointsCircle + 1);
    // Center of the circle
    vertices.push_back(VertexFormat(glm::vec3(0, 0, 0), color));
    // Rest of the points
    for (int i = 0; i < pointsCircle; i++) {
        float angle = 2.0f * M_PI * float(i) / float(pointsCircle - 1); // Calculate the angle for this segment
        float x = radius * cos(angle); // Calculate x component
        float y = radius * sin(angle); // Calculate y component
        vertices.push_back(VertexFormat(glm::vec3(x, y, 0), color));
    }

    vector<unsigned int> indices;
    indices.reserve(pointsCircle + 1);
    for (int i = 0; i < pointsCircle + 1; i++)
        indices.push_back(i);

    Mesh* projectileMesh = new Mesh(meshName);
    projectileMesh->SetDrawMode(GL_TRIANGLE_FAN);
    projectileMesh->InitFromData(vertices, indices);
    AddMeshToList(projectileMesh);
}

void TankWars::InitProjectileMesh()
{
    const size_t pointsCircle = 20;
    const float radius = tw::Projectile::PROJECTILE_RADIUS;
    vector<VertexFormat> vertices;
    vertices.reserve(pointsCircle + 1);
    // Center of the circle
    vertices.push_back(VertexFormat(glm::vec3(0, 0, 0), tw::TankConstants::COLOR_DEFAULT_GUN));
    // Rest of the points
    for (int i = 0; i < pointsCircle; i++) {
        float angle = 2.0f * M_PI * float(i) / float(pointsCircle - 1); // Calculate the angle for this segment
        float x = radius * cos(angle); // Calculate x component
        float y = radius * sin(angle); // Calculate y component
        vertices.push_back(VertexFormat(glm::vec3(x, y, 0), tw::TankConstants::COLOR_DEFAULT_GUN));
    }

    vector<unsigned int> indices;
    indices.reserve(pointsCircle + 1);
    for (int i = 0; i < pointsCircle + 1; i++)
        indices.push_back(i);

    Mesh* projectileMesh = new Mesh(m_ProjectileMeshName);
    projectileMesh->SetDrawMode(GL_TRIANGLE_FAN);
    projectileMesh->InitFromData(vertices, indices);
    AddMeshToList(projectileMesh);
}

void TankWars::InitHealthBarMeshes(const glm::vec3& color, const std::string& meshName)
{
    vector<VertexFormat> vertices
    {
        VertexFormat(glm::vec3(0, 0, 0), color),
        VertexFormat(glm::vec3(tw::TankConstants::BODY_LENGTH / 1.5f, 0, 0), color),
        VertexFormat(glm::vec3(tw::TankConstants::BODY_LENGTH / 1.5f, 10.f, 0), color),
        VertexFormat(glm::vec3(0, 10, 0), color),
    };

    vector<unsigned int> indices =
    {
        0, 1, 2,
        0, 2, 3
    };

    Mesh* healthBarBackground = new Mesh(meshName);
    healthBarBackground->InitFromData(vertices, indices);
    AddMeshToList(healthBarBackground);
}

void TankWars::InitDeathTransitionMeshes()
{
	const glm::ivec2& resolution = window->GetResolution();

    Mesh* tankDeathTransition = Tank::InitTankMesh(m_DeathTankMeshName, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
    Mesh* gunDeathTransition = Tank::InitGunMesh(m_DeathGunMeshName, glm::vec3(0, 0, 0));
    AddMeshToList(tankDeathTransition);
    AddMeshToList(gunDeathTransition);

    vector<VertexFormat> vertices
    {
        VertexFormat(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0)),
        VertexFormat(glm::vec3(resolution.x, 0, 0), glm::vec3(0, 0, 0)),
        VertexFormat(glm::vec3(resolution.x, resolution.y, 0), glm::vec3(0, 0, 0)),
        VertexFormat(glm::vec3(0, resolution.y, 0), glm::vec3(0, 0, 0)),
    };

    vector<unsigned int> indices =
    {
        0, 1, 2,
        0, 2, 3
    };

    Mesh* deathBackground = new Mesh(m_BlackScreenMeshName);
    deathBackground->InitFromData(vertices, indices);
    AddMeshToList(deathBackground);
}

void TankWars::RenderMesh(const tw::RenderData& data)
{
    RenderMesh2D(meshes[data.meshName], shaders[data.shaderType], data.modelMatrix);
}

void TankWars::RenderMesh(const std::string& meshName, const glm::mat3& modelMatrix = glm::mat3(1), const std::string& shaderType = "VertexColor")
{
    RenderMesh2D(meshes[meshName], shaders[shaderType], modelMatrix);
}

void TankWars::RenderMeshes(const std::vector<tw::RenderData>& data)
{
	for (const tw::RenderData& renderData : data)
		RenderMesh(renderData);
}

void TankWars::GenerateTerrainMesh() const
{
    const auto& terrain = m_TerrainManager;
	const auto& windowSize = window->GetResolution();

    size_t resolution = terrain.GetResolution();
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;
    vertices.reserve(resolution + 2);
    indices.reserve((resolution + 2) * 3);

    // Scale points on the Oy axis based on the vertical resolution
    double maxHeightMap = terrain.GetMaxHeight();
    const glm::vec3& colorGrassDark = { 0.055, 0.349, 0.024 };
    const glm::vec3& colorGrassLight = { 0.161, 1, 0.357 };
    const glm::vec3& colorGround = { 0.231, 0.188, 0.008 };

    if (resolution != 0)
    {
        // Add points, but no triangle yet
        vertices.emplace_back(glm::vec3(0, terrain.GetHeight(0), 0), colorGrassLight); // Add noise to the colors
        vertices.emplace_back(glm::vec3(0, 0, 0), colorGround); // Support point at the bottom of the screen

        for (size_t i = 1; i <= resolution + 1; i++)
        {
            // Place them on screen
            double tx = i / (double)(resolution + 1);
            glm::vec2 renderPosition = { tx * windowSize.x, terrain.GetHeight(i) };

#if 0
            float angleTerrain = atan2(terrain.GetHeight(i) - terrain.GetHeight(i - 1), renderPosition.x - vertices[2 * (i - 1)].position.x);
            float angleTerrainClamped = tw::clamp(abs(angleTerrain + 0.2), 0, 0.9);
#endif // 0
            // TODO: shade based on the height difference on both the left and right
			float tHeightLeft = (terrain.GetHeight(i) - terrain.GetHeight(i - 1)) / terrain.GetMaxHeight();
            float tHeightRight = tHeightLeft;
            if (i < resolution + 1)
                tHeightRight = (terrain.GetHeight(i + 1) - terrain.GetHeight(i)) / terrain.GetMaxHeight();

            glm::vec3 colorGrass = tw::lerp(colorGrassLight, colorGrassDark, (tHeightLeft + tHeightRight) / 2.f);
            vertices.emplace_back(glm::vec3(renderPosition.x, renderPosition.y, 0), colorGrass);
            vertices.emplace_back(glm::vec3(renderPosition.x, 0, 0), colorGround); // Support point at the bottom of the screen

            // Connect the previous three points
            size_t indicesIdx = 2 * i;
            indices.emplace_back(indicesIdx - 2);
            indices.emplace_back(indicesIdx - 1);
            indices.emplace_back(indicesIdx);
            indices.emplace_back(indicesIdx - 1);
            indices.emplace_back(indicesIdx);
            indices.emplace_back(indicesIdx + 1);
        }
    }
    else
    {
        // Only two triangles exist in the mesh
        vertices.emplace_back(glm::vec3(0, terrain.GetHeight(0), 0), colorGrassLight);
        vertices.emplace_back(glm::vec3(0, 0, 0), colorGround);
        vertices.emplace_back(glm::vec3(windowSize.x, terrain.GetHeight(1), 0), colorGrassLight);
        vertices.emplace_back(glm::vec3(windowSize.x, 0, 0), colorGround);

        indices.emplace_back(0);
        indices.emplace_back(1);
        indices.emplace_back(2);
        indices.emplace_back(1);
        indices.emplace_back(2);
        indices.emplace_back(3);
    }

    // Reload mesh
	auto terrainMesh = meshes.at(m_TerrainMeshName);
    terrainMesh->ClearData();
    terrainMesh->InitFromData(vertices, indices);
}

void TankWars::RenderProjectileTrajectory(const Tank& tank, float deltaTime, float tInterpolation, const std::string& meshName)
{
    // The real rotation is also dependent on the tank rotation, 
    // so we want to subtract it to make the gun rotation independent
    float prevRealGunRotation = tank.GetPreviousGunRotation() - tank.GetPreviousTankRotation();
    float realGunRotation = tank.GetGunRotation() - tank.GetTankRotation();

	// Interpolate between the previous state and the current state
    glm::vec2 renderPosition = tw::lerp(tank.GetPreviousPosition(), tank.GetTankPosition(), tInterpolation);
    float renderTankAngle = tw::lerp(tank.GetPreviousTankRotation(), tank.GetTankRotation(), tInterpolation);
    float renderGunAngle = prevRealGunRotation * (1 - tInterpolation) + realGunRotation * tInterpolation;
    renderGunAngle = tw::clamp(renderGunAngle, 0, M_PI); // clamp gun rotation to not clip with the body (the gun is already rendered relative to the tank)

    // Calculate the position of the projectile similar to the gun
    // We use a different origin which extends to the end of the gun when rotating according to the tank rotation
    glm::mat3 transformMatrixProjectile = glm::mat3(1) *
        transform2D::Translate(renderPosition) *
        transform2D::Rotate(renderTankAngle) *
        transform2D::Translate(tank.GetProjectileOrigin()) *
        transform2D::Translate(tank.GetProjectileOriginRotation()) *
        transform2D::Rotate(renderGunAngle) *
        transform2D::Translate(-tank.GetProjectileOriginRotation());

    tw::Projectile projectile;
    // Get the translation from the matrix
    projectile.Position = { transformMatrixProjectile[2][0], transformMatrixProjectile[2][1] };
    projectile.PreviousPosition = projectile.Position;
    projectile.Speed = glm::vec2(cos(tank.GetGunRotation()), sin(tank.GetGunRotation())) * tw::Projectile::INITIAL_PROJECTILE_SPEED;

	// Simulate projectile trajectory
	const int steps = 20;
	for (int i = 0; i < steps; i++)
	{
		projectile.Speed.y -= tw::Gravity * deltaTime;
		projectile.Position += projectile.Speed * deltaTime;

		// Check if the projectile is out of bounds (stop the rendering)
		if (projectile.Position.x < 0 || projectile.Position.x > window->GetResolution().x)
			break;

		RenderMesh(meshName, transform2D::Translate(projectile.Position));
	}
}

void TankWars::ToggleInterpolation()
{
    m_interpolation = !m_interpolation;

    std::cout << "Toggled interpolation to " << ((m_interpolation) ? std::string("true") : std::string("false")) << endl;
}

void TankWars::ToggleInterpolation(bool toggle)
{
    m_interpolation = toggle;
}

void TankWars::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}

void TankWars::Update(float deltaTimeSeconds)
{
    switch (m_GameState)
    {
	case GameState::TERRAIN_EDITOR:
		UpdateTerrainEditor(deltaTimeSeconds);
		break;
	case GameState::GAMEPLAY_HUMAN_VS_HUMAN:
		UpdateGameplay(deltaTimeSeconds);
		break;
    }
}

void TankWars::UpdateGameplay(float deltaTimeSeconds)
{
    glLineWidth(1);
    glPointSize(1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);

	OnInputUpdateGameplay(deltaTimeSeconds);

    m_Accumulator += deltaTimeSeconds;

    float fixedDeltaTime = 1.f / m_TPS;
    while (m_Accumulator > fixedDeltaTime)
    {
        // Update game logic
		UpdateProjectiles(fixedDeltaTime, m_ProjectilesTankLeft);
		UpdateProjectiles(fixedDeltaTime, m_ProjectilesTankRight);
        
        Tank::UpdateStatus updateStatusLeft = m_TankLeft.UpdateGameplay(fixedDeltaTime, m_TerrainManager, window, m_ProjectilesTankLeft, m_ProjectilesTankRight);
        Tank::UpdateStatus updateStatusRight = m_TankRight.UpdateGameplay(fixedDeltaTime, m_TerrainManager, window, m_ProjectilesTankLeft, m_ProjectilesTankRight);
        if (!m_TankLeft.IsAlive() || !m_TankRight.IsAlive())
        {
            // Start the death transition
            m_DeathTransition = 1;
        }
        if (!m_TankLeft.IsAlive() && !m_TankLeftHasDied)
        {
			m_TankLeftHasDied = 1;
			m_SmokeGenerator.GenerateSmokeParticles(fixedDeltaTime, m_TankLeft.GetTankPosition());
        }
        if (!m_TankRight.IsAlive() && !m_TankRightHasDied)
        {
            m_TankRightHasDied = 1;
            m_SmokeGenerator.GenerateSmokeParticles(fixedDeltaTime, m_TankRight.GetTankPosition());
        }

        CheckProjectilesTerrainCollision(m_ProjectilesTankLeft);
        CheckProjectilesTerrainCollision(m_ProjectilesTankRight);

        m_TerrainManager.LevelTerrainRegion(fixedDeltaTime, window->GetResolution().x);
        m_Accumulator -= fixedDeltaTime;
    }
    if (m_DeathTransition)
    {
        m_TimerDeathTransition += deltaTimeSeconds;
		UpdateAndRenderDeathTransition();
    }

    float tInterpolation = m_Accumulator / fixedDeltaTime;
    if (!m_interpolation)
        tInterpolation = 1; // If interpolation is not on, set interpolation to 1 (the current position)

	m_SmokeGenerator.UpdateAndRender(deltaTimeSeconds);

    GenerateTerrainMesh();
    RenderMesh(m_TerrainMeshName);

    m_Sun.UpdateAndRender(deltaTimeSeconds);

    // Trajectories
    if (m_TankLeft.IsAlive())
    {
	    RenderProjectileTrajectory(m_TankLeft, fixedDeltaTime, tInterpolation, m_TrajectoryLeftMeshName);
        RenderMeshes(m_TankLeft.Render(tInterpolation));
    }
    if (m_TankRight.IsAlive())
    {
	    RenderProjectileTrajectory(m_TankRight, fixedDeltaTime, tInterpolation, m_TrajectoryRightMeshName);
        RenderMeshes(m_TankRight.Render(tInterpolation));
    }

    // Projectiles
    {
        std::vector<tw::RenderData> renderData;
        for (auto& p : m_ProjectilesTankLeft)
        {
            glm::vec2 renderPosition = tw::lerp(p.PreviousPosition, p.Position, tInterpolation);
            renderData.emplace_back(m_ProjectileMeshName.c_str(), "VertexColor", transform2D::Translate(renderPosition));
        }
        for (auto& p : m_ProjectilesTankRight)
        {
            glm::vec2 renderPosition = tw::lerp(p.PreviousPosition, p.Position, tInterpolation);
            renderData.emplace_back(m_ProjectileMeshName.c_str(), "VertexColor", transform2D::Translate(renderPosition));
        }

		RenderMeshes(renderData);
    }

    RenderMesh(m_BackgroundMeshName);
}

void TankWars::UpdateTerrainEditor(float deltaTimeSeconds)
{
    glLineWidth(1);
    glPointSize(1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);

    OnInputUpdateTerrainEditor(deltaTimeSeconds);

    m_Accumulator += deltaTimeSeconds;
    float fixedDeltaTime = 1.f / m_TPS;
    while (m_Accumulator > fixedDeltaTime)
    {
        // Update title screen
		m_TankLeft.UpdateTitleScreen(fixedDeltaTime, m_TerrainManager, window);
		m_TankRight.UpdateTitleScreen(fixedDeltaTime, m_TerrainManager, window);
        m_Accumulator -= fixedDeltaTime;
    }
    float tInterpolation = m_Accumulator / fixedDeltaTime;
    if (!m_interpolation)
        tInterpolation = 1; // If interpolation is not on, set interpolation to 1 (the current position)

    // Terrain
    RenderMesh(m_TerrainMeshName);

    RenderMeshes(m_TankLeft.RenderTank(tInterpolation));
    RenderMeshes(m_TankRight.RenderTank(tInterpolation));

    // Update sun path
    m_Sun.UpdateAndRender(deltaTimeSeconds);

    RenderMesh(m_BackgroundMeshName);
}

void TankWars::UpdateAndRenderDeathTransition()
{
    // Delay for a few seconds after a tank's death
    if (m_TimerDeathTransition > m_DelayDeathTransitionSeconds)
    {
        // Start the actual transition
        float t = (m_TimerDeathTransition - m_DelayDeathTransitionSeconds) / (m_DeathTransitionDurationSeconds * 2.4f);
        float scaleFactor = window->GetResolution().y / (tw::TankConstants::BODY_HEIGHT + tw::TankConstants::TURRET_RADIUS) / 1.5f;
        float positionX = tw::lerp(-scaleFactor * (tw::TankConstants::BODY_LENGTH / 2 + tw::TankConstants::SLOPE_WIDTH),
            scaleFactor * (window->GetResolution().x + tw::TankConstants::BODY_LENGTH / 2 + tw::TankConstants::SLOPE_WIDTH), t);
        glm::mat3 scaleTransform = transform2D::Scale(scaleFactor, scaleFactor);

        glm::mat3 modelMatrixTank = transform2D::Translate(positionX, 0) * scaleTransform * transform2D::Translate(-Tank::GetTankOrigin());
        glm::mat3 modelMatrixGun = transform2D::Translate(positionX, 0) * scaleTransform * transform2D::Translate(Tank::GetGunOrigin());
        glm::mat3 modelMatrixScreen = transform2D::Translate(positionX - window->GetResolution().x + 10 - scaleFactor * tw::TankConstants::TURRET_RADIUS, 0);

        RenderMesh(m_DeathGunMeshName, modelMatrixGun);
        RenderMesh(m_DeathTankMeshName, modelMatrixTank);
        RenderMesh(m_BlackScreenMeshName, modelMatrixScreen);

        if (positionX > window->GetResolution().x && !m_TriggerRestart)
        {
            m_TriggerRestart = 1;

			// Reset the level
            m_TerrainManager.GenerateRandomHeightMap();

            m_TankLeft.SetPositionX(GetInitialTankPosition(true));
            m_TankLeft.UpdateHeightAndRotation(m_TerrainManager, window->GetResolution());
            m_TankLeft.SetGunRotation(M_PI / 6);
            m_TankLeft.ResetHealth(tw::Healthbar::MAX_HEALTH_DEFAULT);
            m_TankLeftHasDied = 0;

            m_TankRight.SetPositionX(GetInitialTankPosition(false));
			m_TankRight.UpdateHeightAndRotation(m_TerrainManager, window->GetResolution());
            m_TankRight.ResetHealth(tw::Healthbar::MAX_HEALTH_DEFAULT);
			m_TankRight.SetGunRotation(M_PI - M_PI / 6);
            m_TankRightHasDied = 0;

			m_ProjectilesTankLeft.clear();
			m_ProjectilesTankRight.clear();
			m_SmokeGenerator.Clear();
        }
    }
    // When the animation is finished, reset parameters
    if (m_TimerDeathTransition > m_DeathTransitionDurationSeconds)
    {
        m_DeathTransition = 0;
        m_TimerDeathTransition = 0;
        m_TriggerRestart = 0;
    }
}

void TankWars::Smoke::UpdateAndRender(float deltaTimeSeconds)
{
    // Update each smoke particle's position, size, and opacity over time
    for (auto& particle : m_SmokeParticles)
    {
        particle.Lifetime -= deltaTimeSeconds;

        if (particle.Lifetime <= 0)
            continue; // Skip rendering if the particle has "died"

        // Update position based on velocity
        particle.Position += particle.Speed * deltaTimeSeconds;
        particle.SizeScalar += particle.GrowthSpeed * deltaTimeSeconds;

        // Render the smoke particle
        glm::mat3 modelMatrix = glm::mat3(1);
        modelMatrix *= transform2D::Translate(particle.Position);
        modelMatrix *= transform2D::Scale(particle.SizeScalar, particle.SizeScalar);
        m_TankWarsRef->RenderMesh(m_SmokeMeshName, modelMatrix);
    }

    for (int i = 0; i < m_SmokeParticles.size();)
    {
		if (m_SmokeParticles[i].Lifetime <= 0)
			m_SmokeParticles.erase(m_SmokeParticles.begin() + i);
		else
			i++;
    }
}

void TankWars::Smoke::InitSmokeMesh()
{
    const size_t pointsCircle = 20;
    const float radius = 1;
    vector<VertexFormat> vertices;
    vertices.reserve(pointsCircle + 1);
    // Center of the circle
    vertices.push_back(VertexFormat(glm::vec3(0, 0, 0), tw::TankConstants::COLOR_DEFAULT_GUN));
    // Rest of the points
    for (int i = 0; i < pointsCircle; i++) {
        float angle = 2.0f * M_PI * float(i) / float(pointsCircle - 1);
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        vertices.push_back(VertexFormat(glm::vec3(x, y, 0), tw::TankConstants::COLOR_DEFAULT_GUN));
    }

    vector<unsigned int> indices;
    indices.reserve(pointsCircle + 1);
    for (int i = 0; i < pointsCircle + 1; i++)
        indices.push_back(i);

    Mesh* smokeMesh = new Mesh(m_SmokeMeshName);
    smokeMesh->SetDrawMode(GL_TRIANGLE_FAN);
    smokeMesh->InitFromData(vertices, indices);

    m_TankWarsRef->AddMeshToList(smokeMesh);
}

void TankWars::Smoke::Clear()
{
	m_SmokeParticles.clear();
}

void TankWars::Smoke::GenerateSmokeParticles(float deltaTimeSeconds, const glm::vec2& tankPosition) 
{
    for (int i = 0; i < m_NumParticles; ++i)
    {
        Smoke::SmokeParticle particle;
        particle.Position = tankPosition;

        // Generate random direction and speed
		float angle = 2 * M_PI * tw::RandomDouble(0.f, 1.f);
        float speed = tw::lerp(m_MinSpeed, m_MaxSpeed, tw::RandomDouble(0.f, 1.f));
        particle.Speed = glm::vec2(cos(angle), sin(angle)) * speed;

        // Randomize with lerp
		particle.Lifetime = tw::lerp(m_MinLifetime, m_MaxLifetime, tw::RandomDouble(0.f, 1.f));
		particle.SizeScalar = tw::lerp(m_MinSize, m_MaxSize, tw::RandomDouble(0.f, 1.f));
		particle.GrowthSpeed = tw::lerp(m_MinGrowthSpeed, m_MaxGrowthSpeed, tw::RandomDouble(0.f, 1.f));
		particle.FadeSpeed = tw::lerp(m_MinFadeSpeed, m_MaxFadeSpeed, tw::RandomDouble(0.f, 1.f));

        m_SmokeParticles.push_back(particle);
    }
}

void TankWars::UpdateProjectiles(float deltaTime, vector<tw::Projectile>& projectiles)
{
	const glm::ivec2& windowSize = window->GetResolution();

    for (int i = 0; i < projectiles.size();)
    {
        auto& p = projectiles[i];
        p.PreviousPosition = p.Position;
        p.Speed.y -= tw::Gravity * deltaTime;
        p.Position += p.Speed * deltaTime;

        bool eraseProjectile = false;

        // Check if the projectile is out of bounds (do not deform terrain)
        if (p.Position.x < 0 || p.Position.x > windowSize.x)
            projectiles.erase(projectiles.begin() + i);
        else
            i++;
    }
}

void TankWars::CheckProjectilesTerrainCollision(std::vector<tw::Projectile>& projectiles)
{
    const glm::ivec2& windowSize = window->GetResolution();

    for (int i = 0; i < projectiles.size();)
    {
        // Check if the projectile is out of bounds (do not deform terrain)
        if (m_TerrainManager.CollidesWithProjectile(projectiles[i], windowSize.x))
            projectiles.erase(projectiles.begin() + i);
        else
           i++;
    }
}

float TankWars::GetInitialTankPosition(bool onTheLeft) const
{
	float scalar = (onTheLeft) ? 0.05f : 0.95f;
    return window->GetResolution().x * scalar;
}

void TankWars::FrameEnd()
{
}

void TankWars::OnInputUpdateTerrainEditor(float deltaTimeSeconds)
{
	// If both the fire buttons are pressed, change the game state to GAMEPLAY, game mode human vs human
    if (window->KeyHold(tw::s_ControlsLeft.Shoot) && window->KeyHold(tw::s_ControlsRight.Shoot))
    {
        m_GameState = GameState::GAMEPLAY_HUMAN_VS_HUMAN;
        std::cout << "Gameplay on... FIGHT!\n";
        m_DeathTransition = 0;
        m_TimerDeathTransition = 0;
        m_TriggerRestart = 0;
        return;
    }

    bool regenerateTerrain = false;
    if (window->KeyHold(GLFW_KEY_UP))
    {
        double speed = (window->KeyHold(GLFW_KEY_LEFT_SHIFT)) ? 150.F : 10.F;
        m_TerrainManager.TranslateResolution(deltaTimeSeconds * speed);

        regenerateTerrain = true;
    }
    if (window->KeyHold(GLFW_KEY_DOWN))
    {
        double speed = (window->KeyHold(GLFW_KEY_LEFT_SHIFT)) ? -150.F : -10.F;
        m_TerrainManager.TranslateResolution(deltaTimeSeconds * speed);

        regenerateTerrain = true;
    }
    if (window->KeyHold(GLFW_KEY_LEFT))
    {
        double speed = (window->KeyHold(GLFW_KEY_LEFT_SHIFT)) ? -5.F : 5.F;
        m_TerrainManager.TranslateFunctionStart(deltaTimeSeconds * speed);

        regenerateTerrain = true;
    }
    if (window->KeyHold(GLFW_KEY_RIGHT))
    {
        double speed = (window->KeyHold(GLFW_KEY_LEFT_SHIFT)) ? -5.F : 5.F;
        m_TerrainManager.TranslateFunctionEnd(deltaTimeSeconds * speed);

        regenerateTerrain = true;
    }
    if (window->KeyHold(GLFW_KEY_F))
    {
        float scalar = (window->KeyHold(GLFW_KEY_LEFT_SHIFT)) ? -0.05f : 0.05f;
        float speed = window->GetResolution().y * scalar;
        m_TerrainManager.TranslateFloor(deltaTimeSeconds * speed);
        regenerateTerrain = true;
    }
    if (window->KeyHold(GLFW_KEY_H))
    {
        double speed = (window->KeyHold(GLFW_KEY_LEFT_SHIFT)) ? -20.F : 20.F;
        m_TerrainManager.TranslateHeightScalar(deltaTimeSeconds * speed);
        regenerateTerrain = true;
    }
    if (window->KeyHold(GLFW_KEY_Z))
    {
        double speed = (window->KeyHold(GLFW_KEY_LEFT_SHIFT)) ? -1.F : 1.F;
        m_TerrainManager.TranslateBumpAmplitude(deltaTimeSeconds * speed);
        regenerateTerrain = true;
    }
    if (window->KeyHold(GLFW_KEY_X))
    {
        double speed = (window->KeyHold(GLFW_KEY_LEFT_SHIFT)) ? -1.F : 1.F;
		m_TerrainManager.TranslateHillAmplitude(deltaTimeSeconds * speed);
        regenerateTerrain = true;
    }
    if (window->KeyHold(GLFW_KEY_C))
    {
        double speed = (window->KeyHold(GLFW_KEY_LEFT_SHIFT)) ? -0.1F : 0.1F;
        m_TerrainManager.TranslateBumpFrequency(deltaTimeSeconds * speed);
        regenerateTerrain = true;
    }
    if (window->KeyHold(GLFW_KEY_V))
    {
        double speed = (window->KeyHold(GLFW_KEY_LEFT_SHIFT)) ? -0.1F : 0.1F;
        m_TerrainManager.TranslateHillFrequency(deltaTimeSeconds * speed);
        regenerateTerrain = true;
    }
    if (window->KeyHold(GLFW_KEY_B))
    {
        double speed = (window->KeyHold(GLFW_KEY_LEFT_SHIFT)) ? -1.F : 1.F;
		m_TerrainManager.TranslateBumpMaxCount(deltaTimeSeconds * speed);
        regenerateTerrain = true;
    }
    if (window->KeyHold(GLFW_KEY_N))
    {
        double speed = (window->KeyHold(GLFW_KEY_LEFT_SHIFT)) ? -1.F : 1.F;
        m_TerrainManager.TranslateHillMaxCount(deltaTimeSeconds * speed);
        regenerateTerrain = true;
    }
    if (window->KeyHold(GLFW_KEY_M))
    {
        double speed = (window->KeyHold(GLFW_KEY_LEFT_SHIFT)) ? -0.1F : 0.1F;
        m_TerrainManager.TranslateRandomness(deltaTimeSeconds * speed);
        regenerateTerrain = true;
    }

    if (regenerateTerrain)
    {
        GenerateTerrainMesh();
        PrintStats();
    }
}

void TankWars::OnInputUpdateGameplay(float deltaTimeSeconds)
{
}

void TankWars::OnInputUpdate(float deltaTime, int mods)
{
}

void TankWars::OnKeyPress(int key, int mods)
{
	switch (m_GameState)
	{
        case GameState::TERRAIN_EDITOR:
            // Add key press event
            switch (key)
            {
                case GLFW_KEY_G:
                    m_TerrainManager.GenerateRandomHeightMap();
                    GenerateTerrainMesh();
                    PrintStats();
                    break;
                case GLFW_KEY_I:
                    ToggleInterpolation();
                    break;
            }
            break;
        case GameState::GAMEPLAY_HUMAN_VS_HUMAN:
            if (key == GLFW_KEY_P)
            {
                m_GameState = GameState::TERRAIN_EDITOR;
                std::cout << "Terrain Editor enabled!\n";
                return;
            }

            bool addProjectile0 = false; 
            bool addProjectile1 = false; 
            tw::Projectile potentialProjectile0, potentialProjectile1;
            if (m_TankLeft.IsAlive())
            {
				addProjectile0 = m_TankLeft.OnKeyPress(key, potentialProjectile0);
            }
            if (m_TankLeft.IsAlive())
            {
                addProjectile1 = m_TankRight.OnKeyPress(key, potentialProjectile1);
            }

            if (addProjectile0 && m_ProjectilesTankLeft.size() < m_ProjectileCountMax)
            {
                m_ProjectilesTankLeft.push_back(potentialProjectile0);
            }
            if (addProjectile1 && m_ProjectilesTankRight.size() < m_ProjectileCountMax)
            {
                m_ProjectilesTankRight.push_back(potentialProjectile1);
            }
            break;
    }

    if (key == GLFW_KEY_I)
        ToggleInterpolation();
}

void TankWars::PrintStats()
{
	const TerrainManager::Stats& stats = m_TerrainManager.GetStats();

	std::cout << std::setprecision(2) << "Resolution: " << stats.Resolution << endl;
	std::cout << std::setprecision(4) << "Function start: " << stats.FunctionStart << endl;
	std::cout << "Function end: " << stats.FunctionEnd << endl;
	std::cout << "Floor: " << stats.Floor << endl;
	std::cout << "Height Scalar: " << stats.HeightScalar << endl;
	std::cout << std::setprecision(2) << "Bumps count: " << stats.RandomBumps.size() << endl;
	std::cout << "Hills count: " << stats.RandomHills.size() << endl;
	std::cout << "Bumps max count: " << stats.BumpsMax << endl;
	std::cout << "Hills max count: " << stats.HillsMax << endl;
    std::cout << "Bumps offset: " << stats.BaseBump.offset << endl;
	std::cout << "Bump amplitude: " << stats.BaseBump.amplitude << endl;
	std::cout << "Bump frequency: " << stats.BaseBump.frequency << endl;
    std::cout << "Hills offset: " << stats.BaseHill.offset << endl;
	std::cout << "Hill amplitude: " << stats.BaseHill.amplitude << endl;
	std::cout << "Hill frequency: " << stats.BaseHill.frequency << endl;
    std::cout << "Randomness: " << stats.RandomnessMargin << endl;
	std::cout << "Bumps ( " << stats.RandomBumps.size() << "): ";
	for (const auto& bump : stats.RandomBumps)
	{
		std::cout << std::setprecision(2) << "(" << bump.amplitude << ", " << std::setprecision(2) << bump.frequency << ") ";
	}
	std::cout << endl << "/--------------" << endl;
	std::cout << "Hills (" << stats.RandomHills.size() << "): ";
	for (const auto& hill : stats.RandomHills)
	{
		std::cout << std::setprecision(2) << "(" << hill.amplitude << ", " << std::setprecision(2) << hill.frequency << ") ";
	}
	std::cout << endl << "//==============================//" << endl;
}

void m1::TankWars::Sun::UpdateAndRender(float deltaTimeSeconds)
{
    m_Rotation += m_Speed * deltaTimeSeconds;
    m_SunOutlineCurrentRotation += m_RotationSpeed * 3.f * deltaTimeSeconds;
    m_SunCurrentRotation += m_RotationSpeed * deltaTimeSeconds;

    double x = m_RevolutionRotation.x + m_DistanceBig * cos(m_Rotation);
    double y = m_RevolutionRotation.y + m_DistanceSmall * sin(m_Rotation);

    if (x < -100)
        m_Rotation = 45;
    glm::mat3 modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(x, y);
    modelMatrix *= transform2D::Rotate(m_SunCurrentRotation);
    modelMatrix *= transform2D::Translate(-m_SunRadius / 2, -m_SunRadius / 2);
    m_DrawRef->RenderMesh(m_SunMeshName, modelMatrix);
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(x, y);
    modelMatrix *= transform2D::Rotate(m_SunOutlineCurrentRotation);
    modelMatrix *= transform2D::Translate(-m_SunOutlineRadius / 2, -m_SunOutlineRadius / 2);
    m_DrawRef->RenderMesh(m_SunOutlineMeshName, modelMatrix);
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(x, y);
    modelMatrix *= transform2D::Rotate(7 + -m_SunOutlineCurrentRotation * 1.25f);
    modelMatrix *= transform2D::Translate(-m_SunOutlineRadius / 2, -m_SunOutlineRadius / 2);
    m_DrawRef->RenderMesh(m_SunOutlineMeshName, modelMatrix);
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(x, y);
    modelMatrix *= transform2D::Rotate(10 + m_SunOutlineCurrentRotation * 1.2f);
    modelMatrix *= transform2D::Translate(-m_SunOutlineRadius / 2, -m_SunOutlineRadius / 2);
    m_DrawRef->RenderMesh(m_SunOutlineMeshName, modelMatrix);
}

void TankWars::Sun::InitSunMesh(Mesh*& sun, Mesh*& sunOutline)
{
    vector<VertexFormat> vertices
    {
        VertexFormat(glm::vec3(0, 0, 0), glm::vec3(0, 1, 1)),
        VertexFormat(glm::vec3(1, 0, 0), glm::vec3(1, 0, 1)),
        VertexFormat(glm::vec3(0, 1, 0), glm::vec3(1, 1, 0)),
        VertexFormat(glm::vec3(1, 1, 0), glm::vec3(0, 1, 1)),
    };

    vector<unsigned int> indices =
    {
        0, 1, 2,    // indices for first triangle
        1, 2, 3    // indices for second triangle
    };

    sun = tw::CreateSquare(m_SunMeshName, glm::vec3(0, 0, 0), 45, glm::vec3(0.871, 0.835, 0.047), true);
    sunOutline = tw::CreateSquare(m_SunOutlineMeshName, glm::vec3(0, 0, 0), 60, glm::vec3(0.871, 0.639, 0.047), true);
}

void TankWars::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
}

void TankWars::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
}

void TankWars::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
}

void TankWars::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void TankWars::OnWindowResize(int width, int height)
{
}
