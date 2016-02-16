#include "TerrainSimulationProcess.h"

#include <bento/core/Logging.h>
#include <glfw3.h>

#include <utility>

#include <imgui.h>

namespace bento
{
	//////////////////////////////////////////////////////////////////////////
	// UpdateTerrainFluxFrag
	//////////////////////////////////////////////////////////////////////////

	UpdateTerrainFluxFrag::UpdateTerrainFluxFrag() : ShaderStageBase("shaders/UpdateTerrainFlux.frag") {}

	//////////////////////////////////////////////////////////////////////////
	// UpdateTerrainDataFrag
	//////////////////////////////////////////////////////////////////////////

	UpdateTerrainDataFrag::UpdateTerrainDataFrag() : ShaderStageBase("shaders/UpdateTerrainData.frag") {}

	//////////////////////////////////////////////////////////////////////////
	// UpdateTerrainMiscFrag
	//////////////////////////////////////////////////////////////////////////

	UpdateTerrainMiscFrag::UpdateTerrainMiscFrag() : ShaderStageBase("shaders/UpdateTerrainMisc.frag") {}

	//////////////////////////////////////////////////////////////////////////
	// UpdateTerrainSmoothingFrag
	//////////////////////////////////////////////////////////////////////////

	UpdateTerrainSmthFrag::UpdateTerrainSmthFrag() : ShaderStageBase("shaders/UpdateTerrainSmoothing.frag") {}

	//////////////////////////////////////////////////////////////////////////
	// FoamParticleUpdateVert
	//////////////////////////////////////////////////////////////////////////

	FoamParticleUpdateVert::FoamParticleUpdateVert() : ShaderStageBase("shaders/FoamParticleUpdate.vert", false) {}

	void FoamParticleUpdateVert::OnPreLink()
	{
		const char * varyings[] = { "out_position", "out_velocity" };
		GL_CHECK(glTransformFeedbackVaryings(m_programName, 2, varyings, GL_SEPARATE_ATTRIBS));
	}
	
	//////////////////////////////////////////////////////////////////////////
	// TerrainSimulationPass
	//////////////////////////////////////////////////////////////////////////
	
