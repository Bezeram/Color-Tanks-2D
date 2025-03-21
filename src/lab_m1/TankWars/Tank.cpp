#include "Tank.h"

using namespace std;
using namespace m1;

Tank::Tank(std::string_view name, const tw::TankControls& controls, int health)
	: m_TankMeshName(name)
	, m_MaxHealth(health)
	, m_Health(health)
    , controls(controls)
    , m_GunMeshName(m_TankMeshName + "_gun")
{
	float lengthSlopeToTurret = tw::TankConstants::BODY_LENGTH / 2.f - tw::TankConstants::TURRET_RADIUS;

    // Left side, from slope to turret inside the body
    m_HitboxRelative[0] = { {tw::TankConstants::SLOPE_WIDTH + lengthSlopeToTurret / 2.f, tw::TankConstants::BODY_HEIGHT / 2.f}, tw::TankConstants::BODY_HEIGHT / 2.f };
    // Right side, from slope to turret inside the body
    m_HitboxRelative[1] = { {m_HitboxRelative[0].Center + glm::vec2(lengthSlopeToTurret + tw::TankConstants::TURRET_RADIUS * 2, 0)}, m_HitboxRelative[0].Radius};
    // Turret
    m_HitboxRelative[2] = { {tw::TankConstants::SLOPE_WIDTH + tw::TankConstants::BODY_LENGTH / 2.f, tw::TankConstants::BODY_HEIGHT}, tw::TankConstants::TURRET_RADIUS};
}

Tank::Tank(std::string_view name, const tw::TankControls& controls, int health, float initialPositionX, const glm::vec3& bodyColor, const glm::vec3& tracksColor, const glm::vec3& gunColor)
	: m_TankMeshName(name)
    , controls(controls)
	, m_MaxHealth(health)
	, m_Health(health)
    , m_GunMeshName(m_TankMeshName + "_gun")
    , m_TankPosition(initialPositionX)
    , m_colorBody(bodyColor)
    , m_colorTracks(tracksColor)
    , m_colorGun(gunColor)
{
    float lengthSlopeToTurret = tw::TankConstants::BODY_LENGTH / 2.f - tw::TankConstants::TURRET_RADIUS;

    // Left side, from slope to turret inside the body
    m_HitboxRelative[0] = { {tw::TankConstants::SLOPE_WIDTH + lengthSlopeToTurret / 2.f, tw::TankConstants::BODY_HEIGHT / 2.f}, tw::TankConstants::BODY_HEIGHT / 2.f };
    // Right side, from slope to turret inside the body
    m_HitboxRelative[1] = { {m_HitboxRelative[0].Center + glm::vec2(lengthSlopeToTurret + tw::TankConstants::TURRET_RADIUS * 2)}, m_HitboxRelative[0].Radius };
    // Turret
    m_HitboxRelative[2] = { {tw::TankConstants::SLOPE_WIDTH + tw::TankConstants::BODY_LENGTH / 2.f, tw::TankConstants::BODY_HEIGHT}, tw::TankConstants::TURRET_RADIUS };
}

void Tank::Init(float initialPositionX, float initialGunRotation, glm::vec3 bodyColor, glm::vec3 tracksColor, const TerrainManager& terrain, const glm::ivec2& windowSize)
{
	m_TankPosition.x = initialPositionX;
	m_GunRotation = tw::clamp(initialGunRotation, 0.f, M_PI);
	m_PreviousTankPosition = m_TankPosition;
	m_colorBody = bodyColor;
	m_colorTracks = tracksColor;

    m_BodyMesh = InitTankMesh(m_TankMeshName, tracksColor, bodyColor);
	m_GunMesh = InitGunMesh(m_GunMeshName, tw::TankConstants::COLOR_DEFAULT_GUN);
}

void Tank::Move(float distance)
{
	m_TankPosition.x += distance;
}

