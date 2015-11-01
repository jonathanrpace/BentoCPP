#include "SceneObject.h"

namespace bento
{
	int SceneObject::s_id = 0;

	SceneObject::SceneObject(std::string _name)
		: m_name(_name)
	{
		m_id = s_id;
		s_id++;
	}

	const std::string& SceneObject::Name()
	{
		return m_name;
	}

	void SceneObject::Name(std::string _name)
	{
		m_name = _name;
	}

	int SceneObject::ID()
	{
		return m_id;
	}

	const std::type_info& SceneObject::TypeInfo()
	{
		return typeid(SceneObject);
	}
}
