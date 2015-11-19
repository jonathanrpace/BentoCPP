#include "TerrainSimulationProcess.h"

#include <utility>

#include <imgui.h>

namespace bento
{
	//////////////////////////////////////////////////////////////////////////
	// UpdateFluidFluxFrag
	//////////////////////////////////////////////////////////////////////////

	UpdateFluidFluxFrag::UpdateFluidFluxFrag() : ShaderStageBase("shaders/UpdateFluidFlux.frag") {}

	//////////////////////////////////////////////////////////////////////////
	// UpdateFluidHeightFrag
	//////////////////////////////////////////////////////////////////////////

	UpdateFluidHeightFrag::UpdateFluidHeightFrag() : ShaderStageBase("shaders/UpdateFluidHeight.frag") {}

	//////////////////////////////////////////////////////////////////////////
	// UpdateFluidVelocityFrag
	//////////////////////////////////////////////////////////////////////////

	UpdateFluidVelocityFrag::UpdateFluidVelocityFrag() : ShaderStageBase("shaders/UpdateFluidVelocity.frag") {}

	//////////////////////////////////////////////////////////////////////////
	// DiffuseHeightFrag
	//////////////////////////////////////////////////////////////////////////

	DiffuseHeightFrag::DiffuseHeightFrag() : ShaderStageBase("shaders/DiffuseHeight.frag") {}

	//////////////////////////////////////////////////////////////////////////
	// TerrainSimulationPass
	//////////////////////////////////////////////////////////////////////////
	
	TerrainSimulationProcess::TerrainSimulationProcess(std::string _name)
		: Process(_name, typeid(TerrainSimulationProcess))
		, m_updateFluxShader()
		, m_updateHeightShader()
		, m_updateVelocityShader()
		, m_diffuseHeightShader()
		, m_screenQuadGeom()
		, m_renderTargetByNodeMap()
		, m_switch(false)
	{
		m_nodeGroup.NodeAdded += OnNodeAddedDelegate;
		m_nodeGroup.NodeRemoved += OnNodeRemovedDelegate;
	}

	TerrainSimulationProcess::~TerrainSimulationProcess()
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

	void TerrainSimulationProcess::Advance(double _dt)
	{
		m_screenQuadGeom.Bind();

		for (auto node : m_nodeGroup.Nodes())
		{
			RenderTargetBase* renderTarget = m_renderTargetByNodeMap[node];
			AdvanceTerrainSim(*(node->geom), *(node->material), *renderTarget);
		}
	}

	void TerrainSimulationProcess::AddUIElements()
	{
		ImGui::SliderFloat("Viscosity", &m_viscosity, 0.0f, 0.5f);
		ImGui::SliderFloat("Elasticity", &m_elasticity, 0.0f, 0.5f);
		ImGui::SliderFloat("ScrollSpeed", &m_textureScrollSpeed, 0.0f, 0.5f);
		ImGui::SliderFloat("SmoothingStrength", &m_smoothingStrength, 0.0f, 0.45f);
		ImGui::SliderFloat("HeatViscosityPower", &m_heatViscosityPower, 0.0f, 2.0f);
		ImGui::SliderFloat("CoolingSpeed", &m_coolingSpeed, 0.0f, 0.01f, "%.5f");
		ImGui::SliderFloat("HeatViscosity", &m_heatViscosty, 0.0f, 10.0f);
		ImGui::SliderFloat("HeatDissipation", &m_heatDissipation, 0.0f, 0.45f);
		ImGui::SliderFloat("VelocityScalar", &m_velocityScalar, 0.0f, 200.0f);
	}

	//////////////////////////////////////////////////////////////////////////
	// PRIVATE
	//////////////////////////////////////////////////////////////////////////