Tank::UpdateStatus Tank::UpdateGameplay(float deltaTime, TerrainManager& terrain, const WindowObject* window, std::vector<tw::Projectile>& projectilesLeft, std::vector<tw::Projectile>& projectilesRight)
{
	UpdateStatus updateStatus;
    if (!IsAlive())
    {
        return updateStatus;
    }

	const glm::ivec2& windowSize = window->GetResolution();
    // Update previous transformations
    m_PreviousTankPosition = m_TankPosition;
    m_PreviousTankRotation = m_TankRotation;
	m_PrevGunRotation = m_GunRotation;

    OnInputUpdateGameplay(deltaTime, window);

    // Move terrain based on tank angle
    if (abs(m_TankRotation) > 0.5)
        m_TankPosition.x += (-m_TankRotation) * m_TankFakeGravityScalar * deltaTime;

    // Update new parameters
    // Clamp parameters rotation to not clip with the body
	m_GunRotation = tw::clamp(m_GunRotation, m_TankRotation, m_TankRotation + M_PI);
	m_TankPosition.x = tw::clamp(m_TankPosition.x, tw::TankConstants::BODY_LENGTH / 2.f, windowSize.x - tw::TankConstants::BODY_LENGTH / 2.f);

    // Set the tank at the correct height and rotation
	UpdateHeightAndRotation(terrain, window->GetResolution());

	// Check if the tank was hit by a projectile
	CheckProjectilesCollision(projectilesLeft);
	CheckProjectilesCollision(projectilesRight);

	return updateStatus;
}

void Tank::UpdateTitleScreen(float deltaTime, const TerrainManager& terrain, const WindowObject* window)
{
    // Update previous transformations
    m_PreviousTankPosition = m_TankPosition;
    m_PreviousTankRotation = m_TankRotation;
    m_PrevGunRotation = m_GunRotation;

    // Clamp gun rotation to not clip with the body
    m_GunRotation = tw::clamp(m_GunRotation, m_TankRotation, m_TankRotation + M_PI);

    // Set the tank at the correct height
    UpdateHeightAndRotation(terrain, window->GetResolution());
}

std::vector<tw::RenderData> Tank::RenderTank(float tInterpolation) const
{
    glm::vec2 renderPosition = tw::lerp(m_PreviousTankPosition, m_TankPosition, tInterpolation);
    float renderTankAngle = tw::lerp(m_PreviousTankRotation, m_TankRotation, tInterpolation);

    // The real rotation is also dependent on the tank rotation, 
    // so we want to subtract it to make the gun rotation independent
    float prevRealGunRotation = m_PrevGunRotation - m_PreviousTankRotation;
    float realGunRotation = m_GunRotation - m_TankRotation;
    float renderGunAngle = prevRealGunRotation * (1 - tInterpolation) + realGunRotation * tInterpolation;
	renderGunAngle = tw::clamp(renderGunAngle, 0, M_PI); // clamp gun rotation to not clip with the body (the gun is already rendered relative to the tank)

    glm::mat3 transformMatrixTank = glm::mat3(1) *
    transform2D::Translate(renderPosition.x, renderPosition.y) *
    transform2D::Rotate(renderTankAngle) *
    transform2D::Translate(-GetTankOrigin().x, -GetTankOrigin().y);

    glm::mat3 transformMatrixGun = glm::mat3(1) *
    transform2D::Translate(renderPosition) *
    transform2D::Rotate(renderTankAngle) *
    transform2D::Translate(GetGunOrigin()) *
    transform2D::Translate(GetGunOriginRotation()) *
    transform2D::Rotate(renderGunAngle) *
    transform2D::Translate(-GetGunOriginRotation());
    
    std::vector<tw::RenderData > renderData
    {
        tw::RenderData(m_TankMeshName.c_str(), "VertexColor", transformMatrixTank),
        tw::RenderData(m_GunMeshName.c_str(), "VertexColor", transformMatrixGun),
    };

    return renderData;
}

void Tank::OnInputUpdateGameplay(float deltaTime, const WindowObject* window)
{
	float speedBoostIfShift = (window->KeyHold(controls.Accelerate)) ? 1.5f : 1.f;

    if (window->KeyHold(controls.MoveLeft))
    {
        Move(speedBoostIfShift * -m_TankSpeed * deltaTime);
    }
    if (window->KeyHold(controls.MoveRight))
    {
        Move(speedBoostIfShift * m_TankSpeed * deltaTime);
    }
    if (window->KeyHold(controls.RotateGunUp))
    {
        m_GunRotation += speedBoostIfShift * m_GunRotationSpeed * deltaTime;
    }
    if (window->KeyHold(controls.RotateGunDown))
    {
        m_GunRotation += -1 * speedBoostIfShift * m_GunRotationSpeed * deltaTime;
    }
}

bool Tank::OnKeyPress(int key, tw::Projectile& potentialProjectile)
{
	bool addedProjectile = false;

    if (key == controls.Shoot)
    {
        potentialProjectile = Shoot();
		addedProjectile = true;
    }

	return addedProjectile;
}

