#include "InspectorUIProcess.h"

#include <Ramen/Reflection.h>

const std::type_info & InspectorUIProcess::typeInfo()
{
	return typeid(InspectorUIProcess);
}

void InspectorUIProcess::BindToScene(Ramen::Scene * const _scene)
{
	m_scene = _scene;
}

void InspectorUIProcess::UnbindFromScene(Ramen::Scene * const _scene)
{
	m_scene = nullptr;
}

void InspectorUIProcess::Update(double dt)
{
	ImGui::Begin("Inspector");
	auto entities = m_scene->Entities();
	
	for (Ramen::Entity entity : entities)
	{
		//ImGui::PushID(int(entity));
		if ( ImGui::TreeNode((void*)entity, "Entity%d", (int)entity) )
		{
			auto components = m_scene->GetComponentsForEntity(entity);
			for (Ramen::ComponentPtr componentPtr : components)
			{
				ImGui::Text("Component");

				Ramen::Reflectable* reflectable = dynamic_cast<Ramen::Reflectable*>(componentPtr.get());
				if (reflectable != nullptr)
				{
					for (size_t i = 0; i < reflectable->GetReflectionInfo()->GetMembersCount(); i++)
					{
						auto member = reflectable->GetReflectionInfo()->GetMembers()[i];
						ImGui::Text(member.m_name);
					}
				}

				
			}
			ImGui::TreePop();
		}
		
	}
	ImGui::End();
}