	void TerrainSimulationProcess::AdvanceTerrainSim
	(
		TerrainGeometry & _geom, 
		TerrainMaterial & _material, 
		RenderTargetBase & _renderTarget
	)
	{
		GL_CHECK(glViewport(0, 0, _geom.NumVerticesPerDimension(), _geom.NumVerticesPerDimension()));

		vec2 normalisedMousePos = m_scene->GetInputManager()->GetMousePosition();
		normalisedMousePos /= m_scene->GetWindow()->GetWindowSize();
		float mouseStrength = (m_scene->GetInputManager()->IsMouseDown(1) ? 1.0f : 0.0f) * m_mouseStrength;

		vec2 cellSize = vec2(_geom.Size() / (float)_geom.NumVerticesPerDimension());

		// Update flux
		{
			static GLenum fluxDrawBufferA[] = { GL_COLOR_ATTACHMENT2 };
			static GLenum fluxDrawBufferB[] = { GL_COLOR_ATTACHMENT3 };

			if (!m_switch)
				_renderTarget.SetDrawBuffers(fluxDrawBufferB, sizeof(fluxDrawBufferB) / sizeof(fluxDrawBufferB[0]));
			else
				_renderTarget.SetDrawBuffers(fluxDrawBufferA, sizeof(fluxDrawBufferA) / sizeof(fluxDrawBufferA[0]));

			m_updateFluxShader.BindPerPass();
			auto fragShader = m_updateFluxShader.FragmentShader();
			fragShader.SetTexture("s_heightData", &_geom.HeightDataRead());
			fragShader.SetTexture("s_fluxData", &_geom.FluxDataRead());
			fragShader.SetUniform("u_elasticity", m_elasticity);
			fragShader.SetUniform("u_mousePos", normalisedMousePos);
			fragShader.SetUniform("u_mouseStrength", mouseStrength);
			fragShader.SetUniform("u_mouseRadius", m_mouseRadius);
			m_screenQuadGeom.Draw();

			_geom.SwapFluxData();
		}
		
		
		// Update height
		{
			static GLenum heightDrawBufferA[] = { GL_COLOR_ATTACHMENT0 };
			static GLenum heightDrawBufferB[] = { GL_COLOR_ATTACHMENT1 };

			if (!m_switch)
				_renderTarget.SetDrawBuffers(heightDrawBufferB, sizeof(heightDrawBufferB) / sizeof(heightDrawBufferB[0]));
			else
				_renderTarget.SetDrawBuffers(heightDrawBufferA, sizeof(heightDrawBufferA) / sizeof(heightDrawBufferA[0]));
			
			m_updateHeightShader.BindPerPass();
			auto fragShader = m_updateHeightShader.FragmentShader();
			fragShader.SetTexture("s_heightData", &_geom.HeightDataRead());
			fragShader.SetTexture("s_fluxData", &_geom.FluxDataRead());
			fragShader.SetTexture("s_velocityData", &_geom.VelocityData());
			fragShader.SetUniform("u_mousePos", normalisedMousePos);
			fragShader.SetUniform("u_mouseStrength", mouseStrength);
			fragShader.SetUniform("u_mouseRadius", m_mouseRadius);
			fragShader.SetUniform("u_viscocity", m_viscosity);
			fragShader.SetUniform("u_heatViscosityPower", m_heatViscosityPower);
			fragShader.SetUniform("u_coolingSpeed", m_coolingSpeed);
			fragShader.SetUniform("u_meltPower", m_meltPower);
			fragShader.SetUniform("u_condensePower", m_condensePower);
			m_screenQuadGeom.Draw();

			_geom.SwapHeightData();
		}

		// Update velocity
		{
			static GLenum velocityDrawBuffersA[] = { GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT7 };
			static GLenum velocityDrawBuffersB[] = { GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7 };

			if (!m_switch)
				_renderTarget.SetDrawBuffers(velocityDrawBuffersA, sizeof(velocityDrawBuffersA) / sizeof(velocityDrawBuffersA[0]));
			else
				_renderTarget.SetDrawBuffers(velocityDrawBuffersB, sizeof(velocityDrawBuffersB) / sizeof(velocityDrawBuffersB[0]));

			m_updateVelocityShader.BindPerPass();
			auto fragShader = m_updateVelocityShader.FragmentShader();
			fragShader.SetTexture("s_heightDataOld", &_geom.HeightDataRead());
			fragShader.SetTexture("s_heightDataNew", &_geom.HeightDataWrite());
			fragShader.SetTexture("s_mappingData", &_geom.MappingDataRead());
			fragShader.SetTexture("s_diffuseMap", &_material.SomeTexture);

			fragShader.SetUniform("u_mousePos", normalisedMousePos);
			fragShader.SetUniform("u_mouseStrength", mouseStrength);
			fragShader.SetUniform("u_mouseRadius", m_mouseRadius);

			fragShader.SetUniform("u_textureScrollSpeed", m_textureScrollSpeed);
			fragShader.SetUniform("u_velocityScalar", m_velocityScalar);
			fragShader.SetUniform("u_cellSize", cellSize);

			fragShader.SetUniform("u_mapHeightOffset", _material.MapHeightOffset);
			m_screenQuadGeom.Draw();

			_geom.SwapMappingData();
		}

		// Diffuse height
		{
			static GLenum heightDrawBufferA[] = { GL_COLOR_ATTACHMENT0 };
			static GLenum heightDrawBufferB[] = { GL_COLOR_ATTACHMENT1 };

			m_diffuseHeightShader.BindPerPass();
			auto fragShader = m_diffuseHeightShader.FragmentShader();

			fragShader.SetUniform("u_strength", m_smoothingStrength);
			fragShader.SetUniform("u_heatDissipation", m_heatDissipation);
			fragShader.SetUniform("u_heatViscosity", m_heatViscosty);
			fragShader.SetTexture("s_velocityData", &_geom.VelocityData());

			if (&_geom.HeightDataRead() == &_geom.HeightDataA())	
				_renderTarget.SetDrawBuffers(heightDrawBufferB, sizeof(heightDrawBufferB) / sizeof(heightDrawBufferB[0]));
			else			
				_renderTarget.SetDrawBuffers(heightDrawBufferA, sizeof(heightDrawBufferA) / sizeof(heightDrawBufferA[0]));

			fragShader.SetTexture("s_heightData", &_geom.HeightDataRead());
			fragShader.SetUniform("u_axis", vec2(1.0f, 0.0f));
			m_screenQuadGeom.Draw();
			_geom.SwapHeightData();

			if (&_geom.HeightDataRead() == &_geom.HeightDataA())
				_renderTarget.SetDrawBuffers(heightDrawBufferB, sizeof(heightDrawBufferB) / sizeof(heightDrawBufferB[0]));
			else
				_renderTarget.SetDrawBuffers(heightDrawBufferA, sizeof(heightDrawBufferA) / sizeof(heightDrawBufferA[0]));
			fragShader.SetTexture("s_heightData", &_geom.HeightDataRead());
			fragShader.SetUniform("u_axis", vec2(0.0f, 1.0f));
			m_screenQuadGeom.Draw();
			_geom.SwapHeightData();
		}

		m_switch = !m_switch;
	}

