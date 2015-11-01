#pragma once

#include <string>
#include <typeinfo>

#include <bento.h>

namespace bento
{
	class SceneObject
	{
	public:
		SceneObject(std::string _name = "SceneObject");

		const std::string& Name();
		void Name(std::string _name);

		int ID();
		virtual const std::type_info& TypeInfo();

	private:
		std::string m_name;
		int m_id;

		static int s_id;
	};
}