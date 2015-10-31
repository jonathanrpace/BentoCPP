#include "AbstractValidatable.h"

bento::AbstractValidatable::AbstractValidatable() :
	m_isInvalid(true)
{
	
}

void bento::AbstractValidatable::Invalidate()
{
	if (m_isInvalid)
		return;
	m_isInvalid = true;
	OnInvalidate();
}

void bento::AbstractValidatable::ValidateNow()
{
	if (!m_isInvalid)
		return;
	Validate();
	m_isInvalid = false;
}
