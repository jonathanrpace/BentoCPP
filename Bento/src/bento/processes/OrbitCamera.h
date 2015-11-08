#pragma once

// bento
#include <bento.h>
#include <bento/core/Scene.h>
#include <bento/core/Process.h>
#include <bento/core/SharedObject.h>

namespace bento
{
	class OrbitCamera
		: public Process
		, public SharedObject<OrbitCamera>
	{
	public:
		OrbitCamera(std::string _name = "OrbitCamera");

		// from Process
		virtual void BindToScene(Scene * const _scene) override;
		virtual void UnbindFromScene(Scene * const _scene) override;
		virtual void Advance(double dt) override;

		float Dolly() const { return m_dolly; }
		void Dolly(float val) { m_dolly = val; }
		float DollyTarget() const { return m_dollyTarget; }
		void DollyTarget(float val) { m_dollyTarget = val; }
		float DollySpeed() const { return m_dollySpeed; }
		void DollySpeed(float val) { m_dollySpeed = val; }
		float DollyEase() const { return m_dollyEase; }
		void DollyEase(float val) { m_dollyEase = val; }
		float DollyMin() const { return m_dollyMin; }
		void DollyMin(float val) { m_dollyMin = val; }
		float DollyMax() const { return m_dollyMax; }
		void DollyMax(float val) { m_dollyMax = val; }

		vec3 Position() const { return m_position; }
		void Position(vec3 val) { m_position = val; }
		vec3 PositionTarget() const { return m_positionTarget; }
		void PositionTarget(vec3 val) { m_positionTarget = val; }
		vec3 PositionSpeed() const { return m_positionSpeed; }
		void PositionSpeed(vec3 val) { m_positionSpeed = val; }
		vec3 PositionEase() const { return m_positionEase; }
		void PositionEase(vec3 val) { m_positionEase = val; }

		vec2 Rotation() const { return m_rotation; }
		void Rotation(vec2 val) { m_rotation = val; }
		vec2 RotationTarget() const { return m_rotationTarget; }
		void RotationTarget(vec2 val) { m_rotationTarget = val; }
		vec2 RotationSpeed() const { return m_rotationSpeed; }
		void RotationSpeed(vec2 val) { m_rotationSpeed = val; }
		vec2 RotationEase() const { return m_rotationEase; }
		void RotationEase(vec2 val) { m_rotationEase = val; }

	private:
		void ProcessInput();
		void ApplyEase();
		void UpdateMatrix();

		float m_dolly;
		float m_dollyTarget;
		float m_dollySpeed;
		float m_dollyEase;
		float m_dollyMin;
		float m_dollyMax;

		vec3 m_position;
		vec3 m_positionTarget;
		vec3 m_positionSpeed;
		vec3 m_positionEase;

		vec2 m_rotation;
		vec2 m_rotationTarget;
		vec2 m_rotationSpeed;
		vec2 m_rotationEase;

		mat4 m_matrix;
		vec2 m_storedRotation;
		vec2 m_mouseDownPos;
		bool m_mouseIsDown;

		DEFINE_EVENT_HANDLER_1(OrbitCamera, OnMouseButtonPress, int, _button);
		DEFINE_EVENT_HANDLER_1(OrbitCamera, OnMouseButtonRelease, int, _button);
	};
}