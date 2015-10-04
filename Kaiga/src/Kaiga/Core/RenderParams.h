#pragma once

#include <glm/glm.hpp>
#include <Kaiga/Components/ILens.h>

namespace Kaiga
{
class RenderParams
{
public:
	glm::mat4 ModelMatrix() const { return s_modelMatrix; }
	glm::mat3 NormalModelMatrix() const { return s_normalModelMatrix; }
	glm::mat4 ViewMatrix() const { return s_viewMatrix; }
	glm::mat3 NormalViewMatrix() const { return s_normalViewMatrix; }
	glm::mat4 ProjectionMatrix() const { return s_projectionMatrix; }
	glm::mat4 ModelViewMatrix() const { return s_modelViewMatrix; }
	glm::mat3 NormalModelViewMatrix() const { return s_normalModelViewMatrix; }
	glm::mat4 ViewProjetionMatrix() const { return s_viewProjetionMatrix; }
	glm::mat4 ModelViewProjectionMatrix() const { return s_modelViewProjectionMatrix; }

	glm::mat4 InvModelMatrix() const { return s_invModelMatrix; }
	glm::mat4 InvNormalModelMatrix() const { return s_invNormalModelMatrix; }
	glm::mat4 InvViewMatrix() const { return s_invViewMatrix; }
	glm::mat3 InvNormalViewMatrix() const { return s_invNormalViewMatrix; }
	glm::mat4 InvProjectionMatrix() const { return s_invProjectionMatrix; }
	glm::mat4 InvModelViewMatrix() const { return s_invModelViewMatrix; }
	glm::mat3 InvNormalModelViewMatrix() const { return s_invNormalModelViewMatrix; }
	glm::mat4 InvViewProjetionMatrix() const { return s_invViewProjetionMatrix; }
	glm::mat4 InvModelViewProjectionMatrix() const { return s_invModelViewProjectionMatrix; }

	glm::mat4 PrevViewProjectionMatrix() const { return s_prevViewProjectionMatrix; }
	glm::mat4 PrevInvViewProjectionMatrix() const { return s_prevInvViewProjectionMatrix; }


	static void SetViewMatrices(glm::mat4 _viewMatrix, ILens& _lens)
	{
		s_prevInvViewProjectionMatrix = s_viewProjetionMatrix;
		s_prevInvViewProjectionMatrix = s_invViewProjetionMatrix;

		s_viewMatrix = _viewMatrix;
		s_normalViewMatrix = glm::transpose(glm::mat3(_viewMatrix));
		s_projectionMatrix = _lens.GetMatrix();

		s_invViewMatrix = glm::inverse(s_viewMatrix);
		s_invNormalViewMatrix = glm::inverse(s_normalViewMatrix);
		s_invProjectionMatrix = glm::inverse(s_projectionMatrix);
	}

	static void SetModelMatrix(glm::mat4 _modelMatrix)
	{
		s_modelMatrix = _modelMatrix;
		s_modelViewMatrix = s_modelMatrix * s_viewMatrix;
		s_modelViewProjectionMatrix = s_modelViewMatrix * s_projectionMatrix;

		s_normalModelMatrix = glm::transpose(glm::mat3(s_modelMatrix));
		s_normalModelViewMatrix = glm::transpose(glm::mat3(s_modelViewMatrix));

		s_invNormalModelViewMatrix = glm::inverse(s_normalModelViewMatrix);
	}

private:
	static int s_backBufferWidth;
	static int s_backBufferHeigth;
	static int s_lightTransportBufferWidth;
	static int s_lightTransportBufferHeight;

	static glm::mat4 s_modelMatrix;
	static glm::mat3 s_normalModelMatrix;
	static glm::mat4 s_viewMatrix;
	static glm::mat3 s_normalViewMatrix;
	static glm::mat4 s_projectionMatrix;
	static glm::mat4 s_modelViewMatrix;
	static glm::mat3 s_normalModelViewMatrix;
	static glm::mat4 s_viewProjetionMatrix;
	static glm::mat4 s_modelViewProjectionMatrix;
		 
	static glm::mat4 s_invModelMatrix;
	static glm::mat4 s_invNormalModelMatrix;
	static glm::mat4 s_invViewMatrix;
	static glm::mat3 s_invNormalViewMatrix;
	static glm::mat4 s_invProjectionMatrix;
	static glm::mat4 s_invModelViewMatrix;
	static glm::mat3 s_invNormalModelViewMatrix;
	static glm::mat4 s_invViewProjetionMatrix;
	static glm::mat4 s_invModelViewProjectionMatrix;
		 
	static glm::mat4 s_prevViewProjectionMatrix;
	static glm::mat4 s_prevInvViewProjectionMatrix;
};
}