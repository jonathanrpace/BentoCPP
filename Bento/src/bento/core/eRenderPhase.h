#pragma once

namespace bento
{
	enum eRenderPhase
	{
		eRenderPhase_OffScreen,
		eRenderPhase_G,
		eRenderPhase_Forward,
		eRenderPhase_DirectLight,
		eRenderPhase_IndirectLight,
		eRenderPhase_PostLight,
		eRenderPhase_UI
	};
}