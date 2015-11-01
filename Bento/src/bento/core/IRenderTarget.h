#pragma once

namespace bento
{
	struct IRenderTarget
	{
		virtual void Bind() = 0;
	};
}