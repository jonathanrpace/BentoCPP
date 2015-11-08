#include "SceneObject.h"

namespace bento
{
	int SceneObject::s_id = 0;

	SceneObject::SceneObject
	(
		std::string _name,
		const std::type_info& _typeInfo
	)
		: m_name(_name)
		, m_typeInfo(_typeInfo)
	{
		m_id = s_id;
		s_id++;
	}
}