Mesh* Tank::InitTankMesh(const std::string& nameMesh, glm::vec3 colorTracks, glm::vec3 colorBody)
{
	const float slopeWidth = tw::TankConstants::SLOPE_WIDTH;
	const float trackWidth = slopeWidth / 2.f;
    const float trackLength = tw::TankConstants::BODY_LENGTH - 2 * trackWidth;
    const float trackHeight = tw::TankConstants::BODY_HEIGHT / 2.f;
    vector<VertexFormat> vertices
    {
        // Body left slope
        VertexFormat(glm::vec3(0, 0, 0), colorTracks),
        VertexFormat(glm::vec3(slopeWidth, 0, 0), colorTracks),
        VertexFormat(glm::vec3(slopeWidth, tw::TankConstants::BODY_HEIGHT, 0), colorBody),
        // Body right slope
        VertexFormat(glm::vec3(slopeWidth + tw::TankConstants::BODY_LENGTH, tw::TankConstants::BODY_HEIGHT, 0), colorBody),
        VertexFormat(glm::vec3(slopeWidth + tw::TankConstants::BODY_LENGTH, 0, 0), colorTracks),
        VertexFormat(glm::vec3(slopeWidth + tw::TankConstants::BODY_LENGTH + slopeWidth, 0, 0), colorTracks),

        // Tracks left slope
        VertexFormat(glm::vec3(slopeWidth, 0, 0), colorTracks),
        VertexFormat(glm::vec3(slopeWidth + trackWidth, 0, 0), colorTracks),
        VertexFormat(glm::vec3(slopeWidth + trackWidth, -trackHeight, 0), colorTracks),
        // Tracks right slope
        VertexFormat(glm::vec3(slopeWidth + trackWidth + trackLength, -trackHeight, 0), colorTracks),
        VertexFormat(glm::vec3(slopeWidth + trackWidth + trackLength, 0, 0), colorTracks),
        VertexFormat(glm::vec3(slopeWidth + trackWidth + trackLength + trackWidth, 0, 0), colorTracks),
    };

	const size_t pointsCircle = 100;
    const float radius = tw::TankConstants::TURRET_RADIUS;
	// Center of the circle
    vertices.push_back(VertexFormat(glm::vec3(slopeWidth + tw::TankConstants::BODY_LENGTH / 2.f, tw::TankConstants::BODY_HEIGHT, 0), colorTracks));
    for (int i = 0; i < pointsCircle; i++) {
        float angle = 2.0f * M_PI * float(i) / float(pointsCircle); // Calculate the angle for this segment
        float x = radius * cos(angle) + slopeWidth + tw::TankConstants::BODY_LENGTH / 2.f; // Calculate x component
        float y = radius * sin(angle) + tw::TankConstants::BODY_HEIGHT + tw::TankConstants::GUN_HEIGHT / 2.f; // Calculate y component
		vertices.push_back(VertexFormat(glm::vec3(x, y, 0), colorBody));
    }

    vector<unsigned int> indices
    {
        0, 1, 2,    // body left slope
		1, 2, 3,    // body 1
		1, 3, 4,    // body 2
        3, 4, 5,    // body right slope

		6, 7, 8,    // tracks left slope
		7, 8, 9,    // tracks 1
		7, 9, 10,   // tracks 2
		9, 10, 11,  // tracks right slope
    };

	for (int i = 0; i < pointsCircle - 1; i++) {
		indices.push_back(12);
		indices.push_back(i + 12 + 1);
		indices.push_back(i + 12 + 2);
	}

    indices.push_back(12);                      // Center vertex
    indices.push_back(pointsCircle + 11);       // Last vertex on the circle
    indices.push_back(13);                      // First vertex on the circle

    Mesh* tankMesh = new Mesh(nameMesh);
	tankMesh->InitFromData(vertices, indices);
	return tankMesh;
}

Mesh* Tank::InitGunMesh(const std::string& nameMesh, glm::vec3 colorGun)
{
	vector<VertexFormat> vertices = 
    {
		VertexFormat(glm::vec3(0, 0, 0), colorGun),
		VertexFormat(glm::vec3(tw::TankConstants::GUN_LENGTH, 0, 0), colorGun - glm::vec3(0.1f)),
		VertexFormat(glm::vec3(tw::TankConstants::GUN_LENGTH, tw::TankConstants::GUN_HEIGHT, 0), colorGun - glm::vec3(0.1f)),
		VertexFormat(glm::vec3(0, tw::TankConstants::GUN_HEIGHT, 0), colorGun),
	};

	vector<unsigned int> indices = 
    {
		0, 1, 2,
		0, 2, 3,
	};

    Mesh* mesh = new Mesh(nameMesh);
    mesh->InitFromData(vertices, indices);
	return mesh;
}

