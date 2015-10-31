#pragma once

#pragma once

#include <typeinfo>

struct ReflectionInfo;

namespace Ramen
{
	struct Reflectable
	{
		struct Member
		{
			char const* m_name;
			size_t m_offset;
			const std::type_info& m_typeInfo;
		};

		struct ReflectionInfo
		{
			virtual Member const* GetMembers() { return nullptr;  }
			virtual size_t GetMembersCount() { return 0; }
		};

		virtual ReflectionInfo * GetReflectionInfo()
		{
			static ReflectionInfo reflectionInfo;
			return &reflectionInfo;
		}
	};
}

#define MEMBER(T, TYPE, NAME)										\
	{ #NAME, (size_t)&((T*)0)->NAME, typeid(TYPE) },				\

#define DEFINE_REFLECTION_INFO(TYPE, MEMBERS)						\
	public:															\
	struct TYPE##ReflectionInfo : ReflectionInfo					\
	{																\
	private:														\
		Member const m_members[1] = { MEMBERS };					\
	public:															\
		virtual Member const* GetMembers() { return m_members; }	\
		virtual size_t GetMembersCount() { return 1; }				\
	};																\
	virtual ReflectionInfo* GetReflectionInfo() override			\
	{																\
		static StandardMaterialReflectionInfo reflectionInfo;		\
		return &reflectionInfo;										\
	}																\

#define DEFINE_REFLECTION_INFO_1(T, TYPE_0, NAME_0))				\
	DEFINE_REFLECTION_INFO(T, MEMBER(T, TYPE_0, NAME_0))			\

#define DEFINE_REFLECTION_INFO_2(T, TYPE_0, NAME_0, TYPE_1, NAME_1)					\
	DEFINE_REFLECTION_INFO(T, MEMBER(T, TYPE_0, NAME_0) MEMBER(T, TYPE_1, NAME_1))	\

#define DEFINE_REFLECTION_INFO_3(T, TYPE_0, NAME_0, TYPE_1, NAME_1, TYPE_2, NAME_2)								\
	DEFINE_REFLECTION_INFO(T, MEMBER(T, TYPE_0, NAME_0) MEMBER(T, TYPE_1, NAME_1) MEMBER(T, TYPE_2, NAME_2))	\