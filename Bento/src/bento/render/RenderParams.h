#pragma once

#include <bento.h>
#include <bento/core/ILens.h>
#include <bento/components/Transform.h>
#include <glm/gtx/matrix_decompose.hpp>

namespace bento
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

		static std::shared_ptr<Transform> CameraTransform() { return s_cameraTransform; }
		static vec3 CameraPosition() { return s_cameraPos; }
		static vec3 CameraForward() { return s_cameraForward; }
		static vec3 CameraUp() { return s_cameraUp; }
		static vec3 CameraRight() { return s_cameraRight; }

		static ivec2 BackBufferSize() { return ivec2( s_backBufferWidth, s_backBufferHeight ); }
		static int BackBufferWidth() { return s_backBufferWidth; }
		static int BackBufferHeight() { return s_backBufferHeight; }
		static vec4 Viewport() { return s_viewport; }

		static void SetViewMatrices(mat4 _viewMatrix, ILens& _lens)
		{
			s_prevViewProjectionMatrix = s_viewProjetionMatrix;
			s_prevInvViewProjectionMatrix = s_invViewProjetionMatrix;

			s_viewMatrix = _viewMatrix;
			s_normalViewMatrix = mat3(_viewMatrix);
			s_projectionMatrix = _lens.GetMatrix();

			s_viewProjetionMatrix = s_projectionMatrix * s_viewMatrix;

			s_invViewMatrix = inverse(s_viewMatrix);
			s_invNormalViewMatrix = inverse(s_normalViewMatrix);
			s_invProjectionMatrix = inverse(s_projectionMatrix);
			s_invViewProjetionMatrix = inverse(s_viewProjetionMatrix);

			s_cameraPos = vec3(s_invViewMatrix[3]);

			s_cameraForward = vec3(0.0,0.0,-1.0) * s_normalViewMatrix;
			s_cameraRight = vec3(1.0,0.0,0.0) * s_normalViewMatrix;
			s_cameraUp = vec3(0.0,1.0,0.0) * s_normalViewMatrix;
		}

		static void SetModelMatrix(mat4 _modelMatrix)
		{
			s_modelMatrix = _modelMatrix;
			s_modelViewMatrix = s_viewMatrix * s_modelMatrix;
			s_modelViewProjectionMatrix = s_projectionMatrix * s_modelViewMatrix;

			s_normalModelMatrix = transpose(mat3(s_modelMatrix));
			s_normalModelViewMatrix = transpose(mat3(s_modelViewMatrix));

			s_invNormalModelViewMatrix = inverse(s_normalModelViewMatrix);

			s_invModelViewProjectionMatrix = inverse(s_modelViewProjectionMatrix);
		}

		static void SetBackBufferDimensions(int _width, int _height)
		{
			s_backBufferWidth = _width;
			s_backBufferHeight = _height;

			s_viewport = vec4(0.0f, 0.0f, (float)_width, (float)_height);
		}

		static void SetCameraTransform(std::shared_ptr<Transform> _cameraTransform)
		{
			s_cameraTransform = _cameraTransform;
		}

	private:
		static int s_backBufferWidth;
		static int s_backBufferHeight;
		static vec4 s_viewport;

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

		static std::shared_ptr<Transform> s_cameraTransform;
		static vec3 s_cameraPos;
		static vec3 s_cameraForward;
		static vec3 s_cameraRight;
		static vec3 s_cameraUp;
	};
}