#pragma once

namespace bento
{
	class NamedObject
	{
	private:
		char* m_name;

	public:
		void SetName(char* _name);
		char* GetName();
	};
}