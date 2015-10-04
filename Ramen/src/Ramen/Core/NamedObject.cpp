#include "NamedObject.h"

void Ramen::NamedObject::SetName(char * _name)
{
	m_name = _name;
}

char * Ramen::NamedObject::GetName()
{
	return m_name;
}