	TerrainSimulationProcess::TerrainSimulationProcess(std::string _name)
		: Process(_name, typeid(TerrainSimulationProcess))
		, m_updateFluxShader()
		, m_updateDataShader()
		, m_updateMiscShader()
		, m_updateSmthShader()
		, m_foamParticleUpdateShader()
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
			AdvanceTerrainSim(*(node->geom), *(node->material), *renderTarget, *(node->foamGeom));
		}
	}

	void TerrainSimulationProcess::AddUIElements()
	{
		ImGui::Spacing();
		ImGui::Text("Input");
		ImGui::SliderFloat("MouseRadius", &m_mouseRadius, 0.01f, 0.5f);
		ImGui::SliderFloat("MouseVolumeStrength", &m_mouseMoltenVolumeStrength, 0.00f, 0.01f, "%.5f");
		ImGui::SliderFloat("MouseHeatStrength", &m_mouseHeatStrength, 0.00f, 1.0f, "%.2f");
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Environment");
		ImGui::SliderFloat("AmbientTemp", &m_ambientTemperature, -1.0f, 1.0f);
		ImGui::Spacing();

		ImGui::Text("Molten");
		ImGui::SliderFloat("FluxDamping", &m_rockFluxDamping, 0.9f, 1.0f);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::SliderFloat("ViscosityMin", &m_viscosityMin, 0.01f, 0.5f);
		ImGui::SliderFloat("ViscosityMax", &m_viscosityMax, 0.01f, 0.5f);
		ImGui::SliderFloat("HeatViscosityPower", &m_heatViscosityPower, 0.1f, 10.0f);
		ImGui::SliderFloat("HeatViscosityBias", &m_rockMeltingPoint, 0.0f, 2.0f);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::SliderFloat("HeatAdvectSpeed", &m_heatAdvectSpeed, 0.0f, 5.0f);
		ImGui::SliderFloat("HeatSmoothing", &m_heatSmoothingStrength, 0.0f, 0.5f, "%.4f");
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::SliderFloat("TempChangeSpeed", &m_tempChangeSpeed, 0.0f, 0.05f, "%.5f");
		ImGui::SliderFloat("MeltSpeed", &m_meltSpeed, 0.0f, 0.001f, "%.5f");
		ImGui::SliderFloat("CondenseSpeed", &m_condenseSpeed, 0.0f, 0.1f, "%.5f");
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::SliderFloat("ScrollSpeed", &m_textureScrollSpeed, 0.0f, 0.2f);
		ImGui::SliderFloat("TextureCycleSpeed", &m_textureCycleSpeed, 0.0f, 0.5f);
		ImGui::Spacing();

		ImGui::Text("Water");
		ImGui::SliderFloat("FluxDamping2", &m_waterFluxDamping, 0.9f, 1.0f);
		ImGui::SliderFloat("Viscosity2", &m_waterViscosity, 0.01f, 0.5f);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::SliderFloat("ErosionSpeed", &m_erosionSpeed, 0.0f, 0.05f);
		ImGui::SliderFloat("ErosionFluxMin", &m_erosionFluxMin, 0.0f, 0.5f);
		ImGui::SliderFloat("ErosionFluxMax", &m_erosionFluxMax, 0.0f, 0.5f);
		ImGui::SliderFloat("DepositionSpeed", &m_depositionSpeed, 0.0f, 0.05f);
		ImGui::Spacing();


	}

	//////////////////////////////////////////////////////////////////////////
	// PRIVATE
	//////////////////////////////////////////////////////////////////////////

	void TerrainSimulationProcess::AdvanceTerrainSim
	(
		TerrainGeometry & _geom, 
		TerrainMaterial & _material, 
		RenderTargetBase & _renderTarget,
		FoamParticleGeom & _foamParticleGeom
	)
	{
		GL_CHECK(glViewport(0, 0, _geom.NumVerticesPerDimension(), _geom.NumVerticesPerDimension()));

		vec2 normalisedMousePos = m_scene->GetInputManager()->GetMousePosition();
		normalisedMousePos /= m_scene->GetWindow()->GetWindowSize();

		bool mouseIsDown = m_scene->GetInputManager()->IsMouseDown(1);

		float moltenScalar = m_scene->GetInputManager()->IsKeyDown(GLFW_KEY_LEFT_CONTROL) ? 0.0f : 1.0f;
		float moltenVolumeAmount = (m_scene->GetInputManager()->IsMouseDown(1) ? 1.0f : 0.0f) * m_mouseMoltenVolumeStrength * moltenScalar;
		float heatChangeAmount = (m_scene->GetInputManager()->IsMouseDown(1) ? 1.0f : 0.0f) * m_mouseHeatStrength * moltenScalar;

		float waterScalar = m_scene->GetInputManager()->IsKeyDown(GLFW_KEY_LEFT_CONTROL) ? 1.0f : 0.0f;
		float waterVolumeAmount = (m_scene->GetInputManager()->IsMouseDown(1) ? 1.0f : 0.0f) * m_mouseMoltenVolumeStrength * waterScalar;

		vec2 cellSize = vec2(_geom.Size() / (float)_geom.NumVerticesPerDimension());

		// Update Flux
		{
			if (!m_switch)
			{
				_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, &_geom.RockFluxDataB());
				_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT1, &_geom.WaterFluxDataB());
			}
			else
			{
				_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, &_geom.RockFluxDataA());
				_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT1, &_geom.WaterFluxDataA());
			}

			static GLenum fluxDrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			_renderTarget.SetDrawBuffers(fluxDrawBuffers, sizeof(fluxDrawBuffers) / sizeof(fluxDrawBuffers[0]));


			m_updateFluxShader.BindPerPass();
			auto fragShader = m_updateFluxShader.FragmentShader();

			fragShader.SetTexture("s_rockData", &_geom.RockDataRead());
			fragShader.SetTexture("s_rockFluxData", &_geom.RockFluxDataRead());

			fragShader.SetTexture("s_waterData", &_geom.WaterDataRead());
			fragShader.SetTexture("s_waterFluxData", &_geom.WaterFluxDataRead());

			fragShader.SetUniform("u_rockFluxDamping", m_rockFluxDamping);
			fragShader.SetUniform("u_waterFluxDamping", m_waterFluxDamping);

			m_screenQuadGeom.Draw();

			_geom.SwapRockFluxData();
			_geom.SwapWaterFluxData();
		}
		
		_geom.RockFluxDataRead().GenerateMipMaps();
		_geom.WaterFluxDataRead().GenerateMipMaps();
		
		// Update Data
		{
			if (!m_switch)
			{
				_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, &_geom.RockDataB());
				_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT1, &_geom.WaterDataB());
			}
			else
			{
				_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, &_geom.RockDataA());
				_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT1, &_geom.WaterDataA());
			}

			static GLenum heightDrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			_renderTarget.SetDrawBuffers(heightDrawBuffers, sizeof(heightDrawBuffers) / sizeof(heightDrawBuffers[0]));
			
			m_updateDataShader.BindPerPass();
			auto fragShader = m_updateDataShader.FragmentShader();
			fragShader.SetTexture("s_rockData", &_geom.RockDataRead());
			fragShader.SetTexture("s_rockFluxData", &_geom.RockFluxDataRead());
			fragShader.SetTexture("s_rockNormalData", &_geom.RockNormalData());

			fragShader.SetTexture("s_waterData", &_geom.WaterDataRead());
			fragShader.SetTexture("s_waterFluxData", &_geom.WaterFluxDataRead());
			fragShader.SetTexture("s_waterNormalData", &_geom.WaterNormalData());

			fragShader.SetTexture("s_mappingData", &_geom.MappingDataRead());
			fragShader.SetTexture("s_diffuseMap", &_material.SomeTexture);
			
			fragShader.SetUniform("u_mousePos", normalisedMousePos);
			fragShader.SetUniform("u_mouseRadius", m_mouseRadius);
			fragShader.SetUniform("u_mouseMoltenVolumeStrength", moltenVolumeAmount);
			fragShader.SetUniform("u_mouseMoltenHeatStrength", heatChangeAmount);
			fragShader.SetUniform("u_mouseWaterVolumeStrength", waterVolumeAmount);
			
			fragShader.SetUniform("u_heatAdvectSpeed", m_heatAdvectSpeed);

			fragShader.SetUniform("u_viscosityMin", m_viscosityMin);
			fragShader.SetUniform("u_viscosityMax", m_viscosityMax);
			fragShader.SetUniform("u_heatViscosityPower", m_heatViscosityPower);
			fragShader.SetUniform("u_heatViscosityBias", m_rockMeltingPoint);

			fragShader.SetUniform("u_ambientTemp", m_ambientTemperature);
			fragShader.SetUniform("u_tempChangeSpeed", m_tempChangeSpeed);
			fragShader.SetUniform("u_meltSpeed", m_meltSpeed);
			fragShader.SetUniform("u_condenseSpeed", m_condenseSpeed);

			fragShader.SetUniform("u_waterViscosity", m_waterViscosity);

			fragShader.SetUniform("u_erosionSpeed", m_erosionSpeed);
			fragShader.SetUniform("u_erosionFluxMin", m_erosionFluxMin);
			fragShader.SetUniform("u_erosionFluxMax", m_erosionFluxMax);
			fragShader.SetUniform("u_depositionSpeed", m_depositionSpeed);

			// Pass through the mouse position buffer

			TerrainMousePos mousePos;
			glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(mousePos), &mousePos);
			PRINTF("mouse pos %d, %d, %d\n", mousePos.z, mousePos.u, mousePos.v);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _geom.MousePositionBuffer());

			m_screenQuadGeom.Draw();

			_geom.SwapRockData();
			_geom.SwapWaterData();
		}

		// Update Misc
		{
			_geom.RockDataRead().GenerateMipMaps();
			_geom.WaterDataRead().GenerateMipMaps();

			if (!m_switch)
			{
				_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, &_geom.MappingDataA());
				_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT1, &_geom.RockNormalData());
				_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT2, &_geom.WaterNormalData());
			}
			else
			{
				_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, &_geom.MappingDataB());
				_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT1, &_geom.RockNormalData());
				_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT2, &_geom.WaterNormalData());
			}

			static GLenum velocityDrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
			_renderTarget.SetDrawBuffers(velocityDrawBuffers, sizeof(velocityDrawBuffers) / sizeof(velocityDrawBuffers[0]));

			m_updateMiscShader.BindPerPass();
			auto fragShader = m_updateMiscShader.FragmentShader();

			fragShader.SetTexture("s_rockData", &_geom.RockDataRead());
			fragShader.SetTexture("s_rockFluxData", &_geom.RockFluxDataRead());
			fragShader.SetTexture("s_waterData", &_geom.WaterDataRead());
			fragShader.SetTexture("s_waterFluxData", &_geom.WaterFluxDataRead());
			fragShader.SetTexture("s_mappingData", &_geom.MappingDataRead());
			fragShader.SetTexture("s_diffuseMap", &_material.SomeTexture);
			
			fragShader.SetUniform("u_numHeightMips", _geom.RockDataRead().GetNumMipMaps());

			fragShader.SetUniform("u_viscosityMin", m_viscosityMin);
			fragShader.SetUniform("u_viscosityMax", m_viscosityMax);
			fragShader.SetUniform("u_heatViscosityPower", m_heatViscosityPower);
			fragShader.SetUniform("u_heatViscosityBias", m_rockMeltingPoint);

			fragShader.SetUniform("u_waterViscosity", m_waterViscosity);

			fragShader.SetUniform("u_textureScrollSpeed", m_textureScrollSpeed);
			fragShader.SetUniform("u_cycleSpeed", m_textureCycleSpeed);
			fragShader.SetUniform("u_cellSize", cellSize);

			fragShader.SetUniform("u_mapHeightOffset", _material.MapHeightOffset);

			fragShader.SetUniform("u_mousePos", normalisedMousePos);
			fragShader.SetUniform("u_mouseVolumeStrength", moltenVolumeAmount);
			fragShader.SetUniform("u_mouseHeatStrength", heatChangeAmount);
			fragShader.SetUniform("u_mouseRadius", m_mouseRadius);

			fragShader.SetUniform("u_time", (float)glfwGetTime());

			GL_CHECK(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _geom.MousePositionBuffer()));

			m_screenQuadGeom.Draw();
		}

		// Update foam
		{
			glBindProgramPipeline(GL_NONE);

			m_foamParticleUpdateShader.BindPerPass();
			
			if (_foamParticleGeom.Switch())
			{
				GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, _foamParticleGeom.TransformFeedbackObjB()));
				GL_CHECK(glBindVertexArray(_foamParticleGeom.VertexArrayA()));
			}
			else
			{
				GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, _foamParticleGeom.TransformFeedbackObjA()));
				GL_CHECK(glBindVertexArray(_foamParticleGeom.VertexArrayB()));
			}

			GL_CHECK(glBeginTransformFeedback(GL_POINTS));
			GL_CHECK(glEnable(GL_RASTERIZER_DISCARD));

			m_foamParticleUpdateShader.VertexShader().SetTexture("s_waterNormalData", &_geom.WaterNormalData());
			m_foamParticleUpdateShader.VertexShader().SetTexture("s_waterData", &_geom.WaterDataRead());
			m_foamParticleUpdateShader.VertexShader().SetTexture("s_rockData", &_geom.RockDataRead());

			GL_CHECK(glDrawArrays(GL_POINTS, 0, _foamParticleGeom.NumParticles()));

			GL_CHECK(glDisable(GL_RASTERIZER_DISCARD));
			GL_CHECK(glEndTransformFeedback());
			glUseProgram(GL_NONE);

			_foamParticleGeom.Switch(!_foamParticleGeom.Switch());
		}

		/*
		// Diffuse height
		{
			if (&_geom.RockDataRead() == &_geom.RockDataA())
			{
				_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, &_geom.RockDataB());
				_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT1, &_geom.RockDataA());
			}
			else
			{
				_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, &_geom.RockDataA());
				_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT1, &_geom.RockDataB());
			}

			static GLenum drawBuffersA[] = { GL_COLOR_ATTACHMENT0 };
			_renderTarget.SetDrawBuffers(drawBuffersA, sizeof(drawBuffersA) / sizeof(drawBuffersA[0]));

			m_diffuseHeightShader.BindPerPass();
			auto fragShader = m_diffuseHeightShader.FragmentShader();

			fragShader.SetUniform("u_heatSmoothStrength", m_heatSmoothingStrength);
			fragShader.SetTexture("s_heightData", &_geom.RockDataRead());
			fragShader.SetUniform("u_axis", ivec2(1,0));
			m_screenQuadGeom.Draw();
			_geom.SwapRockData();

			static GLenum drawBuffersB[] = { GL_COLOR_ATTACHMENT1 };
			_renderTarget.SetDrawBuffers(drawBuffersB, sizeof(drawBuffersB) / sizeof(drawBuffersB[0]));

			fragShader.SetTexture("s_heightData", &_geom.RockDataRead());
			fragShader.SetUniform("u_axis", ivec2(0,1));
			m_screenQuadGeom.Draw();
			_geom.SwapRockData();
		}
		*/
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
		
		m_renderTargetByNodeMap.insert(std::make_pair(&_node, renderTarget));
	}

	void TerrainSimulationProcess::OnNodeRemoved(const TerrainSimPassNode & _node)
	{
		RenderTargetBase* renderTarget = m_renderTargetByNodeMap[&_node];
		delete renderTarget;
		m_renderTargetByNodeMap.erase(m_renderTargetByNodeMap.find(&_node));
	}
	
}
