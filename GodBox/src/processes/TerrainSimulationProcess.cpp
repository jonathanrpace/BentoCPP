#include "TerrainSimulationProcess.h"

#include <bento/core/Logging.h>
#include <glfw3.h>

#include <utility>

#include <imgui.h>

namespace godBox
{
//////////////////////////////////////////////////////////////////////////
UpdateTerrainDataFrag::UpdateTerrainDataFrag()
	: ShaderStageBase("shaders/UpdateTerrainData.frag")
{}

//////////////////////////////////////////////////////////////////////////
DeriveTerrainDataFrag::DeriveTerrainDataFrag()
	: ShaderStageBase("shaders/DeriveTerrainData.frag")
{}

//////////////////////////////////////////////////////////////////////////
JacobiFrag::JacobiFrag()
	: ShaderStageBase("shaders/Jacobi.frag")
{}

//////////////////////////////////////////////////////////////////////////
ComputeDivergenceFrag::ComputeDivergenceFrag()
	: ShaderStageBase("shaders/ComputeDivergence.frag")
{}

//////////////////////////////////////////////////////////////////////////
// TerrainSimulationPass
//////////////////////////////////////////////////////////////////////////
	
TerrainSimulationProcess::TerrainSimulationProcess(std::string _name)
	: Process(_name, typeid(TerrainSimulationProcess))
	, SerializableBase("TerrainSimulationProcess")
{
	m_nodeGroup.NodeAdded += OnNodeAddedDelegate;
	m_nodeGroup.NodeRemoved += OnNodeRemovedDelegate;
	
	// Molten
	SerializableMember("mouseRadius",			0.1f,		&m_mouseRadius);
	SerializableMember("mouseVolumeStrength",	0.002f,		&m_mouseVolumeStrength);
	SerializableMember("mouseHeatStrength",		0.08f,		&m_mouseHeatStrength);
	SerializableMember("moltenViscosityMin",	0.5f,		&m_moltenViscosityMin);
	SerializableMember("moltenViscosityMax",	0.5f,		&m_moltenViscosityMax);
	SerializableMember("rockMeltingPoint",		0.1f,		&m_rockMeltingPoint);
	SerializableMember("heatViscosityScalar",	10.0f,		&m_heatViscosityScalar);
	SerializableMember("meltSpeedCondenseSpeed",0.01f,		&m_meltCondenseSpeed);
	SerializableMember("meltSpeed",				0.01f,		&m_meltSpeed);
	SerializableMember("condenseSpeed",			0.01f,		&m_condenseSpeed);
	SerializableMember("meltSpeedCondenseSpeed",0.01f,		&m_meltCondenseSpeed);
	SerializableMember("tempChangeSpeed",		0.002f,		&m_tempChangeSpeed);
	SerializableMember("smudgeChangeRate",		0.01f,		&m_smudgeChangeRate);
	SerializableMember("moltenSlopeStrength",	0.3f,		&m_moltenSlopeStrength);
	SerializableMember("moltenSlopePower",		1.0f,		&m_moltenSlopePower);
	SerializableMember("moltenDiffusionStrength",1.0f,		&m_moltenDiffusionStrength);
	SerializableMember("moltenPressureStrength",1.0,		&m_moltenPressureStrength);
	SerializableMember("moltenFluxDamping",		0.0,		&m_moltenFluxDamping);
	
	// Water
	SerializableMember("waterFluxDamping",		0.99f,		&m_waterFluxDamping);
	SerializableMember("waterViscosity",		0.25f,		&m_waterViscosity);
	SerializableMember("waterBoilingPoint",		0.1f,		&m_waterBoilingPoint);
	SerializableMember("waterFreezingPoint",	0.0f,		&m_waterFreezingPoint);
	SerializableMember("evapourationRate",		0.0f,		&m_evapourationRate);
	SerializableMember("rainRate",				0.0f,		&m_rainRate);
	SerializableMember("waterVelocityScalar",	1.0f,		&m_waterVelocityScalar);
	SerializableMember("waterVelocityDamping",	0.99f,		&m_waterVelocityDamping);
	SerializableMember("boilSpeed",				0.001f,		&m_boilSpeed);
	SerializableMember("drainRate",				0.0f,		&m_drainRate);
	SerializableMember("drainMaxDepth",			0.01f,		&m_drainMaxDepth);

	// Dirt
	SerializableMember("dirtViscosity",			0.2f,		&m_dirtViscosity);
	SerializableMember("dirtMaxAngle",			0.2f,		&m_dirtMaxSlope);
	SerializableMember("dirtDensity",			0.5f,		&m_dirtDensity);

	// Foam
	SerializableMember("foamSpawnStrength",		1.0f,		&m_foamSpawnStrength);
	SerializableMember("foamDecayRate",			0.95f,		&m_foamDecayRate);

	// Erosion
	SerializableMember("erosionStrength",		0.0f,		&m_erosionStrength);
	SerializableMember("erosionDirtDepthMax",	0.01f,		&m_erosionDirtDepthMax);
	SerializableMember("erosionWaterDepthMin",	0.01f,		&m_erosionWaterDepthMin);
	SerializableMember("erosionWaterDepthMax",	0.01f,		&m_erosionWaterDepthMax);
	SerializableMember("erosionWaterSpeedMax",	0.01f,		&m_erosionWaterSpeedMax);
	
	// Dirt transport
	SerializableMember("dirtTransportSpeed",	0.0f,		&m_dirtTransportSpeed);
	SerializableMember("dirtPickupRate",		0.0f,		&m_dirtPickupRate);
	SerializableMember("dirtPickupMinWaterSpeed",1.0f,		&m_dirtPickupMinWaterSpeed);
	SerializableMember("dirtDepositSpeed",		0.0f,		&m_dirtDepositSpeed);
	SerializableMember("waterDiffuseStrength",	0.00f,		&m_dissolvedDirtSmoothing);
	
	// Global
	SerializableMember("ambientTemperature",	0.05f,		&m_ambientTemperature);
	SerializableMember("timeStep",				1.0f,		&m_timeStep);

	ResetToDefaults();
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
		AdvanceTerrainSim(*(node->geom), *(node->material), *renderTarget, *(node->moltenParticleGeom));
		AdvanceTerrainSim(*(node->geom), *(node->material), *renderTarget, *(node->moltenParticleGeom));
		AdvanceTerrainSim(*(node->geom), *(node->material), *renderTarget, *(node->moltenParticleGeom));
		AdvanceTerrainSim(*(node->geom), *(node->material), *renderTarget, *(node->moltenParticleGeom));
	}
}

