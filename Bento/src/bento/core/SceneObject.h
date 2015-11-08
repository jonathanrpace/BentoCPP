#pragma once

#include <string>
#include <typeinfo>

#include <bento.h>

namespace bento
{
	class SceneObject
	{
	public:
		SceneObject(std::string _name = "SceneObject", const std::type_info& _typeInfo = typeid(SceneObject));

		const std::string& Name() { return m_name;  }
		void Name(std::string _name) { m_name = _name;  }
		int ID() { return m_id; }

		// From ITypedObject
		const std::type_info& TypeInfo() { return m_typeInfo; }

	private:
		std::string m_name;
		const std::type_info& m_typeInfo;
		int m_id;

		static int s_id;
	};
}