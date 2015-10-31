#include "NamedObject.h"

void bento::NamedObject::SetName(char * _name)
{
	m_name = _name;
}

char * bento::NamedObject::GetName()
{
	return m_name;
}
