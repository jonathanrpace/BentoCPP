#include "TerrainSimulationProcess.h"

#include <bento/core/Logging.h>
#include <glfw3.h>

#include <utility>

#include <imgui.h>

namespace godBox
{

	UpdateFluxFrag::UpdateFluxFrag()
		: ShaderStageBase("shaders/UpdateFlux.frag")
	{}

	ApplyFluxFrag::ApplyFluxFrag()
		: ShaderStageBase("shaders/ApplyFlux.frag")
	{}

	UpdateTerrainDataFrag::UpdateTerrainDataFrag()
		: ShaderStageBase("shaders/UpdateTerrainData.frag")
	{}

	DeriveTerrainDataFrag::DeriveTerrainDataFrag()
		: ShaderStageBase("shaders/DeriveTerrainData.frag")
	{}

	JacobiFrag::JacobiFrag()
		: ShaderStageBase("shaders/Jacobi.frag")
	{}

	ComputeDivergenceFrag::ComputeDivergenceFrag()
		: ShaderStageBase("shaders/ComputeDivergence.frag")
	{}

	TerrainSimulationProcess::TerrainSimulationProcess(std::string _name)
		: Process(_name, typeid(TerrainSimulationProcess))
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
			AdvanceTerrainSim(*renderTarget, *(node->geom), *(node->material), *(node->moltenParticleGeom), *(node->terrainSim));
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// PRIVATE
	//////////////////////////////////////////////////////////////////////////

	void TerrainSimulationProcess::AdvanceTerrainSim
	(
		RenderTargetBase & _renderTarget,
		TerrainGeometry & _geom, 
		TerrainMaterial & _material, 
		MoltenParticleGeom & _moltenParticleGeom,
		TerrainSimulation & _terrainSim
	)
	{
		GL_CHECK(glViewport(0, 0, _geom.NumVerticesPerDimension(), _geom.NumVerticesPerDimension()));
		glDisable(GL_DEPTH_TEST);
		glDepthMask(true);
		glDepthFunc(GL_ALWAYS);

		UpdatePressure(_renderTarget, _geom, _material, _terrainSim);
		UpdateFlux(_renderTarget, _geom, _material, _terrainSim);
		ApplyFlux(_renderTarget, _geom, _material, _terrainSim);
		UpdateTerrainData(_renderTarget, _geom, _material, _terrainSim);
		DeriveTerrainData(_renderTarget, _geom, _material, _terrainSim);
	}

