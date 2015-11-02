#pragma once

namespace bento
{
	enum RenderPhase
	{
		eRenderPhase_OffScreen,
		eRenderPhase_G,
		eRenderPhase_DirectLight,
		eRenderPhase_IndirectLight,
		eRenderPhase_PostLight,
		eRenderPhase_UI
	};
}