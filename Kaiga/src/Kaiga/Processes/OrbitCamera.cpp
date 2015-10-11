#include "OrbitCamera.h"

#include <Ramen/Core/IInputManager.h>

//////////////////////////////////////////////////////////////////////////
// PUBLIC
//////////////////////////////////////////////////////////////////////////

Kaiga::OrbitCamera::OrbitCamera() :
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

void Kaiga::OrbitCamera::BindToScene(Scene * const _scene)
{
	m_scene = _scene;

	Ramen::IInputManager* inputManager = m_scene->GetInputManager();
	inputManager->OnMouseButtonPress += OnMouseButtonPressDelegate;
	inputManager->OnMouseButtonRelease += OnMouseButtonReleaseDelegate;
}

void Kaiga::OrbitCamera::UnbindFromScene(Scene * const _scene)
{
	Ramen::IInputManager* inputManager = m_scene->GetInputManager();
	inputManager->OnMouseButtonPress -= OnMouseButtonPressDelegate;
	inputManager->OnMouseButtonRelease -= OnMouseButtonReleaseDelegate;

	m_scene = nullptr;
}

void Kaiga::OrbitCamera::Update(double dt)
{
	ProcessInput();
	ApplyEase();
	UpdateMatrix();
}

//////////////////////////////////////////////////////////////////////////
// PRIVATE
//////////////////////////////////////////////////////////////////////////

void Kaiga::OrbitCamera::ProcessInput()
{
	
}

void Kaiga::OrbitCamera::ApplyEase()
{
}

void Kaiga::OrbitCamera::UpdateMatrix()
{
}

void Kaiga::OrbitCamera::OnMouseButtonPress(int _button)
{
	if (_button == 0)
	{
		m_mouseIsDown = true;
	}
}

void Kaiga::OrbitCamera::OnMouseButtonRelease(int _button)
{
	if (_button == 0)
	{
		m_mouseIsDown = false;
	}
}