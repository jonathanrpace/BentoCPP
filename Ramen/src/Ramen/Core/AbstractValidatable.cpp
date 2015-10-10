#include "AbstractValidatable.h"

Ramen::AbstractValidatable::AbstractValidatable() :
	m_isInvalid(true)
{
	
}

void Ramen::AbstractValidatable::Invalidate()
{
	if (m_isInvalid)
		return;
	m_isInvalid = true;
}

void Ramen::AbstractValidatable::ValidateNow()
{
	if (!m_isInvalid)
		return;
	Validate();
	m_isInvalid = false;
}
