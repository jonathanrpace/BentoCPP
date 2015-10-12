#include "OrbitCamera.h"

#include <Kaiga/Components/Transform.h>
#include <Kaiga/Renderers/DefaultRenderer.h>
#include <Ramen/Core/IInputManager.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Kaiga
{
	//////////////////////////////////////////////////////////////////////////
	// PUBLIC
	//////////////////////////////////////////////////////////////////////////

	OrbitCamera::OrbitCamera() :
		m_mouseIsDown(false),
		m_scene(nullptr),
		m_dolly(2.0f),
		m_dollyTarget(2.0f),
		m_dollySpeed(0.1f),
		m_dollyEase(0.1f),
		m_dollyMin(0.1f),
		m_dollyMax(5.0f),
		m_position(0.0f, 0.0f, 0.0f),
		m_positionTarget(0.0f, 0.0f, 0.0f),
		m_positionSpeed(0.005f, 0.005f, 0.005f),
		m_positionEase(0.2f, 0.2f, 0.2f),
		m_rotation(0.0f, 0.0f),
		m_rotationEase(0.2f, 0.2f),
		m_rotationSpeed(3.0f, 3.0f)
	{

	}

	const std::type_info & OrbitCamera::typeInfo()
	{
		return typeid(OrbitCamera);
	}

	void OrbitCamera::BindToScene(Scene * const _scene)
	{
		m_scene = _scene;

		Ramen::IInputManager* inputManager = m_scene->GetInputManager();
		inputManager->OnMouseButtonPress += OnMouseButtonPressDelegate;
		inputManager->OnMouseButtonRelease += OnMouseButtonReleaseDelegate;
	}

	void OrbitCamera::UnbindFromScene(Scene * const _scene)
	{
		Ramen::IInputManager* inputManager = m_scene->GetInputManager();
		inputManager->OnMouseButtonPress -= OnMouseButtonPressDelegate;
		inputManager->OnMouseButtonRelease -= OnMouseButtonReleaseDelegate;

		m_scene = nullptr;
	}

	void OrbitCamera::Update(double dt)
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
		if (m_mouseIsDown)
		{
			vec2 mousePos = m_scene->GetInputManager()->GetMousePosition();
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
		Ramen::Entity camera = renderer->GetCamera();
		auto cameraTransform = m_scene->GetComponentForEntity<Transform>(camera);
		cameraTransform->matrix = m_matrix;
	}

	void OrbitCamera::OnMouseButtonPress(int _button)
	{
		if (_button == 0)
		{
			m_mouseIsDown = true;
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