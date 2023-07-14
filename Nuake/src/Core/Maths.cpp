#include "src/Core/Maths.h"

using namespace Nuake;

Quat Nuake::LookAt(Vector3 sourcePoint, Vector3 destPoint)
{
    Vector3 forwardVector = glm::normalize(destPoint - sourcePoint);

    float dot = glm::dot(Vector3(0, 0, 1), forwardVector);
    if (glm::abs(dot - (-1.0f)) < 0.000001f)
    {
        return Quat(0, 1, 0, 3.1415926535897932f);
    }
    if (glm::abs(dot - (1.0f)) < 0.000001f)
    {
        return Quat(); // identity
    }

    float rotAngle = acos(dot);
    Vector3 rotAxis = glm::cross(Vector3(0, 0, 1), forwardVector);
    rotAxis = glm::normalize(rotAxis);
    return CreateFromAxisAngle(rotAxis, rotAngle);
}

// just in case you need that function also
Quat Nuake::CreateFromAxisAngle(Vector3 axis, float angle)
{
    float halfAngle = angle * .5f;
    float s = sin(halfAngle);
    Quat q;
    q.x = axis.x * s;
    q.y = axis.y * s;
    q.z = axis.z * s;
    q.w = cos(halfAngle);
    return q;
}

Quat Nuake::QuatFromEuler(float x, float y, float z)
{
   glm::quat yawQuat = glm::angleAxis(Rad(y), glm::vec3(0.0f, 1.0f, 0.0f));
   glm::quat rollQuat = glm::angleAxis(Rad(z), glm::vec3(0.0f, 0.0f, 1.0f));
   glm::quat pitchQuat = glm::angleAxis(Rad(x), glm::vec3(1.0f, 0.0f, 0.0f));
   glm::quat orientation = yawQuat * pitchQuat * rollQuat;
   
   return glm::normalize(orientation);

    return Quat(Vector3(Rad(x), Rad(y), Rad(z)));

    Quat q;

    float cr = cos(x * 0.5);
    float sr = sin(x * 0.5);
    float cp = cos(y * 0.5);
    float sp = sin(y * 0.5);
    float cy = cos(z * 0.5);
    float sy = sin(z * 0.5);

    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;
    return q;
}

Vector3 Nuake::QuatToDirection(const Quat& quat)
{
	return glm::normalize(quat * Vector3(0, 0, -1));
	//return glm::normalize(glm::rotate(glm::inverse(quat), glm::vec3(-1.0, 0.0, 0.0)));
}

void Nuake::Decompose(const Matrix4& m, Vector3& pos, Quat& rot, Vector3& scale)
{
    pos = m[3];
    for (int i = 0; i < 3; i++)
        scale[i] = glm::length(Vector3(m[i]));
    const glm::mat3 rotMtx(
        glm::vec3(m[0]) / scale[0],
        glm::vec3(m[1]) / scale[1],
        glm::vec3(m[2]) / scale[2]);
    rot = glm::quat_cast(rotMtx);
}