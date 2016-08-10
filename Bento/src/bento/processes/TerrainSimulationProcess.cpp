#include "TerrainSimulationProcess.h"

#include <bento/core/Logging.h>
#include <glfw3.h>

#include <utility>

#include <imgui.h>

namespace bento
{
//////////////////////////////////////////////////////////////////////////
UpdateTerrainFluxFrag::UpdateTerrainFluxFrag()
	: ShaderStageBase("shaders/UpdateTerrainFlux.frag")
{}

//////////////////////////////////////////////////////////////////////////
UpdateTerrainDataFrag::UpdateTerrainDataFrag()
	: ShaderStageBase("shaders/UpdateTerrainData.frag")
{}

//////////////////////////////////////////////////////////////////////////
DiffuseHeightFrag::DiffuseHeightFrag()
	: ShaderStageBase("shaders/DiffuseHeight.frag")
{}

//////////////////////////////////////////////////////////////////////////
MoltenParticleUpdateVert::MoltenParticleUpdateVert()
	: ShaderStageBase("shaders/MoltenParticleUpdate.vert", false)
{}

void MoltenParticleUpdateVert::OnPreLink()
{
	const char * varyings[] = { "out_position", "out_angle" };
	GL_CHECK(glTransformFeedbackVaryings(m_programName, 2, varyings, GL_SEPARATE_ATTRIBS));
}

//////////////////////////////////////////////////////////////////////////
MoltenMapVert::MoltenMapVert()
	: ShaderStageBase("shaders/MoltenMap.vert", false)
{}

//////////////////////////////////////////////////////////////////////////
MoltenMapFrag::MoltenMapFrag() 
	: ShaderStageBase("shaders/MoltenMap.frag", false)
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
	SerializableMember("moltenFluxDamping",		0.99f,		&m_moltenFluxDamping);
	SerializableMember("moltenViscosity",		0.5f,		&m_moltenViscosity);
	SerializableMember("rockMeltingPoint",		0.3f,		&m_rockMeltingPoint);
	SerializableMember("textureScrollSpeed",	0.04f,		&m_textureScrollSpeed);
	SerializableMember("textureCycleSpeed",		0.003f,		&m_textureCycleSpeed);
	SerializableMember("heatAdvectSpeed",		0.5f,		&m_heatAdvectSpeed);
	SerializableMember("heatDiffuseStrength",	0.5f,		&m_heatDiffuseStrength);
	SerializableMember("meltSpeed",				0.00001f,	&m_meltSpeed);
	SerializableMember("condenseSpeed",			0.01f,		&m_condenseSpeed);
	SerializableMember("tempChangeSpeed",		0.002f,		&m_tempChangeSpeed);
	SerializableMember("moltenVelocityScalar",	1.0f,		&m_moltenVelocityScalar);
	SerializableMember("moltenVelocityDamping",	0.9f,		&m_moltenVelocityDamping);

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
	SerializableMember("dirtDiffuseStrength",	0.05f,		&m_dirtDiffuseStrength);
	SerializableMember("waterDiffuseStrength",	0.00f,		&m_dissolvedDirtSmoothing);

	// Vegetation
	SerializableMember("m_vegMinDirt",			0.001f,		&m_vegMinDirt);
	SerializableMember("m_vegMaxDirt",			0.002f,		&m_vegMaxDirt);
	SerializableMember("m_vegGrowthRate",		0.001f,		&m_vegGrowthRate);
	SerializableMember("m_vegMinSlope",			0.1f,		&m_vegMinSlope);
	SerializableMember("m_vegMaxSlope",			0.2f,		&m_vegMaxSlope);


	// Global
	SerializableMember("ambientTemperature",	0.05f,		&m_ambientTemperature);

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
	ImGui::Spacing();

	ImGui::Text("Molten");
	ImGui::SliderFloat("FluxDamping", &m_moltenFluxDamping, 0.9f, 1.0f);
	ImGui::SliderFloat("VelocityScalar##molten", &m_moltenVelocityScalar, 0.0f, 4.0f);
	ImGui::SliderFloat("VelocityDamping##molten", &m_moltenVelocityDamping, 0.8f, 1.0f);
	ImGui::Spacing();

	ImGui::Spacing();
	ImGui::SliderFloat("MoltenViscosity", &m_moltenViscosity, 0.01f, 0.5f);
	ImGui::SliderFloat("MeltingPoint", &m_rockMeltingPoint, 0.0f, 2.0f);
	ImGui::SliderFloat("HeatAdvectSpeed", &m_heatAdvectSpeed, 0.0f, 2.0f);
	ImGui::SliderFloat("HeatDiffuseStrength", &m_heatDiffuseStrength, 0.0f, 1.0f);
	ImGui::Spacing();

