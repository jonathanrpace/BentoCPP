#include "TerrainSimulationPass.h"

#include <utility>

namespace bento
{
	TerrainSimFrag::TerrainSimFrag() : ShaderStageBase("shaders/TerrainSim.frag") {}

	void TerrainSimFrag::SetAxis(float _x, float _y)
	{
		SetUniform("u_axis", vec2(_x, _y));
	}

	void TerrainSimFrag::SetDataTextures(TextureSquare * _data0, TextureSquare * _data1, TextureSquare * _data2)
	{
		SetTexture("s_data0", _data0);
		SetTexture("s_data1", _data1);
		SetTexture("s_data2", _data2);
	}
	
	TerrainSimulationPass::TerrainSimulationPass(std::string _name)
		: NodeGroupRenderPassBase(_name)
		, m_shader()
		, m_screenQuadGeom()
		, m_renderTargetByNodeMap()
	{
		m_nodeGroup.NodeAdded += OnNodeAddedDelegate;
		m_nodeGroup.NodeRemoved += OnNodeRemovedDelegate;
	}

	TerrainSimulationPass::~TerrainSimulationPass()
	{
		m_nodeGroup.NodeAdded -= OnNodeAddedDelegate;
		m_nodeGroup.NodeRemoved -= OnNodeRemovedDelegate;

		for( auto iter : m_renderTargetByNodeMap)
		{
			RenderTargetBase* renderTarget = iter.second;
			delete renderTarget;
		}
		m_renderTargetByNodeMap.clear();
	}

	void TerrainSimulationPass::Render()
	{
		m_shader.BindPerPass();
		m_screenQuadGeom.Bind();

		for (auto node : m_nodeGroup.Nodes())
		{
			RenderTargetBase* renderTarget = m_renderTargetByNodeMap[node];

			AdvanceTerrainSim(*(node->geom), *(node->material), *renderTarget);
		}
	}

	RenderPhase TerrainSimulationPass::GetRenderPhase()
	{
		return RenderPhase::eRenderPhase_OffScreen;
	}

	//////////////////////////////////////////////////////////////////////////
	// PRIVATE
	//////////////////////////////////////////////////////////////////////////

	void TerrainSimulationPass::AdvanceTerrainSim(TerrainGeometry & _geom, TerrainMaterial & _material, RenderTargetBase & _renderTarget)
	{
		GL_CHECK(glViewport(0, 0, _geom.NumVerticesPerDimension(), _geom.NumVerticesPerDimension()));

		// Phase A 
		static GLenum phaseADrawBuffers[] = {
			GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2
		};
		_renderTarget.SetDrawBuffers(phaseADrawBuffers, sizeof(phaseADrawBuffers) / sizeof(phaseADrawBuffers[0]));

		m_shader.FragmentShader().SetAxis(1.0f, 0.0f);
		m_shader.FragmentShader().SetDataTextures(&_geom.Texture0B(), &_geom.Texture1B(), &_geom.Texture2B());

		m_screenQuadGeom.Draw();

		// Phase B
		static GLenum phaseBDrawBuffers[] = {
			GL_COLOR_ATTACHMENT3,
			GL_COLOR_ATTACHMENT4,
			GL_COLOR_ATTACHMENT5
		};
		_renderTarget.SetDrawBuffers(phaseBDrawBuffers, sizeof(phaseBDrawBuffers) / sizeof(phaseBDrawBuffers[0]));

		m_shader.FragmentShader().SetAxis(0.0f, 1.0f);
		m_shader.FragmentShader().SetDataTextures(&_geom.Texture0A(), &_geom.Texture1A(), &_geom.Texture2A());

		m_screenQuadGeom.Draw();
	}

	void TerrainSimulationPass::OnNodeAdded(const TerrainSimPassNode & _node)
	{
		RenderTargetBase* renderTarget = new RenderTargetBase
		(
			_node.geom->NumVerticesPerDimension(),
			_node.geom->NumVerticesPerDimension(),
			false, false, GL_RGBA16F
		);

		renderTarget->AttachTexture(GL_COLOR_ATTACHMENT0, &_node.geom->Texture0A());
		renderTarget->AttachTexture(GL_COLOR_ATTACHMENT1, &_node.geom->Texture1A());
		renderTarget->AttachTexture(GL_COLOR_ATTACHMENT2, &_node.geom->Texture2A());
		renderTarget->AttachTexture(GL_COLOR_ATTACHMENT3, &_node.geom->Texture0B());
		renderTarget->AttachTexture(GL_COLOR_ATTACHMENT4, &_node.geom->Texture1B());
		renderTarget->AttachTexture(GL_COLOR_ATTACHMENT5, &_node.geom->Texture2B());

		m_renderTargetByNodeMap.insert(std::make_pair(&_node, renderTarget));
	}

	void TerrainSimulationPass::OnNodeRemoved(const TerrainSimPassNode & _node)
	{
		RenderTargetBase* renderTarget = m_renderTargetByNodeMap[&_node];
		delete renderTarget;
		m_renderTargetByNodeMap.erase(m_renderTargetByNodeMap.find(&_node));
	}
	
}