	void TerrainSimulationProcess::ApplyFlux
	(
		RenderTargetBase& _renderTarget, 
		TerrainGeometry& _geom, 
		TerrainMaterial& _material,
		TerrainSimulation& _terrainSim
	)
	{
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, _geom.HeightData().GetWrite());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT1, _geom.MiscData().GetWrite());
		static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		_renderTarget.SetDrawBuffers(drawBuffers, sizeof(drawBuffers) / sizeof(drawBuffers[0]));
			
		m_applyFluxShader.BindPerPass();
		ApplyFluxFrag& fragShader = m_applyFluxShader.FragmentShader();

		// Samplers
		fragShader.SetTexture("s_heightData",					_geom.HeightData().GetRead());
		fragShader.SetTexture("s_moltenFluxData",				_geom.MoltenFluxData().GetRead());
		fragShader.SetTexture("s_waterFluxData",				_geom.WaterFluxData().GetRead());
		fragShader.SetTexture("s_miscData",						_geom.MiscData().GetRead());
	
		// Molten
		fragShader.SetUniform("u_moltenDiffuseStrength",		_terrainSim.moltenDiffusionStrength);
	
		m_screenQuadGeom.Draw();

		_geom.HeightData().GetWrite().GenerateMipMaps();
		_geom.MiscData().GetWrite().GenerateMipMaps();

		_geom.HeightData().Swap();
		_geom.MiscData().Swap();
	}

	void TerrainSimulationProcess::UpdateFlux
	(
		RenderTargetBase & _renderTarget,
		TerrainGeometry & _geom,
		TerrainMaterial & _material,
		TerrainSimulation& _terrainSim
	)
	{
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, _geom.MoltenFluxData().GetWrite());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT1, _geom.WaterFluxData().GetWrite());
		static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		_renderTarget.SetDrawBuffers(drawBuffers, sizeof(drawBuffers) / sizeof(drawBuffers[0]));

		m_updateFluxShader.BindPerPass();
		UpdateFluxFrag& fragShader = m_updateFluxShader.FragmentShader();

		// Samplers
		fragShader.SetTexture("s_heightData",					_geom.HeightData().GetRead());
		fragShader.SetTexture("s_moltenFluxData",				_geom.MoltenFluxData().GetRead());
		fragShader.SetTexture("s_waterFluxData",				_geom.WaterFluxData().GetRead());
		fragShader.SetTexture("s_miscData",						_geom.MiscData().GetRead());
		fragShader.SetTexture( "s_pressureData",				_geom.PressureData().GetRead() );
	
		// Molten
		fragShader.SetUniform("u_moltenViscosity",				_terrainSim.moltenViscosity);
		fragShader.SetUniform("u_moltenDamping",				_terrainSim.moltenDamping);
		fragShader.SetUniform("u_moltenMinHeat",				_terrainSim.minMoltenHeat);
		fragShader.SetUniform("u_moltenSlopeStrength",			_terrainSim.moltenSlopeStrength);
		fragShader.SetUniform("u_moltenPressureScale",			_terrainSim.moltenPressureStrength );
		
		// Water
		fragShader.SetUniform("u_waterViscosity",				_terrainSim.waterViscosity);
		fragShader.SetUniform("u_waterSlopeStrength",			_terrainSim.waterSlopeStrength);

		m_screenQuadGeom.Draw();

		_geom.MoltenFluxData().Swap();
		_geom.WaterFluxData().Swap();
	}

	void TerrainSimulationProcess::UpdateTerrainData
	(
		RenderTargetBase & _renderTarget,
		TerrainGeometry & _geom,
		TerrainMaterial & _material,
		TerrainSimulation& _terrainSim
	)
	{
		IInputManager& inputManager = m_scene->GetInputManager();

		float dirtScalar = inputManager.IsKeyDown(GLFW_KEY_LEFT_ALT) ? 1.0f : 0.0f;
		float waterScalar = inputManager.IsKeyDown(GLFW_KEY_LEFT_CONTROL) ? 1.0f : 0.0f && dirtScalar == 0.0f;
		float moltenScalar = (dirtScalar == 0.0f && waterScalar == 0.0f) ? 1.0f : 0.0f;
		float mouseIsDown = inputManager.IsMouseDown(1) ? 1.0f : 0.0f;
	
		float moltenVolumeAmount = mouseIsDown * _terrainSim.mouseVolumeStrength * moltenScalar;
		float dirtVolumeAmount = mouseIsDown * _terrainSim.mouseVolumeStrength * dirtScalar;
		float waterVolumeAmount = mouseIsDown * _terrainSim.mouseVolumeStrength * waterScalar;
		float heatChangeAmount = mouseIsDown * _terrainSim.mouseHeatStrength * moltenScalar;

		float phaseA = fmodf( (float)glfwGetTime() * _material.moltenFlowSpeed, 1.0f );
		float phaseAlpha = fabs( phaseA - 0.5f ) * 2.0f;	// Sawtooth
		float phaseB = fmodf( phaseA + 0.5f, 1.0f );

		bool m_phaseALatch = m_prevPhaseA > phaseA;
		bool m_phaseBLatch = m_prevPhaseB > phaseB;
		m_prevPhaseA = phaseA;
		m_prevPhaseB = phaseB;

		float cellSize = _geom.Size() / (float)_geom.NumVerticesPerDimension();
		

		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, _geom.HeightData().GetWrite());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT1, _geom.MiscData().GetWrite());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT2, _geom.SmudgeData().GetWrite());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT3, _geom.UVOffsetData().GetWrite());
		static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		_renderTarget.SetDrawBuffers(drawBuffers, sizeof(drawBuffers) / sizeof(drawBuffers[0]));

		m_updateDataShader.BindPerPass();
		UpdateTerrainDataFrag& fragShader = m_updateDataShader.FragmentShader();

		fragShader.SetUniform("u_cellSize",						cellSize);

		// Samplers
		fragShader.SetTexture("s_heightData",					_geom.HeightData().GetRead());
		fragShader.SetTexture("s_moltenFluxData",				_geom.MoltenFluxData().GetRead());
		fragShader.SetTexture("s_waterFluxData",				_geom.WaterFluxData().GetRead());
		fragShader.SetTexture("s_miscData",						_geom.MiscData().GetRead());
		fragShader.SetTexture("s_smudgeData",					_geom.SmudgeData().GetRead());
		fragShader.SetTexture("s_uvOffsetData",					_geom.UVOffsetData().GetRead());
		fragShader.SetTexture("s_derivedData",					_geom.DerivedData().GetRead() );
		fragShader.SetTexture("s_pressureData",					_geom.PressureData().GetRead() );
		fragShader.SetTexture("s_noiseMap",						_material.lavaMat );

		// Mouse
		fragShader.SetUniform("u_mouseRadius",					_terrainSim.mouseRadius);
		fragShader.SetUniform("u_mouseMoltenVolumeStrength",	moltenVolumeAmount);
		fragShader.SetUniform("u_mouseMoltenHeatStrength",		heatChangeAmount);
		fragShader.SetUniform("u_mouseWaterVolumeStrength",		waterVolumeAmount);
		fragShader.SetUniform("u_mouseDirtVolumeStrength",		dirtVolumeAmount);
		
		// Environment
		fragShader.SetUniform("u_ambientTemp",					_terrainSim.ambientTemperature);
		fragShader.SetUniform("u_phase",						phaseAlpha);
		fragShader.SetUniform("u_phaseALatch",					m_phaseALatch);
		fragShader.SetUniform("u_phaseBLatch",					m_phaseBLatch);

		// Molten
		fragShader.SetUniform("u_tempChangeSpeed",				_terrainSim.tempChangeSpeed);
		fragShader.SetUniform("u_moltenMinHeat",				_terrainSim.minMoltenHeat);
		fragShader.SetUniform("u_meltSpeed",					_terrainSim.meltSpeed);
		fragShader.SetUniform("u_condenseSpeed",				_terrainSim.condenseSpeed);
		fragShader.SetUniform("u_smudgeChangeRate",				_terrainSim.smudgeChangeRate);
	
		// Water
		fragShader.SetUniform("u_waterViscosity",				_terrainSim.waterViscosity);
		fragShader.SetUniform("u_evapourationRate",				_terrainSim.evapourationRate);
		fragShader.SetUniform("u_rainRate",						_terrainSim.rainRate);
		fragShader.SetUniform("u_boilSpeed",					_terrainSim.boilSpeed);
		fragShader.SetUniform("u_drainRate",					_terrainSim.drainRate);
		fragShader.SetUniform("u_drainMaxDepth",				_terrainSim.drainMaxDepth);

		// Dirt
		fragShader.SetUniform("u_dirtViscosity",				_terrainSim.dirtViscosity);
		fragShader.SetUniform("u_dirtMaxSlope",					_terrainSim.dirtMaxSlope);
		fragShader.SetUniform("u_dirtDensity",					_terrainSim.dirtDensity);

		// Erosion
		fragShader.SetUniform("u_erosionStrength",				_terrainSim.erosionStrength);
		fragShader.SetUniform("u_erosionMaxDepth",				_terrainSim.erosionDirtDepthMax);
		fragShader.SetUniform("u_erosionWaterDepthMin",			_terrainSim.erosionWaterDepthMin);
		fragShader.SetUniform("u_erosionWaterDepthMax",			_terrainSim.erosionWaterDepthMax);
		fragShader.SetUniform("u_erosionWaterSpeedMax",			_terrainSim.erosionWaterSpeedMax);

		// Dirt transport
		fragShader.SetUniform("u_dirtTransportSpeed",			_terrainSim.dirtTransportSpeed);
		fragShader.SetUniform("u_dirtPickupMinWaterSpeed",		_terrainSim.dirtPickupMinWaterSpeed);
		fragShader.SetUniform("u_dirtPickupRate",				_terrainSim.dirtPickupRate);
		fragShader.SetUniform("u_dirtDepositSpeed",				_terrainSim.dirtDepositSpeed);
		fragShader.SetUniform("u_dissolvedDirtSmoothing",		_terrainSim.dissolvedDirtSmoothing);

		// Pass through the mouse position buffer
		TerrainMousePos mousePos;
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(mousePos), &mousePos);
		//PRINTF("mouse pos %d, %d, %d\n", mousePos.z, mousePos.u, mousePos.v);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _geom.MousePositionBuffer());

		m_screenQuadGeom.Draw();

		_geom.HeightData().GetWrite().GenerateMipMaps();
		_geom.MiscData().GetWrite().GenerateMipMaps();
		_geom.SmudgeData().GetWrite().GenerateMipMaps();

		_geom.HeightData().Swap();
		_geom.MiscData().Swap();
		_geom.SmudgeData().Swap();
		_geom.UVOffsetData().Swap();
	}

	void TerrainSimulationProcess::DeriveTerrainData
	(
		RenderTargetBase & _renderTarget,
		TerrainGeometry & _geom,
		TerrainMaterial & _material,
		TerrainSimulation& _terrainSim
	)
	{
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, _geom.NormalData());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT1, _geom.DerivedData().GetWrite());
		static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		_renderTarget.SetDrawBuffers(drawBuffers, sizeof(drawBuffers) / sizeof(drawBuffers[0]));

		m_deriveDataShader.BindPerPass();
		DeriveTerrainDataFrag& fragShader = m_deriveDataShader.FragmentShader();

		// Samplers
		fragShader.SetTexture("s_heightData",					_geom.HeightData().GetRead());

		// Misc
		vec2 cellSize = vec2(_geom.Size() / (float)_geom.NumVerticesPerDimension());
		fragShader.SetUniform("u_cellSize",						cellSize);
		fragShader.SetUniform("u_numHeightMips",				_geom.HeightData().GetRead().GetNumMipMaps());

		m_screenQuadGeom.Draw();

		_geom.DerivedData().Swap();
	}

	void TerrainSimulationProcess::UpdatePressure(
		RenderTargetBase & _renderTarget,
		TerrainGeometry & _geom,
		TerrainMaterial & _material,
		TerrainSimulation& _terrainSim
	)
	{
		vec2 cellSize = vec2(_geom.Size() / (float)_geom.NumVerticesPerDimension());

		// Given the fluid's current motion, calculate how much the velocity arrows are pointing towards or away from each other.
		// A so-called 'divergence' field.
		{
			m_computeDivergenceShader.BindPerPass();

			m_computeDivergenceShader.FragmentShader().SetUniform( "u_halfInverseCellSize", 0.5f / cellSize.x );
			m_computeDivergenceShader.FragmentShader().SetTexture( "s_fluxData", _geom.MoltenFluxData().GetRead() );
			m_computeDivergenceShader.FragmentShader().SetTexture( "s_heightData", _geom.HeightData().GetRead() );
			
			_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, _geom.DivergenceData());
			static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
			_renderTarget.SetDrawBuffers(drawBuffers, sizeof(drawBuffers) / sizeof(drawBuffers[0]));

			m_screenQuadGeom.Draw();
		}

		// This output is then consumed by a 'jacobi' iterative pass that progressively calculates the pressure created
		// by the divergence. This pressure gradient is then used to affect velocity during update data
		ClearSurface(_renderTarget, _geom.PressureData().GetRead(), 0.0f);
		for (int i = 0; i < 2; ++i) 
		{
			Jacobi(_renderTarget, _geom.PressureData().GetRead(), _geom.DivergenceData(), cellSize, _geom.PressureData().GetWrite());
			_geom.PressureData().Swap();
		}
	}

	void TerrainSimulationProcess::ClearSurface(RenderTargetBase& renderTarget, TextureSquare& dest, float v)
	{
		renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, dest);
		static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
		renderTarget.SetDrawBuffers(drawBuffers, sizeof(drawBuffers) / sizeof(drawBuffers[0]));

		GL_CHECK( glClearColor(v, v, v, v) );
		GL_CHECK( glClear(GL_COLOR_BUFFER_BIT) );
	}

	void TerrainSimulationProcess::Jacobi(RenderTargetBase& renderTarget, TextureSquare & pressure,TextureSquare & divergence, vec2 cellSize, TextureSquare & dest)
	{
		m_jacobiShader.BindPerPass();

		m_jacobiShader.FragmentShader().SetUniform( "u_alpha", -cellSize.x * cellSize.x );
		m_jacobiShader.FragmentShader().SetTexture( "s_pressureData", pressure );
		m_jacobiShader.FragmentShader().SetTexture( "s_divergenceData", divergence );

		renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, dest);
		static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
		renderTarget.SetDrawBuffers(drawBuffers, sizeof(drawBuffers) / sizeof(drawBuffers[0]));

		m_screenQuadGeom.Draw();
	}

	void TerrainSimulationProcess::OnNodeAdded(const TerrainSimPassNode & _node)
	{
		RenderTargetBase* renderTarget = new RenderTargetBase
		(
			_node.geom->NumVerticesPerDimension(),
			_node.geom->NumVerticesPerDimension(),
			false
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
