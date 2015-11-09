#include "InspectorUIProcess.h"

#include <typeinfo>

#include <imgui.h>

#include <bento/core/Scene.h>
#include <bento/core/Reflection.h>

namespace bento
{
	InspectorUIProcess::InspectorUIProcess(std::string _name)
		: Process(_name, typeid(InspectorUIProcess))
	{
	}

	void InspectorUIProcess::Advance(double dt)
	{
		ImGui::Begin("Inspector");
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
							AddControlsIfReflectable(componentPtr);

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
					AddControlsIfReflectable(process);

					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}

		ImGui::PopItemWidth();
		ImGui::End();
	}

	void InspectorUIProcess::AddControlsIfReflectable(ComponentPtr _component)
	{
		Reflectable* reflectable = dynamic_cast<Reflectable*>(_component.get());
		if (reflectable == nullptr) return;
		AddControls(reflectable);
	}

	void InspectorUIProcess::AddControlsIfReflectable(ProcessPtr _process)
	{
		Reflectable* reflectable = dynamic_cast<Reflectable*>(_process.get());
		if (reflectable == nullptr) return;
		AddControls(reflectable);
	}

	void InspectorUIProcess::AddControls(Reflectable* _reflectable)
	{
		for (size_t i = 0; i < _reflectable->GetReflectionInfo()->GetMembersCount(); i++)
		{
			auto member = _reflectable->GetReflectionInfo()->GetMembers()[i];

			if (member.m_typeInfo == typeid(float))
			{
				float* valuePtr = (float*)((size_t)_reflectable + member.m_offset);
				if (ImGui::SliderFloat(member.m_name, valuePtr, 0.0f, 1.0f))
				{
					
				}
			}
			else if (member.m_typeInfo == typeid(int))
			{
				int* valuePtr = (int*)((size_t)_reflectable + member.m_offset);
				if (ImGui::SliderInt(member.m_name, valuePtr,0,100))
				{
					
				}
			}

			else
			{
				ImGui::Text(member.m_name);
			}
		}
	}
}
