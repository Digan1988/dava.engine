#include "Particles/ParticleForces.h"

#include "Particles/ParticleDragForce.h"
#include "Scene3D/Entity.h"
#include "Math/MathHelpers.h"
#include "Math/Noise.h"
#include "Particles/Particle.h"

#include <random>
#include <chrono>

namespace DAVA
{
namespace ParticleForces
{
const int32 noiseWidth = 256;
const int32 noiseHeight = 256;
Array<Array<Vector3, noiseWidth>, noiseHeight> noise;

void GenerateNoise()
{
    float32 xFactor = 2.0f / (noiseWidth - 1.0f);
    float32 yFactor = 2.0f / (noiseHeight - 1.0f);
    for (int32 i = 0; i < noiseWidth; ++i)
    {
        Vector2 q(i * xFactor, 0);
        for (int32 j = 0; j < noiseHeight; ++j)
        {
            q.y = j * yFactor;
            noise[i][j] = Generate4OctavesPerlin(q);
        }
    }
}

const uint32 sphereRandSize = 1024;
Array<Vector3, sphereRandSize> sphereRandomVectors;
void GenerateSphereRandomVectors()
{
    uint32 seed = static_cast<uint32>(std::chrono::system_clock::now().time_since_epoch().count());
    std::mt19937 generator(seed);
    std::uniform_real_distribution<float32> uinform01(0.0f, 1.0f);
    for (uint32 i = 0; i < sphereRandSize; ++i)
    {
        float32 theta = 2 * PI * uinform01(generator);
        float32 phi = acos(1 - 2 * uinform01(generator));
        float32 sinPhi = sin(phi);
        sphereRandomVectors[i] = { sinPhi * cos(theta), sinPhi * sin(theta), cos(phi) };
    }
}

const float32 windPeriod = 2 * PI;
const uint32 windTableSize = 64;
Array<float32, windTableSize> windValuesTable;

void Init()
{
    for (int32 i = 0; i < windTableSize; i++) // Taken from wind system.
    {
        float32 t = windPeriod * i / static_cast<float32>(windTableSize);
        windValuesTable[i] = (2.f + std::sin(t) * 0.7f + std::cos(t * 10) * 0.3f) * 0.5f;
    }
    GenerateNoise();
    GenerateSphereRandomVectors();
}

void ApplyDragForce(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle);
void ApplyLorentzForce(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle);
void ApplyPointGravity(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, Particle* particle);
void ApplyGravity(const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpaceDown, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle);
void ApplyWind(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle);
void ApplyPlaneCollision(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, Particle* particle, const Vector3& prevEffectSpacePosition);

Vector3 GetForceValue(const ParticleDragForce* force, float32 particleOverLife, float32 layerOverLife, float32 particleLife);
float32 GetTurbulenceValue(const ParticleDragForce* force, float32 particleOverLife, float32 layerOverLife, float32 particleLife);
float32 GetWindValueFromTable(const Vector3& inPosition, const ParticleDragForce* force, float32 layerOverLife, int32 index);

void ApplyForce(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, const Vector3& effectSpaceDown, Particle* particle, const Vector3& prevEffectSpacePosition)
{
    using ForceType = ParticleDragForce::eType;

    if (!force->isActive)
        return;
    if (force->type == ForceType::DRAG_FORCE)
    {
        ApplyDragForce(parent, force, effectSpaceVelocity, effectSpacePosition, dt, particleOverLife, layerOverLife, particle);
        return;
    }
    if (force->type == ForceType::LORENTZ_FORCE)
    {
        ApplyLorentzForce(parent, force, effectSpaceVelocity, effectSpacePosition, dt, particleOverLife, layerOverLife, particle);
        return;
    }
    if (force->type == ForceType::GRAVITY)
    {
        ApplyGravity(force, effectSpaceVelocity, effectSpaceDown, dt, particleOverLife, layerOverLife, particle);
        return;
    }
    if (force->type == ForceType::WIND)
    {
        ApplyWind(parent, force, effectSpaceVelocity, effectSpacePosition, dt, particleOverLife, layerOverLife, particle);
        return;
    }
    if (force->type == ForceType::POINT_GRAVITY)
    {
        ApplyPointGravity(parent, force, effectSpaceVelocity, effectSpacePosition, dt, particleOverLife, layerOverLife, particle);
        return;
    }
    if (force->type == ForceType::PLANE_COLLISION)
    {
        ApplyPlaneCollision(parent, force, effectSpaceVelocity, effectSpacePosition, dt, particleOverLife, layerOverLife, particle, prevEffectSpacePosition);
        return;
    }
}

bool IsPositionInForceShape(const Entity* parent, const ParticleDragForce* force, const Vector3& effectSpacePosition)
{
    Vector3 center = force->position;
    if (force->GetShape() == ParticleDragForce::eShape::BOX)
    {
        Vector3 p1 = center - force->GetHalfBoxSize();
        Vector3 p2 = center + force->GetHalfBoxSize();
        AABBox3 box(p1, p2);

        if (box.IsInside(effectSpacePosition))
            return true;
    }
    else if (force->GetShape() == ParticleDragForce::eShape::SPHERE)
    {
        float32 distSqr = (center - effectSpacePosition).SquareLength();
        if (distSqr <= force->GetSquareRadius())
            return true;
    }
    return false;
}

void ApplyDragForce(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle)
{
    if (!force->isInfinityRange)
    {
        if (!IsPositionInForceShape(parent, force, effectSpacePosition))
            return;
    }

    Vector3 forceStrength = GetForceValue(force, particleOverLife, layerOverLife, particle->life) * dt;
    Vector3 v(Max(0.0f, 1.0f - forceStrength.x), Max(0.0f, 1.0f - forceStrength.y), Max(0.0f, 1.0f - forceStrength.z));
    effectSpaceVelocity *= v;
}

void ApplyLorentzForce(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle)
{
    if (!force->isInfinityRange)
    {
        if (!IsPositionInForceShape(parent, force, effectSpacePosition))
            return;
    }

    Vector3 forceDir = (effectSpacePosition - force->position).CrossProduct(force->direction);
    float32 len = forceDir.SquareLength();
    if (len > 0.0f)
    {
        float32 d = 1.0f / std::sqrt(len);
        forceDir *= d;
    }
    Vector3 forceStrength = GetForceValue(force, particleOverLife, layerOverLife, particle->life) * dt;
    effectSpaceVelocity += forceStrength * forceDir;
}

void ApplyGravity(const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpaceDown, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle)
{
    effectSpaceVelocity += effectSpaceDown * GetForceValue(force, particleOverLife, layerOverLife, particle->life).x * dt;
}

void ApplyWind(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle)
{
    if (!force->isInfinityRange)
    {
        if (!IsPositionInForceShape(parent, force, effectSpacePosition))
            return;
    }
    Vector3 forceStrength = GetForceValue(force, particleOverLife, layerOverLife, particle->life) * dt;

    intptr_t partInd = reinterpret_cast<intptr_t>(particle);
    uint32 particleIndex = *reinterpret_cast<uint32*>(&partInd);
    Vector3 turbulence;
    float32 tubulencePower = GetTurbulenceValue(force, particleOverLife, layerOverLife, particle->life);

    uint32 offset = particleIndex % noiseWidth;
    float32 windMultiplier = 1.0f;
    if (Abs(tubulencePower) > EPSILON || Abs(force->windFrequency) > EPSILON)
    {
        float32 indexUnclamped = particleOverLife * noiseWidth * force->windTurbulenceFrequency + offset;
        float32 intPart = 0.0f;
        float32 fractPart = modf(particleOverLife * noiseWidth * force->windTurbulenceFrequency + offset, &intPart);
        uint32 xindex = static_cast<uint32>(intPart);

        xindex %= noiseWidth;
        uint32 yindex = offset % noiseHeight;
        uint32 nextIndex = (xindex + 1) % noiseWidth;
        Vector3 t1 = noise[xindex][yindex];
        Vector3 t2 = noise[nextIndex][yindex];
        turbulence = Lerp(t1, t2, fractPart);
    }
    if (Abs(force->windFrequency) > EPSILON)
        windMultiplier = turbulence.x + force->windBias; // TODO wind freq
    if (Abs(tubulencePower) > EPSILON)
    {
        //turbulence *= Vector3(sin(particleOverLife * 2 * PI * force->windTurbulenceFrequency), cos(particleOverLife * 2 * PI * force->windTurbulenceFrequency), sin(particleOverLife * 2 * PI * force->windTurbulenceFrequency) * cos(particleOverLife * 2 * PI * force->windTurbulenceFrequency) * 2.0f);
        if ((100 - force->backwardTurbulenceProbability) > offset % 100)
        {
            float32 dot = Normalize(force->direction).DotProduct(Normalize(turbulence));
            if (dot < 0)
                turbulence *= -1.0f;
        }
        turbulence *= tubulencePower * dt;
        effectSpacePosition += turbulence; // how with drug and other forces, turb not adding to velocity? turbulence to drug and multiply by v. add to position when velocity added. note add drug to v.
    }
    static const float32 windScale = 100.0f; // Artiom request.
    effectSpaceVelocity += force->direction * dt * /*GetWindValueFromTable(effectSpacePosition, force, particleOverLife, particleIndex)*/ windMultiplier * forceStrength.x * windScale; // +turbulence;
}

void ApplyPointGravity(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, Particle* particle)
{
    if (!force->isInfinityRange)
    {
        if (!IsPositionInForceShape(parent, force, effectSpacePosition))
            return;
    }
    Vector3 forceStrength = GetForceValue(force, particleOverLife, layerOverLife, particle->life) * dt;
    Vector3 forcePosition = force->position;
    Vector3 forceDirection;
    Vector3 toCenter;
    float32 toCenterDist = -1;
    float32 distToTarget = -1;
    if (force->pointGravityUseRandomPointsOnSphere) // TODO
    {
        intptr_t partInd = reinterpret_cast<intptr_t>(particle);
        uint32 particleIndex = *reinterpret_cast<uint32*>(&partInd);
        particleIndex %= sphereRandSize;
        forcePosition += sphereRandomVectors[particleIndex] * force->pointGravityRadius;
        forceDirection = forcePosition - effectSpacePosition;
        toCenter = (force->position - effectSpacePosition);
        toCenterDist = toCenter.SquareLength();
        distToTarget = forceDirection.SquareLength();
        if (toCenterDist > 0)
            toCenter.Normalize();
    }
    else
    {
        forceDirection = forcePosition - effectSpacePosition;
        toCenter = forceDirection;
        toCenterDist = toCenter.SquareLength();
        if (toCenterDist > 0)
            toCenter.Normalize();
        distToTarget = toCenterDist;
    }

    if (distToTarget > 0)
        forceDirection /= sqrt(distToTarget);

    if (toCenterDist > force->pointGravityRadius * force->pointGravityRadius)
        effectSpaceVelocity += forceDirection * forceStrength;
    else
    {
        if (force->killParticles)
            particle->life = particle->lifeTime + 0.1f;
        else
            effectSpacePosition = force->position - force->pointGravityRadius * toCenter;
    }
}

void ApplyPlaneCollision(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, Particle* particle, const Vector3& prevEffectSpacePosition)
{
    float32 sqrLen = force->direction.SquareLength();
    if (sqrLen < EPSILON * EPSILON)
        return;
    if (!force->isInfinityRange)
    {
        if (!IsPositionInForceShape(parent, force, effectSpacePosition))
            return;
    }
    Vector3 normal = force->direction;
    float32 invLen = 1.0f / sqrt(sqrLen);
    normal *= invLen;
    Vector3 a = prevEffectSpacePosition - force->position;
    Vector3 b = effectSpacePosition - force->position;
    float32 bProj = b.DotProduct(normal);
    if (bProj <= 0 && a.DotProduct(normal) > 0)
    {
        if (effectSpaceVelocity.SquareLength() < force->velocityThreshold * force->velocityThreshold)
        {
            effectSpaceVelocity = Vector3::Zero;
            return;
        }
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<int32> uniInt(0, 99);
        int32 rnd100 = uniInt(rng);
        bool reflectParticle = static_cast<uint32>(rnd100) < force->reflectionPercent;
        if (force->killParticles && !reflectParticle)
        {
            particle->life = particle->lifeTime + 0.1f;
            return;
        }

        Vector3 newVel;

        if (force->normalAsReflectionVector)
            newVel = (normal * effectSpaceVelocity.Length()); // Artiom request.
        else
            newVel = Reflect(effectSpaceVelocity, normal);

        Vector3 rndVec;
        Quaternion q;

        if (abs(force->reflectionChaos) > EPSILON)
        {
            intptr_t partInd = reinterpret_cast<intptr_t>(particle);
            uint32 particleIndex = *reinterpret_cast<uint32*>(&partInd);
            particleIndex %= sphereRandSize;
            rndVec = sphereRandomVectors[particleIndex];
            if (rndVec.DotProduct(normal) < 0)
                rndVec = -rndVec;

            std::random_device rd;
            std::mt19937 rng(rd());
            std::uniform_real_distribution<float32> uni(-force->reflectionChaos, force->reflectionChaos);

            float32 random_x = DegToRad(uni(rng));
            float32 random_y = DegToRad(uni(rng));
            float32 random_z = DegToRad(uni(rng));
            q = Quaternion::MakeRotationFastX(random_x) * Quaternion::MakeRotationFastY(random_y) * Quaternion::MakeRotationFastZ(random_z);
            newVel = q.ApplyToVectorFast(newVel);
            if (newVel.DotProduct(normal) < 0)
                newVel = -newVel;
        }
        effectSpaceVelocity = newVel * force->forcePower;
        if (force->randomizeReflectionForce)
        {
            std::random_device rd;
            std::mt19937 rng(rd());
            std::uniform_real_distribution<float32> uni(force->rndReflectionForceMin, force->rndReflectionForceMax);
            effectSpaceVelocity *= uni(rng);
        }

        Vector3 dir = prevEffectSpacePosition - effectSpacePosition;
        float32 abProj = abs(dir.DotProduct(normal));
        if (abProj < EPSILON)
            return;

        effectSpacePosition = effectSpacePosition + dir * (-bProj) / abProj;

        if (!reflectParticle)
        {
            if (force->killParticles)
                particle->life = particle->lifeTime + 0.1f;
            else
                effectSpaceVelocity = Vector3::Zero;
        }
    }
    else if ((bProj < 0.0f && a.DotProduct(normal) < 0.0f))
    {
        if (force->killParticles)
            particle->life = particle->lifeTime + 0.1f;
        else
            effectSpaceVelocity = Vector3::Zero;
    }
}

Vector3 GetForceValue(const ParticleDragForce* force, float32 particleOverLife, float32 layerOverLife, float32 particleLife)
{
    if (force->timingType == ParticleDragForce::eTimingType::CONSTANT || force->forcePowerLine == nullptr)
        return force->forcePower;

    if (force->timingType == ParticleDragForce::eTimingType::OVER_PARTICLE_LIFE)
        return force->forcePowerLine->GetValue(particleOverLife);

    if (force->timingType == ParticleDragForce::eTimingType::OVER_LAYER_LIFE)
        return force->forcePowerLine->GetValue(layerOverLife);

    if (force->timingType == ParticleDragForce::eTimingType::SECONDS_PARTICLE_LIFE)
        return force->forcePowerLine->GetValue(particleLife);

    return force->forcePower;
}

float32 GetTurbulenceValue(const ParticleDragForce* force, float32 particleOverLife, float32 layerOverLife, float32 particleLife)
{
    if (force->timingType == ParticleDragForce::eTimingType::CONSTANT || force->turbulenceLine == nullptr)
        return force->windTurbulence;

    if (force->timingType == ParticleDragForce::eTimingType::OVER_PARTICLE_LIFE)
        return force->turbulenceLine->GetValue(particleOverLife);

    if (force->timingType == ParticleDragForce::eTimingType::OVER_LAYER_LIFE)
        return force->turbulenceLine->GetValue(layerOverLife);

    if (force->timingType == ParticleDragForce::eTimingType::SECONDS_PARTICLE_LIFE)
        return force->turbulenceLine->GetValue(particleLife);

    return force->windTurbulence;
}

float32 GetWindValueFromTable(const Vector3& inPosition, const ParticleDragForce* force, float32 layerOverLife, int32 index)
{
    if (abs(force->windFrequency) < EPSILON)
        return 1.0f;

    Vector3 dir = force->direction;
    Vector3 projPt = dir * inPosition.DotProduct(dir);
    float32 tMod = std::fmod((Abs(index) + layerOverLife) * force->windFrequency, windPeriod);
    int32 i = static_cast<int32>(std::floor(tMod / windPeriod * windTableSize));
    return (std::sin(Abs(index) % 255 + layerOverLife * force->windFrequency) * 0.6f + std::cos(Abs(index) % 255 + layerOverLife * force->windFrequency) * 0.4f) + force->windBias;
    DVASSERT(i >= 0 && i < windTableSize);
    return windValuesTable[i];
}
}
}