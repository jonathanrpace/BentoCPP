#pragma once

#include <bento/components/textures/RectangleTexture.h>
#include <bento/core/ShaderStageBase.h>

namespace bento
{
	struct RectBlurTap5Frag : ShaderStageBase
	{
		RectBlurTap5Frag() 
			: ShaderStageBase("shaders/RectBlurTap5.frag") 
		{

		}

		void SetSourceTexture(RectangleTexture& _texture)
		{
			SetTexture("s_tex", _texture);
		}
	};
}