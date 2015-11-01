#include "InspectorUIProcess.h"

#include <bento/core/Reflection.h>

const std::type_info & InspectorUIProcess::TypeInfo()
{
	return typeid(InspectorUIProcess);
}

void InspectorUIProcess::BindToScene(bento::Scene * const _scene)
{
	m_scene = _scene;
}

void InspectorUIProcess::UnbindFromScene(bento::Scene * const _scene)
{
	m_scene = nullptr;
}

void InspectorUIProcess::Update(double dt)
{
	ImGui::Begin("Inspector");
	auto entities = m_scene->Entities();
	
	for (bento::EntityPtr entity : entities)
	{
		//ImGui::PushID(int(entity));
		if ( ImGui::TreeNode((void*)entity->ID(), entity->Name().c_str()) )
		{
			auto components = m_scene->GetComponentsForEntity(entity);
			for (bento::ComponentPtr componentPtr : components)
			{
				if ( ImGui::TreeNode((void*)componentPtr->ID(), componentPtr->Name().c_str()) )
				{
					bento::Reflectable* reflectable = dynamic_cast<bento::Reflectable*>(componentPtr.get());
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
