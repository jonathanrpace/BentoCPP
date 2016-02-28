#include "RenderParams.h"

namespace bento
{
	int RenderParams::s_backBufferWidth;
	int RenderParams::s_backBufferHeight;
	int RenderParams::s_lightTransportBufferWidth;
	int RenderParams::s_lightTransportBufferHeight;
	float RenderParams::s_lightTransportScalar = 0.5f;
	DeferredRenderTarget* RenderParams::DeferedRenderTarget;
	glm::mat4 RenderParams::s_modelMatrix;
	glm::mat3 RenderParams::s_normalModelMatrix;
	glm::mat4 RenderParams::s_viewMatrix;
	glm::mat3 RenderParams::s_normalViewMatrix;
	glm::mat4 RenderParams::s_projectionMatrix;
	glm::mat4 RenderParams::s_modelViewMatrix;
	glm::mat3 RenderParams::s_normalModelViewMatrix;
	glm::mat4 RenderParams::s_viewProjetionMatrix;
	glm::mat4 RenderParams::s_modelViewProjectionMatrix;
	glm::mat4 RenderParams::s_invModelMatrix;
	glm::mat4 RenderParams::s_invNormalModelMatrix;
	glm::mat4 RenderParams::s_invViewMatrix;
	glm::mat3 RenderParams::s_invNormalViewMatrix;
	glm::mat4 RenderParams::s_invProjectionMatrix;
	glm::mat4 RenderParams::s_invModelViewMatrix;
	glm::mat3 RenderParams::s_invNormalModelViewMatrix;
	glm::mat4 RenderParams::s_invViewProjetionMatrix;
	glm::mat4 RenderParams::s_invModelViewProjectionMatrix;
	glm::mat4 RenderParams::s_prevViewProjectionMatrix;
	glm::mat4 RenderParams::s_prevInvViewProjectionMatrix;
}