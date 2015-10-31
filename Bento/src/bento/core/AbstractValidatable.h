#pragma once

namespace bento
{
	class AbstractValidatable
	{
	public:
		AbstractValidatable();
	protected:
		void Invalidate();
		void ValidateNow();
		virtual void Validate() = 0;
		virtual void OnInvalidate() = 0;
	private:
		bool m_isInvalid;
	};
}