void TerrainSimulationProcess::AddUIElements()
{
	ImGui::Spacing();
	ImGui::Text("Input");
	ImGui::SliderFloat("MouseRadius", &m_mouseRadius, 0.01f, 0.5f);
	ImGui::SliderFloat("MouseVolumeStrength", &m_mouseVolumeStrength, 0.00f, 0.01f, "%.5f");
	ImGui::SliderFloat("MouseHeatStrength", &m_mouseHeatStrength, 0.00f, 1.0f, "%.2f");
	ImGui::Spacing();

	ImGui::Spacing();
	ImGui::Text("Environment");
	ImGui::SliderFloat("AmbientTemp", &m_ambientTemperature, -1.0f, 1.0f);
	ImGui::SliderFloat("Time Step", &m_timeStep, 0.0f, 10.0f);
	ImGui::Spacing();

	ImGui::Text("Molten");
	ImGui::SliderFloat("Viscosity Min##molten", &m_moltenViscosityMin, 0.9f, 1.0f);
	ImGui::SliderFloat("Viscosity Max##molten", &m_moltenViscosityMax, 0.9f, 1.0f);
	ImGui::SliderFloat("MeltingPoint", &m_rockMeltingPoint, 0.0f, 2.0f);
	ImGui::SliderFloat("Heat Viscosity Scalar", &m_heatViscosityScalar, 1.0f, 50.0f);
	ImGui::SliderFloat("TempChangeSpeed", &m_tempChangeSpeed, 0.0f, 0.01f, "%.5f");
	ImGui::DragFloat("Melt Speed", &m_meltSpeed, 0.0001f, 0.0f, 0.0f, "%.5f");
	ImGui::DragFloat("Condense Speed", &m_condenseSpeed, 0.0001f, 0.0f, 0.0f, "%.5f");
	ImGui::SliderFloat("SmudgeChangeRate", &m_smudgeChangeRate, 0.0f, 10.0f, "%.5f");
	ImGui::SliderFloat("Slope Strength##molten", &m_moltenSlopeStrength, 0.0f, 50.0f, "%.4f");
	ImGui::DragFloat("Slope Power##molten", &m_moltenSlopePower, 0.001f);
	ImGui::SliderFloat("Diffusion##molten", &m_moltenDiffusionStrength, 0.0f, 0.5f, "%.4f");
	ImGui::DragFloat("Pressure Strength##molten", &m_moltenPressureStrength, 1.0f, 0.0f, 0.0f, "%.3f");
	ImGui::DragFloat("Damping##molten", &m_moltenFluxDamping, 0.0001f, 0.0, 0.1f, "%.4f");
	ImGui::Spacing();

	ImGui::Spacing();
	ImGui::Text("Water");
	ImGui::SliderFloat("Viscosity", &m_waterViscosity, 0.01f, 0.5f);
	ImGui::SliderFloat("FluxDamping##water", &m_waterFluxDamping, 0.7f, 1.0f);
	ImGui::SliderFloat("VelocityScalar", &m_waterVelocityScalar, 0.0f, 100.0f, "%.2f");
	ImGui::SliderFloat("VelocityDamping", &m_waterVelocityDamping, 0.1f, 1.0f);
	ImGui::SliderFloat("RainRate", &m_rainRate, 0.00f, 0.00001f, "%.8f");
	ImGui::SliderFloat("EvapourationRate", &m_evapourationRate, 0.00f, 0.0001f, "%.8f");
	ImGui::SliderFloat("DrainRate", &m_drainRate, 0.0f, 0.001f, "%.5f");
	ImGui::SliderFloat("DrainMaxDepth", &m_drainMaxDepth, 0.0f, 0.05f, "%.3f");
	ImGui::SliderFloat("BoilSpeed", &m_boilSpeed, 0.0f, 0.001f, "%.4f");
	ImGui::Spacing();

	ImGui::Spacing();
	ImGui::Text("Dirt");
	ImGui::SliderFloat("Viscosity##dirt", &m_dirtViscosity, 0.01f, 1.0f);
	ImGui::SliderFloat("MaxSlope##dirt", &m_dirtMaxSlope, 0.0f, 0.01f, "%.5f");
	ImGui::SliderFloat("Density##dirt", &m_dirtDensity, 0.0f, 1.0f, "%.4f");
	ImGui::Spacing();

	ImGui::Spacing();
	ImGui::Text("Foam");
	ImGui::SliderFloat("SpawnStrength##foam", &m_foamSpawnStrength, 0.0f, 1.0f);
	ImGui::SliderFloat("DecayRate##foam", &m_foamDecayRate, 0.9f, 1.0f);
	ImGui::Spacing();

	ImGui::Text("Erosion");
	ImGui::SliderFloat("Strength", &m_erosionStrength, 0.0f, 0.000001f, "%.9f");
	ImGui::SliderFloat("DirtDepthMax", &m_erosionDirtDepthMax, 0.0f, 0.03f, "%.4f");
	ImGui::SliderFloat("WaterSpeedMax##erode", &m_erosionWaterSpeedMax, 0.0f, 0.1f, "%.3f");

	ImGui::Text("Dirt Transport");
	ImGui::SliderFloat("WaterSpeedMax##pickup", &m_dirtPickupMinWaterSpeed, 0.0f, 0.1f, "%.7f");
	ImGui::SliderFloat("PickupRate", &m_dirtPickupRate, 0.0f, 0.00001f, "%.7f");
	ImGui::SliderFloat("TransportSpeed", &m_dirtTransportSpeed, 0.0f, 0.1f, "%.4f");
	ImGui::SliderFloat("DepositRate", &m_dirtDepositSpeed, 0.0f, 0.05f, "%.7f");
	ImGui::SliderFloat("DissolvedDirtSmoothing", &m_dissolvedDirtSmoothing, 0.0f, 1.0f, "%.5f");
	ImGui::Spacing();

	if (ImGui::Button("Reset"))
	{
		ResetToDefaults();
	}
	if (ImGui::Button("Save"))
	{
		FlushChanges();
	}
}