	void TerrainSimulationProcess::OnNodeAdded(const TerrainSimPassNode & _node)
	{
		RenderTargetBase* renderTarget = new RenderTargetBase
		(
			_node.geom->NumVerticesPerDimension(),
			_node.geom->NumVerticesPerDimension(),
			false, false, GL_RGBA16F
		);

		renderTarget->AttachTexture(GL_COLOR_ATTACHMENT0, &_node.geom->HeightDataA());
		renderTarget->AttachTexture(GL_COLOR_ATTACHMENT1, &_node.geom->HeightDataB());
		renderTarget->AttachTexture(GL_COLOR_ATTACHMENT2, &_node.geom->FluxDataA());
		renderTarget->AttachTexture(GL_COLOR_ATTACHMENT3, &_node.geom->FluxDataB());
		renderTarget->AttachTexture(GL_COLOR_ATTACHMENT4, &_node.geom->VelocityData());
		renderTarget->AttachTexture(GL_COLOR_ATTACHMENT5, &_node.geom->MappingDataA());
		renderTarget->AttachTexture(GL_COLOR_ATTACHMENT6, &_node.geom->MappingDataB());
		renderTarget->AttachTexture(GL_COLOR_ATTACHMENT7, &_node.geom->NormalData());

		m_renderTargetByNodeMap.insert(std::make_pair(&_node, renderTarget));
	}

	void TerrainSimulationProcess::OnNodeRemoved(const TerrainSimPassNode & _node)
	{
		RenderTargetBase* renderTarget = m_renderTargetByNodeMap[&_node];
		delete renderTarget;
		m_renderTargetByNodeMap.erase(m_renderTargetByNodeMap.find(&_node));
	}
	
}