	ImGui::Spacing();
	ImGui::SliderFloat("TempChangeSpeed", &m_tempChangeSpeed, 0.0f, 0.01f, "%.5f");
	ImGui::SliderFloat("MeltSpeed", &m_meltSpeed, 0.0f, 0.0001f, "%.5f");
	ImGui::SliderFloat("CondenseSpeed", &m_condenseSpeed, 0.0f, 0.1f, "%.5f");
	ImGui::Spacing();

	ImGui::Spacing();
	ImGui::SliderFloat("ScrollSpeed", &m_textureScrollSpeed, 0.0f, 0.2f);
	ImGui::SliderFloat("TextureCycleSpeed", &m_textureCycleSpeed, 0.0f, 0.1f);
	ImGui::Spacing();

	ImGui::Spacing();
	ImGui::Text("Water");
	ImGui::SliderFloat("FluxDamping2", &m_waterFluxDamping, 0.9f, 1.0f);
	ImGui::SliderFloat("Viscosity", &m_waterViscosity, 0.01f, 0.5f);
	ImGui::SliderFloat("EvapourationRate", &m_evapourationRate, 0.00f, 0.0001f, "%.8f");
	ImGui::SliderFloat("RainRate", &m_rainRate, 0.00f, 0.000002f, "%.8f");
	ImGui::SliderFloat("VelocityScalar", &m_waterVelocityScalar, 0.0f, 50.0f, "%.2f");
	ImGui::SliderFloat("VelocityDamping", &m_waterVelocityDamping, 0.1f, 1.0f, "%.3f");
	ImGui::SliderFloat("BoilSpeed", &m_boilSpeed, 0.0f, 0.001f, "%.4f");
	ImGui::SliderFloat("DrainRate", &m_drainRate, 0.0f, 0.001f, "%.5f");
	ImGui::SliderFloat("DrainMaxDepth", &m_drainMaxDepth, 0.0f, 0.05f, "%.3f");
	ImGui::Spacing();

	ImGui::Text("Erosion");
	ImGui::SliderFloat("Strength", &m_erosionStrength, 0.0f, 0.0001f, "%.6f");
	ImGui::SliderFloat("DirtDepthMax", &m_erosionDirtDepthMax, 0.0f, 0.1f, "%.4f");
	ImGui::SliderFloat("WaterDepthMin", &m_erosionWaterDepthMin, 0.0f, 0.001f, "%.5f");
	ImGui::SliderFloat("WaterDepthMax", &m_erosionWaterDepthMax, 0.0f, 0.001f, "%.5f");
	ImGui::SliderFloat("WaterSpeedMax", &m_erosionWaterSpeedMax, 0.0f, 0.1f, "%.3f");
	ImGui::SliderFloat("DirtSmoothing", &m_dirtDiffuseStrength, 0.0f, 0.1f, "%.5f");

	ImGui::Text("Dirt Transport");
	ImGui::SliderFloat("PickupMinWaterSpeed", &m_dirtPickupMinWaterSpeed, 0.0f, 0.1f, "%.7f");
	ImGui::SliderFloat("PickupRate", &m_dirtPickupRate, 0.0f, 0.0005f, "%.7f");
	ImGui::SliderFloat("TransportSpeed", &m_dirtTransportSpeed, 0.0f, 0.01f, "%.4f");
	ImGui::SliderFloat("DepositSpeed", &m_dirtDepositSpeed, 0.0f, 0.1f, "%.7f");
	ImGui::SliderFloat("DissolvedDirtSmoothing", &m_dissolvedDirtSmoothing, 0.0f, 1.0f, "%.5f");
	ImGui::Spacing();

	ImGui::Text("Vegetation");
	ImGui::SliderFloat("MinDirt#veg", &m_vegMinDirt, 0.0f, 0.05f, "%.5f");
	ImGui::SliderFloat("MaxDirt#veg", &m_vegMaxDirt, 0.0f, 0.05f, "%.5f");
	ImGui::SliderFloat("MinSlope#veg", &m_vegMinSlope, 0.0f, 1.00f, "%.5f");
	ImGui::SliderFloat("MaxSlope#veg", &m_vegMaxSlope, 0.0f, 1.00f, "%.5f");
	ImGui::SliderFloat("GrowthRate#veg", &m_vegGrowthRate, 0.0f, 0.01f, "%.5f");
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
	GL_CHECK(glViewport(0, 0, _geom.NumVerticesPerDimension(), _geom.NumVerticesPerDimension()));

	IInputManager& inputManager = m_scene->GetInputManager();

