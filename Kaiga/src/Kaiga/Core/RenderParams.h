#pragma once

#include <glm.h>
#include <Kaiga/Components/ILens.h>

namespace Kaiga
{
class RenderParams
{
public:
	static mat4 ModelMatrix()					{ return s_modelMatrix; }
	static mat3 NormalModelMatrix()				{ return s_normalModelMatrix; }
	static mat4 ViewMatrix()					{ return s_viewMatrix; }
	static mat3 NormalViewMatrix()				{ return s_normalViewMatrix; }
	static mat4 ProjectionMatrix()				{ return s_projectionMatrix; }
	static mat4 ModelViewMatrix()				{ return s_modelViewMatrix; }
	static mat3 NormalModelViewMatrix()			{ return s_normalModelViewMatrix; }
	static mat4 ViewProjetionMatrix()			{ return s_viewProjetionMatrix; }
	static mat4 ModelViewProjectionMatrix()		{ return s_modelViewProjectionMatrix; }

	static mat4 InvModelMatrix()				{ return s_invModelMatrix; }
	static mat4 InvNormalModelMatrix()			{ return s_invNormalModelMatrix; }
	static mat4 InvViewMatrix()					{ return s_invViewMatrix; }
	static mat3 InvNormalViewMatrix()			{ return s_invNormalViewMatrix; }
	static mat4 InvProjectionMatrix()			{ return s_invProjectionMatrix; }
	static mat4 InvModelViewMatrix()			{ return s_invModelViewMatrix; }
	static mat3 InvNormalModelViewMatrix()		{ return s_invNormalModelViewMatrix; }
	static mat4 InvViewProjetionMatrix()		{ return s_invViewProjetionMatrix; }
	static mat4 InvModelViewProjectionMatrix()	{ return s_invModelViewProjectionMatrix; }

	static mat4 PrevViewProjectionMatrix()		{ return s_prevViewProjectionMatrix; }
	static mat4 PrevInvViewProjectionMatrix()	{ return s_prevInvViewProjectionMatrix; }

	static void SetViewMatrices(mat4 _viewMatrix, ILens& _lens)
	{
		s_prevInvViewProjectionMatrix = s_viewProjetionMatrix;
		s_prevInvViewProjectionMatrix = s_invViewProjetionMatrix;

		s_viewMatrix = _viewMatrix;
		s_normalViewMatrix = transpose(mat3(_viewMatrix));
		s_projectionMatrix = _lens.GetMatrix();

		s_invViewMatrix = inverse(s_viewMatrix);
		s_invNormalViewMatrix = inverse(s_normalViewMatrix);
		s_invProjectionMatrix = inverse(s_projectionMatrix);
	}

	static void SetModelMatrix(mat4 _modelMatrix)
	{
		s_modelMatrix = _modelMatrix;
		s_modelViewMatrix = s_viewMatrix * s_modelMatrix;
		s_modelViewProjectionMatrix = s_projectionMatrix * s_modelViewMatrix;

		s_normalModelMatrix = transpose(mat3(s_modelMatrix));
		s_normalModelViewMatrix = transpose(mat3(s_modelViewMatrix));

		s_invNormalModelViewMatrix = inverse(s_normalModelViewMatrix);
	}

private:
	static int s_backBufferWidth;
	static int s_backBufferHeigth;
	static int s_lightTransportBufferWidth;
	static int s_lightTransportBufferHeight;

	static mat4 s_modelMatrix;
	static mat3 s_normalModelMatrix;
	static mat4 s_viewMatrix;
	static mat3 s_normalViewMatrix;
	static mat4 s_projectionMatrix;
	static mat4 s_modelViewMatrix;
	static mat3 s_normalModelViewMatrix;
	static mat4 s_viewProjetionMatrix;
	static mat4 s_modelViewProjectionMatrix;
		 
	static mat4 s_invModelMatrix;
	static mat4 s_invNormalModelMatrix;
	static mat4 s_invViewMatrix;
	static mat3 s_invNormalViewMatrix;
	static mat4 s_invProjectionMatrix;
	static mat4 s_invModelViewMatrix;
	static mat3 s_invNormalModelViewMatrix;
	static mat4 s_invViewProjetionMatrix;
	static mat4 s_invModelViewProjectionMatrix;
		 
	static mat4 s_prevViewProjectionMatrix;
	static mat4 s_prevInvViewProjectionMatrix;
};
}