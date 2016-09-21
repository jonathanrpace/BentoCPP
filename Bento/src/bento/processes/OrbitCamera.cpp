#include "OrbitCamera.h"

#include <imgui.h>

#include <bento/core/IInputManager.h>
#include <bento/Components/Transform.h>
#include <bento/render/DefaultRenderer.h>

namespace bento
{
	//////////////////////////////////////////////////////////////////////////
	// PUBLIC
	//////////////////////////////////////////////////////////////////////////

	OrbitCamera::OrbitCamera(std::string _name)
		: Process(_name, typeid(OrbitCamera))
		, m_mouseIsDown(false)
		, m_dolly(11.0f)
		, m_dollyTarget(11.0f)
		, m_dollySpeed(0.1f)
		, m_dollyEase(0.1f)
		, m_dollyMin(0.1f)
		, m_dollyMax(5.0f)
		, m_position(0.0f, 0.0f, 0.0f)
		, m_positionTarget(0.0f, 0.0f, 0.0f)
		, m_positionSpeed(0.005f, 0.005f, 0.005f)
		, m_positionEase(0.2f, 0.2f, 0.2f)
		, m_rotation(0.0f, 0.0f)
		, m_rotationEase(0.2f, 0.2f)
		, m_rotationSpeed(3.0f, 3.0f)
	{

	}

	void OrbitCamera::BindToScene(Scene& _scene)
	{
		Process::BindToScene(_scene);

		IInputManager& inputManager = _scene.GetInputManager();
		inputManager.OnMouseButtonPress += OnMouseButtonPressDelegate;
		inputManager.OnMouseButtonRelease += OnMouseButtonReleaseDelegate;
	}

	void OrbitCamera::UnbindFromScene(Scene& _scene)
	{
		Process::UnbindFromScene(_scene);

		IInputManager& inputManager = _scene.GetInputManager();
		inputManager.OnMouseButtonPress -= OnMouseButtonPressDelegate;
		inputManager.OnMouseButtonRelease -= OnMouseButtonReleaseDelegate;
	}

	void OrbitCamera::Advance(double dt)
	{
		ProcessInput();
		ApplyEase();
		UpdateMatrix();
	}

	//////////////////////////////////////////////////////////////////////////
	// PRIVATE
	//////////////////////////////////////////////////////////////////////////

	void OrbitCamera::ProcessInput()
	{
		if (ImGui::IsAnyItemActive()) return;
		if (m_mouseIsDown)
		{
			vec2 mousePos = m_scene->GetInputManager().GetMousePosition();
			vec2 mouseDelta = mousePos - m_mouseDownPos;
			mouseDelta /= vec2(640.0f, 480.0f);

			m_rotationTarget = m_storedRotation + mouseDelta * m_rotationSpeed;
		}
	}

	void OrbitCamera::ApplyEase()
	{
		m_position += (m_positionTarget - m_position) * m_positionEase;
		m_rotation += (m_rotationTarget - m_rotation) * m_rotationEase;
		m_dollyTarget = glm::clamp(m_dollyTarget, m_dollyMin, m_dollyMax);
		m_dolly += (m_dollyTarget - m_dolly) * m_dollyEase;
	}

	void OrbitCamera::UpdateMatrix()
	{
		m_matrix = mat4();
		
		m_matrix = glm::translate(m_matrix, vec3(0.0f, 0.0f, -m_dolly));
		m_matrix = glm::rotate(m_matrix, m_rotation.y, vec3(1.0f, 0.0f, 0.0f));
		m_matrix = glm::rotate(m_matrix, m_rotation.x, vec3(0.0f, 1.0f, 0.0f));
		m_matrix = glm::translate(m_matrix, m_position);
		
		auto renderer = m_scene->GetProcess<DefaultRenderer>();
		if (renderer)
		{
			EntityPtr camera = renderer->GetCamera();
			auto cameraTransform = m_scene->GetComponentForEntity<Transform>(camera);
			cameraTransform->matrix = m_matrix;
		}
	}

	void OrbitCamera::OnMouseButtonPress(int _button)
	{
		if (ImGui::IsAnyItemHovered()) return;

		if (_button == 0)
		{
			m_mouseIsDown = true;
			m_mouseDownPos = m_scene->GetInputManager().GetMousePosition();
			m_storedRotation = m_rotationTarget;
		}
	}

	void OrbitCamera::OnMouseButtonRelease(int _button)
	{
		if (_button == 0)
		{
			m_mouseIsDown = false;
		}
	}
}