	vec2 normalisedMousePos = inputManager.GetMousePosition();
	normalisedMousePos /= m_scene->GetWindow().GetWindowSize();

	bool mouseIsDown = inputManager.IsMouseDown(1);

	float moltenScalar = inputManager.IsKeyDown(GLFW_KEY_LEFT_CONTROL) ? 0.0f : 1.0f;
	float moltenVolumeAmount = (inputManager.IsMouseDown(1) ? 1.0f : 0.0f) * m_mouseVolumeStrength * moltenScalar;
	float heatChangeAmount = (inputManager.IsMouseDown(1) ? 1.0f : 0.0f) * m_mouseHeatStrength * moltenScalar;

	float waterScalar = inputManager.IsKeyDown(GLFW_KEY_LEFT_CONTROL) ? 1.0f : 0.0f;
	float waterVolumeAmount = (inputManager.IsMouseDown(1) ? 1.0f : 0.0f) * m_mouseVolumeStrength * waterScalar;

	vec2 cellSize = vec2(_geom.Size() / (float)_geom.NumVerticesPerDimension());

	glDisable(GL_DEPTH_TEST);
	glDepthMask(true);
	glDepthFunc(GL_ALWAYS);

	// Update Flux
	{
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, _geom.RockFluxData().GetWrite());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT1, _geom.WaterFluxData().GetWrite());

		static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		_renderTarget.SetDrawBuffers(drawBuffers, sizeof(drawBuffers) / sizeof(drawBuffers[0]));

		m_updateFluxShader.BindPerPass();
		auto fragShader = m_updateFluxShader.FragmentShader();

		fragShader.SetTexture("s_heightData", _geom.HeightData().GetRead());
		fragShader.SetTexture("s_miscData", _geom.MiscData().GetRead());
		fragShader.SetTexture("s_rockFluxData", _geom.RockFluxData().GetRead());
		fragShader.SetTexture("s_waterFluxData", _geom.WaterFluxData().GetRead());

		fragShader.SetUniform("u_rockFluxDamping", m_moltenFluxDamping);
		fragShader.SetUniform("u_waterFluxDamping", m_waterFluxDamping);
		fragShader.SetUniform("u_rockMeltingPoint", m_rockMeltingPoint);
		fragShader.SetUniform("u_mapHeightOffset",	_material.moltenMapOffset);

		m_screenQuadGeom.Draw();

		_geom.RockFluxData().Swap();
		_geom.WaterFluxData().Swap();

		_geom.RockFluxData().GetRead().GenerateMipMaps();
		_geom.WaterFluxData().GetRead().GenerateMipMaps();
	}


	// Update Data
	{
		m_updateDataShader.BindPerPass();
		auto fragShader = m_updateDataShader.FragmentShader();

		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, _geom.HeightData().GetWrite());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT1, _geom.VelocityData().GetWrite());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT2, _geom.MiscData().GetWrite());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT3, _geom.NormalData().GetWrite());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT4, _geom.SmudgeData().GetWrite());

		static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
		_renderTarget.SetDrawBuffers(drawBuffers, sizeof(drawBuffers) / sizeof(drawBuffers[0]));
			
		// Samplers
		fragShader.SetTexture("s_heightData",					_geom.HeightData().GetRead());
		fragShader.SetTexture("s_velocityData",					_geom.VelocityData().GetRead());
		fragShader.SetTexture("s_miscData",						_geom.MiscData().GetRead());
		fragShader.SetTexture("s_normalData",					_geom.NormalData().GetRead());
		fragShader.SetTexture("s_moltenMapData",				_geom.MoltenMapData().GetRead());
		fragShader.SetTexture("s_smudgeData",					_geom.SmudgeData().GetRead());
		fragShader.SetTexture("s_waterFluxData",				_geom.WaterFluxData().GetRead());
		fragShader.SetTexture("s_rockFluxData",					_geom.RockFluxData().GetRead());
		fragShader.SetTexture("s_diffuseMap",					_material.someTexture);

		// Mouse
		fragShader.SetUniform("u_mousePos",						normalisedMousePos);
		fragShader.SetUniform("u_mouseRadius",					m_mouseRadius);
		fragShader.SetUniform("u_mouseMoltenVolumeStrength",	moltenVolumeAmount);
		fragShader.SetUniform("u_mouseMoltenHeatStrength",		heatChangeAmount);
		fragShader.SetUniform("u_mouseWaterVolumeStrength",		waterVolumeAmount);
		
		// Environment
		fragShader.SetUniform("u_ambientTemp",					m_ambientTemperature);

		// Molten
		fragShader.SetUniform("u_heatAdvectSpeed",				m_heatAdvectSpeed);
		fragShader.SetUniform("u_moltenViscosity",				m_moltenViscosity);
		fragShader.SetUniform("u_rockMeltingPoint",				m_rockMeltingPoint);
		fragShader.SetUniform("u_tempChangeSpeed",				m_tempChangeSpeed);
		fragShader.SetUniform("u_condenseSpeed",				m_condenseSpeed);
		fragShader.SetUniform("u_meltSpeed",					m_meltSpeed);
		fragShader.SetUniform("u_moltenVelocityScalar",			m_moltenVelocityScalar);
		fragShader.SetUniform("u_moltenVelocityDamping",		m_moltenVelocityDamping);
		fragShader.SetUniform("u_mapHeightOffset",				_material.moltenMapOffset);

		// Water
		fragShader.SetUniform("u_waterViscosity",				m_waterViscosity);
		fragShader.SetUniform("u_waterVelocityScalar",			m_waterVelocityScalar);
		fragShader.SetUniform("u_waterVelocityDamping",			m_waterVelocityDamping);
		fragShader.SetUniform("u_evapourationRate",				m_evapourationRate);
		fragShader.SetUniform("u_rainRate",						m_rainRate);
		fragShader.SetUniform("u_boilSpeed",					m_boilSpeed);
		fragShader.SetUniform("u_drainRate",					m_drainRate);
		fragShader.SetUniform("u_drainMaxDepth",				m_drainMaxDepth);

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

		// Vegetation
		fragShader.SetUniform("u_vegMinDirt",					m_vegMinDirt);
		fragShader.SetUniform("u_vegMaxDirt",					m_vegMaxDirt);
		fragShader.SetUniform("u_vegMinSlope",					m_vegMinSlope);
		fragShader.SetUniform("u_vegMaxSlope",					m_vegMaxSlope);
		fragShader.SetUniform("u_vegGrowthRate",				m_vegGrowthRate);
		fragShader.SetUniform("u_vegBump",						_material.vegBump);

		// Misc
		fragShader.SetUniform("u_textureScrollSpeed",			m_textureScrollSpeed);
		fragShader.SetUniform("u_cycleSpeed",					m_textureCycleSpeed);
		fragShader.SetUniform("u_cellSize",						cellSize);
		fragShader.SetUniform("u_numHeightMips",				_geom.HeightData().GetRead().GetNumMipMaps());
		

		// Pass through the mouse position buffer
		TerrainMousePos mousePos;
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(mousePos), &mousePos);
		//PRINTF("mouse pos %d, %d, %d\n", mousePos.z, mousePos.u, mousePos.v);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _geom.MousePositionBuffer());

		m_screenQuadGeom.Draw();

		_geom.HeightData().GetWrite().GenerateMipMaps();
		_geom.NormalData().GetWrite().GenerateMipMaps();
		_geom.MiscData().GetWrite().GenerateMipMaps();

		_geom.HeightData().Swap();
		_geom.VelocityData().Swap();
		_geom.MiscData().Swap();
		_geom.NormalData().Swap();
		_geom.SmudgeData().Swap();
	}

	
	// Diffuse height
	{
		static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		m_diffuseHeightShader.BindPerPass();
		auto fragShader = m_diffuseHeightShader.FragmentShader();

		// Shared Uniforms
		fragShader.SetUniform("u_dirtDiffuseStrength", m_dirtDiffuseStrength);
		fragShader.SetUniform("u_waterDiffuseStrength", m_dissolvedDirtSmoothing);
		fragShader.SetUniform("u_heatDiffuseStrength", m_heatDiffuseStrength);
		fragShader.SetUniform("u_dissolvedDirtDiffuseStrength", m_dissolvedDirtSmoothing);
		fragShader.SetUniform("u_mapHeightOffset", _material.moltenMapOffset);

		// X Pass

		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, _geom.HeightData().GetWrite());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT1, _geom.VelocityData().GetWrite());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT2, _geom.MiscData().GetWrite());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT3, _geom.SmudgeData().GetWrite());
		_renderTarget.SetDrawBuffers(drawBuffers, sizeof(drawBuffers) / sizeof(drawBuffers[0]));

		// Uniforms
		fragShader.SetTexture("s_heightData", _geom.HeightData().GetRead());
		fragShader.SetTexture("s_velocityData", _geom.VelocityData().GetRead());
		fragShader.SetTexture("s_miscData", _geom.MiscData().GetRead());
		fragShader.SetTexture("s_smudgeData", _geom.SmudgeData().GetRead());
		fragShader.SetUniform("u_axis", ivec2(1, 0));

		m_screenQuadGeom.Draw();

		_geom.HeightData().GetWrite().GenerateMipMaps();
		_geom.MiscData().GetWrite().GenerateMipMaps();

		_geom.HeightData().Swap();
		_geom.VelocityData().Swap();
		_geom.MiscData().Swap();
		_geom.SmudgeData().Swap();
		
		// Y Pass

		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, _geom.HeightData().GetWrite());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT1, _geom.VelocityData().GetWrite());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT2, _geom.MiscData().GetWrite());
		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT3, _geom.SmudgeData().GetWrite());
		_renderTarget.SetDrawBuffers(drawBuffers, sizeof(drawBuffers) / sizeof(drawBuffers[0]));

		// Uniforms
		fragShader.SetTexture("s_heightData", _geom.HeightData().GetRead());
		fragShader.SetTexture("s_velocityData", _geom.VelocityData().GetRead());
		fragShader.SetTexture("s_miscData", _geom.MiscData().GetRead());
		fragShader.SetTexture("s_smudgeData", _geom.SmudgeData().GetRead());
		fragShader.SetUniform("u_axis", ivec2(0, 1));

		m_screenQuadGeom.Draw();

		_geom.HeightData().GetWrite().GenerateMipMaps();
		_geom.MiscData().GetWrite().GenerateMipMaps();

		_geom.HeightData().Swap();
		_geom.VelocityData().Swap();
		_geom.MiscData().Swap();
		_geom.SmudgeData().Swap();
	}

	// Update molten particles
	{
		glBindProgramPipeline(GL_NONE);

		m_moltenParticleUpdateShader.BindPerPass();
			
		if (_moltenParticleGeom.Switch())
		{
			GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, _moltenParticleGeom.TransformFeedbackObjB()));	// TODO: Make this work like read/write texture buffers
			GL_CHECK(glBindVertexArray(_moltenParticleGeom.VertexArrayA()));
		}
		else
		{
			GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, _moltenParticleGeom.TransformFeedbackObjA()));
			GL_CHECK(glBindVertexArray(_moltenParticleGeom.VertexArrayB()));
		}

		GL_CHECK(glBeginTransformFeedback(GL_POINTS));
		GL_CHECK(glEnable(GL_RASTERIZER_DISCARD));

		auto vertexShader = m_moltenParticleUpdateShader.VertexShader();

		vertexShader.SetTexture("s_heightData", _geom.HeightData().GetRead());
		vertexShader.SetTexture("s_velocityData", _geom.VelocityData().GetRead());

		GL_CHECK(glDrawArrays(GL_POINTS, 0, _moltenParticleGeom.NumParticles()));

		GL_CHECK(glDisable(GL_RASTERIZER_DISCARD));
		GL_CHECK(glEndTransformFeedback());
		glUseProgram(GL_NONE);

		_moltenParticleGeom.Switch(!_moltenParticleGeom.Switch());
	}

	// Render molten particles to a map
	{
		m_moltenMapShader.BindPerPass();
		auto fragShader = m_moltenMapShader.FragmentShader();
		auto vertShader = m_moltenMapShader.VertexShader();

		_renderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, _geom.MoltenMapData().GetWrite());

		static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
		_renderTarget.SetDrawBuffers(drawBuffers, sizeof(drawBuffers) / sizeof(drawBuffers[0]));

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glDepthFunc(GL_GREATER);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(true);

		//glEnable(GL_BLEND);
		//glEnable(GL_PROGRAM_POINT_SIZE);
		//glBlendEquation(GL_MAX);
		//glBlendFunc(GL_ONE, GL_ONE);

		

		vertShader.SetTexture("s_velocityData", _geom.VelocityData().GetRead() );

		fragShader.SetTexture("s_texture", _material.moltenPlatesTexture);

		//glPointSize(16.0f);

		glEnable(GL_PROGRAM_POINT_SIZE);

		GL_CHECK(glDrawArrays(GL_POINTS, 0, _moltenParticleGeom.NumParticles()));

		//glDisable(GL_BLEND);
		glDisable(GL_PROGRAM_POINT_SIZE);

		//glDisable(GL_DEPTH_TEST);
		//glDepthMask(false);



		_geom.MoltenMapData().GetWrite().GenerateMipMaps();
		_geom.MoltenMapData().Swap();
	}
}

void TerrainSimulationProcess::OnNodeAdded(const TerrainSimPassNode & _node)
{
	RenderTargetBase* renderTarget = new RenderTargetBase
	(
		_node.geom->NumVerticesPerDimension(),
		_node.geom->NumVerticesPerDimension(),
		false, true, GL_RGBA16F
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
