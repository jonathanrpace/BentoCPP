#pragma once

// std
#include <typeinfo>
#include <string>

// bento
#include <bento/core/SharedObject.h>
#include <bento/components/geom/Geometry.h>

namespace bento
{
	class PlaneGeometry
		: public Geometry
		, public bento::SharedObject<PlaneGeometry>
	{
	public:
		PlaneGeometry(std::string _name = "PlaneGeometry");

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