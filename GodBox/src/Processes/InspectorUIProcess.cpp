#include "InspectorUIProcess.h"

#include <typeinfo>

#include <imgui.h>

#include <bento/core/Scene.h>
#include <bento/core/IInspectable.h>
#include <bento/render/RendererBase.h>

namespace bento
{
	InspectorUIProcess::InspectorUIProcess(std::string _name)
		: Process(_name, typeid(InspectorUIProcess))
	{
	}

	void InspectorUIProcess::Advance(double dt)
	{
		ivec2 backBufferSize = m_scene->GetWindow().GetWindowSize();
		bool opened = true;
		ImGui::SetNextWindowSize(ImVec2(500.0f, (float)backBufferSize.y));
		ImGui::SetNextWindowPos(ImVec2());
		
		ImGui::Begin("Inspector", &opened, 
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | 
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoTitleBar );
		
		ImGui::PushItemWidth(-140.0f);
		if (ImGui::TreeNode("Entities"))
		{
			auto entities = m_scene->Entities();
			for (EntityPtr entity : entities)
			{
				if (ImGui::TreeNode((void*)entity->ID(), entity->Name().c_str()))
				{
					auto components = m_scene->GetComponentsForEntity(entity);
					for (ComponentPtr componentPtr : components)
					{
						if (ImGui::TreeNode((void*)componentPtr->ID(), componentPtr->Name().c_str()))
						{
							AddControlsIfInspectable(componentPtr);

							ImGui::TreePop();
						}
					}
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}

		if ( ImGui::TreeNode("Processes") )
		{
			auto processes = m_scene->Processes();
			for (ProcessPtr process : processes)
			{
				if (ImGui::TreeNode((void*)process->ID(), process->Name().c_str()))
				{
					AddControlsIfInspectable(process);

					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}

		if ( ImGui::TreeNode("Render Passes") )
		{
			auto renderProcess = m_scene->GetProcess<RendererBase>();
			auto renderPasses = renderProcess->RenderPasses();
			for (RenderPassPtr renderPass : renderPasses)
			{
				if (ImGui::TreeNode((void*)renderPass->ID(), renderPass->Name().c_str()))
				{
					AddControlsIfInspectable(renderPass);

					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}

		ImGui::PopItemWidth();
		ImGui::End();
	}

	void InspectorUIProcess::AddControlsIfInspectable(ComponentPtr _component)
	{
		IInspectable* inspectable = dynamic_cast<IInspectable*>(_component.get());
		if (inspectable == nullptr) return;
		inspectable->AddUIElements();
	}

	void InspectorUIProcess::AddControlsIfInspectable(ProcessPtr _process)
	{
		IInspectable* inspectable = dynamic_cast<IInspectable*>(_process.get());
		if (inspectable == nullptr) return;
		inspectable->AddUIElements();
	}
}
