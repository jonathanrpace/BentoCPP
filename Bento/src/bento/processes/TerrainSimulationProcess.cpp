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
		ImGui::SliderFloat("MouseRadius", &m_mouseRadius, 0.01f, 0.5f);
		ImGui::SliderFloat("MouseVolumeStrength", &m_mouseVolumeStrength, 0.00f, 0.01f, "%.5f");
		ImGui::SliderFloat("MouseHeatStrength", &m_mouseHeatStrength, 0.00f, 0.4f, "%.2f");
		ImGui::SliderFloat("ViscosityMin", &m_viscosityMin, 0.01f, 0.5f);
		ImGui::SliderFloat("ViscosityMax", &m_viscosityMax, 0.01f, 0.5f);
		ImGui::SliderFloat("Elasticity", &m_elasticity, 0.0f, 0.5f);
		ImGui::SliderFloat("FluxDamping", &m_fluxDamping, 0.9f, 1.0f);
		ImGui::SliderFloat("HeatViscosityPower", &m_heatViscosityPower, 0.1f, 10.0f);
		ImGui::SliderFloat("HeatViscosityBias", &m_heatViscosityBias, 0.0f, 2.0f);
		ImGui::SliderFloat("HeatAdvectSpeed", &m_heatAdvectSpeed, 0.0f, 5.0f);
		ImGui::SliderFloat("VelocityScalar", &m_velocityScalar, 0.0f, 20.0f);
		ImGui::SliderFloat("ScrollSpeed", &m_textureScrollSpeed, 0.0f, 0.5f);
		ImGui::SliderFloat("CoolingSpeedMin", &m_coolingSpeedMin, 0.0f, 0.02f, "%.5f");
		ImGui::SliderFloat("CoolingSpeedMax", &m_coolingSpeedMax, 0.0f, 0.02f, "%.5f");
		ImGui::SliderFloat("MeltSpeed", &m_meltSpeed, 0.0f, 0.01f, "%.5f");
		ImGui::SliderFloat("CondenseSpeed", &m_condenseSpeed, 0.0f, 0.01f, "%.5f");
		ImGui::SliderFloat("HeatSmoothing", &m_heatDissipation, 0.0f, 0.5f, "%.4f");

		ImGui::SliderFloat("UVTension", &m_uvTension, 0.0f, 1.0f, "%.3f");
		ImGui::SliderFloat("UVTensionThreshold", &m_uvTensionThreshold, 0.0f, 50.0f, "%.1f");
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
		float mouseVolumeStrength = (m_scene->GetInputManager()->IsMouseDown(1) ? 1.0f : 0.0f) * m_mouseVolumeStrength;
		float mouseHeatStrength = (m_scene->GetInputManager()->IsMouseDown(1) ? 1.0f : 0.0f) * m_mouseHeatStrength;

		vec2 cellSize = vec2(_geom.Size() / (float)_geom.NumVerticesPerDimension());

		// Update flux
		{
			static GLenum fluxDrawBufferA[] = { FLUX_DATA_A };
			static GLenum fluxDrawBufferB[] = { FLUX_DATA_B };

			if (!m_switch)
				_renderTarget.SetDrawBuffers(fluxDrawBufferB, sizeof(fluxDrawBufferB) / sizeof(fluxDrawBufferB[0]));
			else
				_renderTarget.SetDrawBuffers(fluxDrawBufferA, sizeof(fluxDrawBufferA) / sizeof(fluxDrawBufferA[0]));

			m_updateFluxShader.BindPerPass();
			auto fragShader = m_updateFluxShader.FragmentShader();
			fragShader.SetTexture("s_heightData", &_geom.HeightDataRead());
			fragShader.SetTexture("s_fluxData", &_geom.FluxDataRead());

			fragShader.SetUniform("u_elasticity", m_elasticity);
			fragShader.SetUniform("u_fluxDamping", m_fluxDamping);

			fragShader.SetUniform("u_viscosityMin", m_viscosityMin);
			fragShader.SetUniform("u_viscosityMax", m_viscosityMax);
			fragShader.SetUniform("u_heatViscosityPower", m_heatViscosityPower);
			fragShader.SetUniform("u_heatViscosityBias", m_heatViscosityBias);

			m_screenQuadGeom.Draw();

			_geom.SwapFluxData();
		}
		
		
		// Update height
		{
			static GLenum heightDrawBufferA[] = { HEIGHT_DATA_A };
			static GLenum heightDrawBufferB[] = { HEIGHT_DATA_B };

			if (!m_switch)
				_renderTarget.SetDrawBuffers(heightDrawBufferB, sizeof(heightDrawBufferB) / sizeof(heightDrawBufferB[0]));
			else
				_renderTarget.SetDrawBuffers(heightDrawBufferA, sizeof(heightDrawBufferA) / sizeof(heightDrawBufferA[0]));
			
			m_updateHeightShader.BindPerPass();
			auto fragShader = m_updateHeightShader.FragmentShader();
			fragShader.SetTexture("s_heightData", &_geom.HeightDataRead());
			fragShader.SetTexture("s_fluxData", &_geom.FluxDataRead());
			//fragShader.SetTexture("s_velocityData", &_geom.VelocityData());
			fragShader.SetTexture("s_mappingData", &_geom.MappingDataRead());
			fragShader.SetTexture("s_diffuseMap", &_material.SomeTexture);

			fragShader.SetUniform("u_mousePos", normalisedMousePos);
			fragShader.SetUniform("u_mouseVolumeStrength", mouseVolumeStrength);
			fragShader.SetUniform("u_mouseHeatStrength", mouseHeatStrength);
			fragShader.SetUniform("u_mouseRadius", m_mouseRadius);
			
			fragShader.SetUniform("u_heatAdvectSpeed", m_heatAdvectSpeed);

			fragShader.SetUniform("u_viscosityMin", m_viscosityMin);
			fragShader.SetUniform("u_viscosityMax", m_viscosityMax);
			fragShader.SetUniform("u_heatViscosityPower", m_heatViscosityPower);
			fragShader.SetUniform("u_heatViscosityBias", m_heatViscosityBias);

			fragShader.SetUniform("u_coolingSpeedMin", m_coolingSpeedMin);
			fragShader.SetUniform("u_coolingSpeedMax", m_coolingSpeedMax);
			fragShader.SetUniform("u_meltSpeed", m_meltSpeed);
			fragShader.SetUniform("u_condenseSpeed", m_condenseSpeed);

			m_screenQuadGeom.Draw();

			_geom.SwapHeightData();
		}

		// Update velocity
		{
			static GLenum velocityDrawBuffersA[] = { VELOCITY_DATA, MAPPING_DATA_A, NORMAL_DATA };
			static GLenum velocityDrawBuffersB[] = { VELOCITY_DATA, MAPPING_DATA_B, NORMAL_DATA };

			if (!m_switch)
				_renderTarget.SetDrawBuffers(velocityDrawBuffersA, sizeof(velocityDrawBuffersA) / sizeof(velocityDrawBuffersA[0]));
			else
				_renderTarget.SetDrawBuffers(velocityDrawBuffersB, sizeof(velocityDrawBuffersB) / sizeof(velocityDrawBuffersB[0]));

			m_updateVelocityShader.BindPerPass();
			auto fragShader = m_updateVelocityShader.FragmentShader();
			fragShader.SetTexture("s_heightData", &_geom.HeightDataRead());
			fragShader.SetTexture("s_mappingData", &_geom.MappingDataRead());
			fragShader.SetTexture("s_diffuseMap", &_material.SomeTexture);
			fragShader.SetTexture("s_fluxData", &_geom.FluxDataRead());

			fragShader.SetUniform("u_mousePos", normalisedMousePos);
			fragShader.SetUniform("u_mouseVolumeStrength", mouseVolumeStrength);
			fragShader.SetUniform("u_mouseRadius", m_mouseRadius);

			fragShader.SetUniform("u_viscosityMin", m_viscosityMin);
			fragShader.SetUniform("u_viscosityMax", m_viscosityMax);
			fragShader.SetUniform("u_heatViscosityPower", m_heatViscosityPower);
			fragShader.SetUniform("u_heatViscosityBias", m_heatViscosityBias);

			fragShader.SetUniform("u_textureScrollSpeed", m_textureScrollSpeed);
			fragShader.SetUniform("u_velocityScalar", m_velocityScalar);
			fragShader.SetUniform("u_cellSize", cellSize);

			fragShader.SetUniform("u_mapHeightOffset", _material.MapHeightOffset);

			fragShader.SetUniform("u_uvTension", m_uvTension);
			fragShader.SetUniform("u_uvTensionThreshold", m_uvTensionThreshold);

			m_screenQuadGeom.Draw();

			_geom.SwapMappingData();
		}
		
		// Diffuse height
		{
			static GLenum heightDrawBufferA[] = { HEIGHT_DATA_A };
			static GLenum heightDrawBufferB[] = { HEIGHT_DATA_B };

			m_diffuseHeightShader.BindPerPass();
			auto fragShader = m_diffuseHeightShader.FragmentShader();

			fragShader.SetUniform("u_heatDissipation", m_heatDissipation);
			//fragShader.SetUniform("u_heatViscosityBias", m_heatViscosityBias);
			//fragShader.SetTexture("s_velocityData", &_geom.VelocityData());

			if (&_geom.HeightDataRead() == &_geom.HeightDataA())	
				_renderTarget.SetDrawBuffers(heightDrawBufferB, sizeof(heightDrawBufferB) / sizeof(heightDrawBufferB[0]));
			else			
				_renderTarget.SetDrawBuffers(heightDrawBufferA, sizeof(heightDrawBufferA) / sizeof(heightDrawBufferA[0]));

			//_geom.HeightDataRead().MagFilter(GL_LINEAR);
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

		renderTarget->AttachTexture(HEIGHT_DATA_A, &_node.geom->HeightDataA());
		renderTarget->AttachTexture(HEIGHT_DATA_B, &_node.geom->HeightDataB());
		renderTarget->AttachTexture(FLUX_DATA_A, &_node.geom->FluxDataA());
		renderTarget->AttachTexture(FLUX_DATA_B, &_node.geom->FluxDataB());
		renderTarget->AttachTexture(VELOCITY_DATA, &_node.geom->VelocityData());
		renderTarget->AttachTexture(MAPPING_DATA_A, &_node.geom->MappingDataA());
		renderTarget->AttachTexture(MAPPING_DATA_B, &_node.geom->MappingDataB());
		renderTarget->AttachTexture(NORMAL_DATA, &_node.geom->NormalData());

		m_renderTargetByNodeMap.insert(std::make_pair(&_node, renderTarget));
	}

	void TerrainSimulationProcess::OnNodeRemoved(const TerrainSimPassNode & _node)
	{
		RenderTargetBase* renderTarget = m_renderTargetByNodeMap[&_node];
		delete renderTarget;
		m_renderTargetByNodeMap.erase(m_renderTargetByNodeMap.find(&_node));
	}
	
}