tw::Projectile Tank::Shoot() const
{
    // The real rotation is also dependent on the tank rotation, 
    // so we want to subtract it to make the gun rotation independent
	float realGunRotation = m_GunRotation - m_TankRotation;

	// Calculate the position of the projectile similar to the gun
    // We use a different origin which extends to the end of the gun when rotating according to the tank rotation
    glm::mat3 transformMatrixProjectile = glm::mat3(1) *
    transform2D::Translate(m_TankPosition) *
    transform2D::Rotate(m_TankRotation) *
    transform2D::Translate(GetProjectileOrigin()) *
    transform2D::Translate(GetProjectileOriginRotation()) *
    transform2D::Rotate(realGunRotation) *
    transform2D::Translate(-GetProjectileOriginRotation());

    tw::Projectile projectile;
    // Get the translation from the matrix
    projectile.Position = { transformMatrixProjectile[2][0], transformMatrixProjectile[2][1] };
    projectile.PreviousPosition = projectile.Position;
	projectile.Speed = glm::vec2(cos(m_GunRotation), sin(m_GunRotation)) * tw::Projectile::INITIAL_PROJECTILE_SPEED;
    return projectile;
}

void Tank::UpdateHeightAndRotation(const TerrainManager& terrain, const glm::ivec2& windowSize)
{
    // Set the tank at the correct height
    float outHeightLeft, outAngleLeft, outHeightRight, outAngleRight;
    terrain.CalculateHeightAndAngle(m_TankPosition.x, windowSize.x, outHeightLeft, outAngleLeft);
    terrain.CalculateHeightAndAngle(m_TankPosition.x + 3, windowSize.x, outHeightRight, outAngleRight);

    m_TankPosition.y = (outHeightLeft + outHeightRight) / 2.f;
    m_TankRotation = (outAngleLeft + outAngleRight) / 2.f;
}

std::vector<tw::RenderData> Tank::RenderHealthbar(float tInterpolation) const
{
    glm::vec2 renderPosition = tw::lerp(m_PreviousTankPosition, m_TankPosition, tInterpolation);
    float renderTankAngle = tw::lerp(m_PreviousTankRotation, m_TankRotation, tInterpolation);

    // The real rotation is also dependent on the tank rotation, 
    // so we want to subtract it to make the gun rotation independent
    float prevRealGunRotation = m_PrevGunRotation - m_PreviousTankRotation;
    float realGunRotation = m_GunRotation - m_TankRotation;
    float renderGunAngle = prevRealGunRotation * (1 - tInterpolation) + realGunRotation * tInterpolation;
    renderGunAngle = tw::clamp(renderGunAngle, 0, M_PI); // clamp gun rotation to not clip with the body (the gun is already rendered relative to the tank)

	float tScaleForeground = m_Health / (float)m_MaxHealth;
	glm::mat3 scaleForeground = transform2D::Scale(tScaleForeground, 1);
    glm::vec2 offset = glm::vec2(tw::TankConstants::SLOPE_WIDTH + tw::TankConstants::BODY_LENGTH / 2.f, tw::TankConstants::BODY_HEIGHT / 2.f) - GetHealthbarOrigin();
    glm::mat3 modelMatrixBackground = glm::mat3(1) *
        transform2D::Translate(renderPosition.x, renderPosition.y) *
        transform2D::Rotate(renderTankAngle) *
        transform2D::Translate(offset - GetTankOrigin());
	glm::mat3 modelMatrixForeground = modelMatrixBackground * 
        transform2D::Scale(tScaleForeground, 1.f);

	std::vector<tw::RenderData> renderData
	{
		tw::RenderData(s_HealthbarForegroundMeshName.c_str(), "VertexColor", modelMatrixForeground),
		tw::RenderData(s_HealthbarBackgroundMeshName.c_str(), "VertexColor", modelMatrixBackground),
	};

	return renderData;
}

std::vector<tw::RenderData> Tank::Render(float tInterpolation) const
{
    if (!IsAlive())
        return {};

    std::vector<tw::RenderData> dataTank = RenderTank(tInterpolation);
    std::vector<tw::RenderData> dataHealthbar = RenderHealthbar(tInterpolation);
    
	std::vector<tw::RenderData> renderData = dataTank;
    renderData.insert(renderData.begin(), dataHealthbar.begin(), dataHealthbar.end());

    return renderData;
}

