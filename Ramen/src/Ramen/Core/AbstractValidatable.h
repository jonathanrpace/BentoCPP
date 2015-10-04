#pragma once

namespace Ramen
{
	class AbstractValidatable
	{
	private:
		bool m_isInvalid;
	protected:
		void Invalidate();
		void ValidateNow();
		virtual void Validate() = 0;
		virtual void OnInvalidate() = 0;
	};
}