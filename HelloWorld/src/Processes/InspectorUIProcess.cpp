#include "InspectorUIProcess.h"

#include <typeinfo>

#include <imgui.h>

#include <bento/core/Component.h>
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
		auto entities = m_scene->Entities();

		for (EntityPtr entity : entities)
		{
			//ImGui::PushID(int(entity));
			if (ImGui::TreeNode((void*)entity->ID(), entity->Name().c_str()))
			{
				auto components = m_scene->GetComponentsForEntity(entity);
				for (ComponentPtr componentPtr : components)
				{
					if (ImGui::TreeNode((void*)componentPtr->ID(), componentPtr->Name().c_str()))
					{
						Reflectable* reflectable = dynamic_cast<Reflectable*>(componentPtr.get());
						if (reflectable != nullptr)
						{
							for (size_t i = 0; i < reflectable->GetReflectionInfo()->GetMembersCount(); i++)
							{
								auto member = reflectable->GetReflectionInfo()->GetMembers()[i];
								ImGui::Text(member.m_name);
							}
						}
						ImGui::TreePop();
					}
				}
				ImGui::TreePop();
			}

		}
		ImGui::End();
	}
}