void Tank::CheckProjectilesCollision(std::vector<tw::Projectile>& projectiles)
{
	// Translate the relative hitboxes to the tank position
    Hitbox hitboxesTransformed[3] = { m_HitboxRelative[0], m_HitboxRelative[1], m_HitboxRelative[2] };
    for (auto& hitbox : hitboxesTransformed)
    {
        glm::mat3 transformMatrix = glm::mat3(1) *
            transform2D::Translate(GetTankPosition()) *
            transform2D::Rotate(GetTankRotation()) *
            transform2D::Translate((hitbox.Center) - GetTankOrigin());
        glm::vec2 translation = glm::vec2(transformMatrix[2][0], transformMatrix[2][1]);
        hitbox.Center = translation;
    }

	for (int i = 0; i < projectiles.size() && m_Health > 0;)
	{
        bool eraseProjectile = false;

		for (auto& hitbox : hitboxesTransformed)
		{
            if (IntersectsWithHitbox(projectiles[i], hitbox))
            {
                m_Health--;
                eraseProjectile = true;
            }
			if (eraseProjectile)
				break;
		}

        if (eraseProjectile)
            projectiles.erase(projectiles.begin() + i);
        else
            i++;
	}
}

bool Tank::IntersectsWithHitbox(const tw::Projectile& projectile, const Hitbox& hitbox)
{
    float distance = glm::distance(projectile.Position, hitbox.Center);
    return distance <= (tw::Projectile::PROJECTILE_RADIUS + hitbox.Radius);
}

bool Tank::IsAlive() const
{
    const float error = 10;
    return m_Health > 0 && m_TankPosition.y > -(tw::TankConstants::GUN_LENGTH + tw::TankConstants::BODY_HEIGHT + tw::TankConstants::TURRET_RADIUS + error);
}

Mesh* Tank::GetBodyMesh() const
{
    return m_BodyMesh;
}

Mesh* Tank::GetGunMesh() const
{
    return m_GunMesh;
}

void Tank::SetRotation(float rotation)
{
	m_TankRotation = rotation;
}

void Tank::SetGunRotation(float rotation)
{
	m_GunRotation = rotation;
}

void Tank::ResetHealth(int health)
{
	m_Health = health;
}

const glm::vec2& Tank::GetTankPosition() const
{
    return m_TankPosition;
}

const glm::vec2& Tank::GetPreviousPosition() const
{
    return m_PreviousTankPosition;
}

float Tank::GetTankRotation() const
{
    return m_TankRotation;
}

float m1::Tank::GetPreviousTankRotation() const
{
    return m_PreviousTankRotation;
}

glm::vec2 Tank::GetTankOrigin()
{
    const float slopeWidth = tw::TankConstants::SLOPE_WIDTH;
	const float trackHeight = tw::TankConstants::BODY_HEIGHT / 2.f;
	return glm::vec2(slopeWidth + tw::TankConstants::BODY_LENGTH / 2.f, -trackHeight);
}

glm::vec2 Tank::GetGunOrigin()
{
    const float slopeWidth = tw::TankConstants::SLOPE_WIDTH;
    float offset = 2.f;
    return glm::vec2(slopeWidth + tw::TankConstants::BODY_LENGTH / 2.f + tw::TankConstants::TURRET_RADIUS - offset, tw::TankConstants::BODY_HEIGHT) - GetTankOrigin();
}

glm::vec2 Tank::GetProjectileOrigin() const
{
    return GetGunOrigin() + glm::vec2(tw::TankConstants::GUN_LENGTH, tw::TankConstants::GUN_HEIGHT / 2.f);
}

glm::vec2 Tank::GetGunOriginRotation() const
{
    float offset = 2.f;
    return glm::vec2(-tw::TankConstants::TURRET_RADIUS + offset, tw::TankConstants::GUN_HEIGHT / 2.f);
}

glm::vec2 Tank::GetProjectileOriginRotation() const
{
    return GetGunOriginRotation() - glm::vec2(tw::TankConstants::GUN_LENGTH, tw::TankConstants::GUN_HEIGHT / 2.f);
}

float Tank::GetGunRotation() const
{
    return m_GunRotation;
}

float Tank::GetPreviousGunRotation() const
{
    return m_PrevGunRotation;
}

glm::vec2 Tank::GetHealthbarOrigin() const
{
    return { tw::TankConstants::BODY_LENGTH / 3.f, 5.f };
}

void Tank::SetPositionX(float position)
{
	m_TankPosition.x = position;
}

int Tank::GetHealth() const
{
    return m_Health;
}

const std::string& Tank::GetMeshBodyName() const
{
    return m_TankMeshName;
}

const std::string& Tank::GetMeshGunName() const
{
    return m_GunMeshName;
}

Tank::Hitbox Tank::GetHitbox(int idx) const
{
	return m_HitboxRelative[idx];
}