//////////////////////////////////////////////////////////////////////////
// PRIVATE
//////////////////////////////////////////////////////////////////////////

void TerrainSimulationProcess::AdvanceTerrainSim
(
	TerrainGeometry & _geom, 
	TerrainMaterial & _material, 
	RenderTargetBase & _renderTarget,
	MoltenParticleGeom & _moltenParticleGeom
)
{
	IInputManager& inputManager = m_scene->GetInputManager();

	vec2 normalisedMousePos = inputManager.GetMousePosition();
	normalisedMousePos /= m_scene->GetWindow().GetWindowSize();

	float dirtScalar = inputManager.IsKeyDown(GLFW_KEY_LEFT_ALT) ? 1.0f : 0.0f;
	float waterScalar = inputManager.IsKeyDown(GLFW_KEY_LEFT_CONTROL) ? 1.0f : 0.0f && dirtScalar == 0.0f;
	float moltenScalar = (dirtScalar == 0.0f && waterScalar == 0.0f) ? 1.0f : 0.0f;
	float mouseIsDown = inputManager.IsMouseDown(1) ? 1.0f : 0.0f;
	
	float moltenVolumeAmount = mouseIsDown * m_mouseVolumeStrength * moltenScalar;
	float dirtVolumeAmount = mouseIsDown * m_mouseVolumeStrength * dirtScalar;
	float waterVolumeAmount = mouseIsDown * m_mouseVolumeStrength * waterScalar;
	float heatChangeAmount = mouseIsDown * m_mouseHeatStrength * moltenScalar;

	float phase = fmodf( (float)glfwGetTime() * _material.moltenFlowSpeed, 1.0f );
	float phaseA = fmodf( phase + 0.0f, 1.0f );
	float phaseB = fmodf( phase + 0.5f, 1.0f );

	bool m_phaseALatch = m_prevPhaseA > phaseA;
	bool m_phaseBLatch = m_prevPhaseB > phaseB;
	m_prevPhaseA = phaseA;
	m_prevPhaseB = phaseB;

	vec2 cellSize = vec2(_geom.Size() / (float)_geom.NumVerticesPerDimension());

	GL_CHECK(glViewport(0, 0, _geom.NumVerticesPerDimension(), _geom.NumVerticesPerDimension()));
	glDisable(GL_DEPTH_TEST);
	glDepthMask(true);
	glDepthFunc(GL_ALWAYS);

	
	{
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
		for (int i = 0; i < 10; ++i) 
		{
			Jacobi(_renderTarget, _geom.PressureData().GetRead(), _geom.DivergenceData(), cellSize, _geom.PressureData().GetWrite());
			_geom.PressureData().Swap();
		}
	}

	// Calculate derived values like normals and occclusion
	// This is done in a seperate step as it can be calculated in parallel to the update below.
	{
		m_deriveDataShader.BindPerPass();
		DeriveTerrainDataFrag& fragShader = m_deriveDataShader.FragmentShader();

		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, _geom.NormalData());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT1, _geom.DerivedData());
		static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		_renderTarget.SetDrawBuffers(drawBuffers, sizeof(drawBuffers) / sizeof(drawBuffers[0]));

		// Samplers
		fragShader.SetTexture("s_heightData",					_geom.HeightData().GetRead());

		// Misc
		fragShader.SetUniform("u_cellSize",						cellSize);
		fragShader.SetUniform("u_numHeightMips",				_geom.HeightData().GetRead().GetNumMipMaps());

		m_screenQuadGeom.Draw();
	}

	// Update Data
	{
		m_updateDataShader.BindPerPass();
		UpdateTerrainDataFrag& fragShader = m_updateDataShader.FragmentShader();

		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, _geom.HeightData().GetWrite());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT1, _geom.MiscData().GetWrite());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT2, _geom.SmudgeData().GetWrite());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT3, _geom.UVOffsetData().GetWrite());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT4, _geom.MoltenFluxData().GetWrite());

		static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
		_renderTarget.SetDrawBuffers(drawBuffers, sizeof(drawBuffers) / sizeof(drawBuffers[0]));
			
		// Samplers
		fragShader.SetTexture("s_heightData",					_geom.HeightData().GetRead());
		fragShader.SetTexture("s_miscData",						_geom.MiscData().GetRead());
		fragShader.SetTexture("s_smudgeData",					_geom.SmudgeData().GetRead());
		fragShader.SetTexture("s_waterFluxData",				_geom.WaterFluxData().GetRead());
		fragShader.SetTexture("s_uvOffsetData",					_geom.UVOffsetData().GetRead());
		fragShader.SetTexture("s_fluidVelocityData",			_geom.MoltenFluxData().GetRead());
		fragShader.SetTexture("s_albedoFluidGradient",			_material.albedoFluidGradient);
		fragShader.SetTexture("s_grungeMap",					_material.grungeTexture);
		fragShader.SetTexture( "s_pressureData",				_geom.PressureData().GetRead() );
		fragShader.SetTexture( "s_derivedData",					_geom.DerivedData() );
		
		// Mouse
		fragShader.SetUniform("u_mouseRadius",					m_mouseRadius);
		fragShader.SetUniform("u_mouseMoltenVolumeStrength",	moltenVolumeAmount);
		fragShader.SetUniform("u_mouseMoltenHeatStrength",		heatChangeAmount);
		fragShader.SetUniform("u_mouseWaterVolumeStrength",		waterVolumeAmount);
		fragShader.SetUniform("u_mouseDirtVolumeStrength",		dirtVolumeAmount);
		
		// Environment
		fragShader.SetUniform("u_time",							(float)glfwGetTime());
		fragShader.SetUniform("u_ambientTemp",					m_ambientTemperature);
		fragShader.SetUniform("u_phaseALatch",					m_phaseALatch);
		fragShader.SetUniform("u_phaseBLatch",					m_phaseBLatch);
		fragShader.SetUniform("u_cellSize",						cellSize.x);
		fragShader.SetUniform("u_dt",							m_timeStep);

		fragShader.SetUniform( "u_moltenPressureScale",			m_moltenPressureStrength );

		// Molten
		fragShader.SetUniform("u_moltenViscosity",				vec2(m_moltenViscosityMin, m_moltenViscosityMax));
		fragShader.SetUniform("u_rockMeltingPoint",				m_rockMeltingPoint);
		fragShader.SetUniform("u_heatViscosityScalar",			m_heatViscosityScalar);
		fragShader.SetUniform("u_tempChangeSpeed",				m_tempChangeSpeed);
		fragShader.SetUniform("u_meltCondenseSpeed",			m_meltCondenseSpeed);
		fragShader.SetUniform("u_meltSpeed",					m_meltSpeed);
		fragShader.SetUniform("u_condenseSpeed",				m_condenseSpeed);
		fragShader.SetUniform("u_mapHeightOffset",				_material.heightOffset);
		fragShader.SetUniform("u_smudgeChangeRate",				m_smudgeChangeRate);
		fragShader.SetUniform("u_moltenSlopeStrength",			m_moltenSlopeStrength);
		fragShader.SetUniform("u_moltenSlopePower",				m_moltenSlopePower);
		fragShader.SetUniform("u_moltenDiffuseStrength",		m_moltenDiffusionStrength);
		fragShader.SetUniform("u_moltenFluxDamping",			m_moltenFluxDamping);
		
		// Water
		fragShader.SetUniform("u_waterViscosity",				m_waterViscosity);
		fragShader.SetUniform("u_waterVelocityScalar",			m_waterVelocityScalar);
		fragShader.SetUniform("u_waterVelocityDamping",			m_waterVelocityDamping);
		fragShader.SetUniform("u_evapourationRate",				m_evapourationRate);
		fragShader.SetUniform("u_rainRate",						m_rainRate);
		fragShader.SetUniform("u_boilSpeed",					m_boilSpeed);
		fragShader.SetUniform("u_drainRate",					m_drainRate);
		fragShader.SetUniform("u_drainMaxDepth",				m_drainMaxDepth);

		// Dirt
		fragShader.SetUniform("u_dirtViscosity",				m_dirtViscosity);
		fragShader.SetUniform("u_dirtMaxSlope",					m_dirtMaxSlope);
		fragShader.SetUniform("u_dirtDensity",					m_dirtDensity);

		// Foam
		fragShader.SetUniform("u_foamSpawnStrength",			m_foamSpawnStrength);
		fragShader.SetUniform("u_foamDecayRate",				m_foamDecayRate);

		// Erosion
		fragShader.SetUniform("u_erosionStrength",				m_erosionStrength);
		fragShader.SetUniform("u_erosionMaxDepth",				m_erosionDirtDepthMax);
		fragShader.SetUniform("u_erosionWaterDepthMin",			m_erosionWaterDepthMin);
		fragShader.SetUniform("u_erosionWaterDepthMax",			m_erosionWaterDepthMax);
		fragShader.SetUniform("u_erosionWaterSpeedMax",			m_erosionWaterSpeedMax);

		// Dirt transport
		fragShader.SetUniform("u_dirtTransportSpeed",			m_dirtTransportSpeed);
		fragShader.SetUniform("u_dirtPickupMinWaterSpeed",		m_dirtPickupMinWaterSpeed);
		fragShader.SetUniform("u_dirtPickupRate",				m_dirtPickupRate);
		fragShader.SetUniform("u_dirtDepositSpeed",				m_dirtDepositSpeed);
		fragShader.SetUniform("u_dissolvedDirtSmoothing",		m_dissolvedDirtSmoothing);

		// Pass through the mouse position buffer
		TerrainMousePos mousePos;
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(mousePos), &mousePos);
		//PRINTF("mouse pos %d, %d, %d\n", mousePos.z, mousePos.u, mousePos.v);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _geom.MousePositionBuffer());

		m_screenQuadGeom.Draw();

		glFinish();

		_geom.HeightData().GetWrite().GenerateMipMaps();
		_geom.MiscData().GetWrite().GenerateMipMaps();
		_geom.SmudgeData().GetWrite().GenerateMipMaps();
		_geom.DerivedData().GenerateMipMaps();

		_geom.HeightData().Swap();
		_geom.MiscData().Swap();
		_geom.SmudgeData().Swap();
		_geom.UVOffsetData().Swap();
		_geom.MoltenFluxData().Swap();
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

void TerrainSimulationProcess::SubtractGradient(RenderTargetBase& renderTarget, TextureSquare & velocity,TextureSquare & pressure,TextureSquare & dest)
{
	
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
