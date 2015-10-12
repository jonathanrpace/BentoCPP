#pragma once

#include <typeinfo>

#include <ramen.h>
#include <kaiga.h>

namespace Kaiga
{
	class PlaneGeometry
		: public AbstractGeometry
		, public Ramen::SharedObject<PlaneGeometry>
	{
	public:
		PlaneGeometry();
		~PlaneGeometry();

		// From ISceneObject
		virtual const std::type_info & typeInfo() override;

		float GetWidth();
		void SetWidth(float _value);
		float GetHeight();
		void SetHeight(float _value);
		int GetNumDivisionsX();
		void SetNumDivisionsX(int _value);
		int GetNumDivisionsY();
		void SetNumDivisionsY(int _value);

	protected:
		// From AbstractValidatable
		virtual void Validate() override;

	private:
		float m_width;
		float m_height;
		int m_numDivisionsX;
		int m_numDivisionsY;

		
	};
}