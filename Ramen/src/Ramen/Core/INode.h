#pragma once
#pragma once

#include <typeinfo>

namespace Ramen
{
	struct INode
	{
		
	};
}


#define MEMBER(NodeType, MemberType, MemberName) \
{ #MemberName, (size_t)&((NodeType*)0)->MemberName, typeid(MemberType) },

#define DEFINE_NODE_COMMON(NODE_TYPE, MEMBERS, MEMBERS_COUNT)\
	struct Member \
	{ \
		char const* m_name; \
		size_t m_offset; \
		const std::type_info& m_typeInfo; \
	}; \
	struct ReflectionInfo \
	{ \
		inline Member const* GetMembers() \
		{ \
			return m_members; \
		} \
		inline size_t GetMembersCount() \
		{ \
			return MEMBERS_COUNT; \
		} \
	private: \
		Member const m_members[MEMBERS_COUNT] = { MEMBERS }; \
	}; \
	static ReflectionInfo & GetReflectionInfo() \
	{ \
		static ReflectionInfo reflectionInfo; \
		return reflectionInfo; \
	} \
}; \

#define DEFINE_NODE_1(NODE_TYPE, MEMBER_TYPE_0, MEMBER_NAME_0)\
struct NODE_TYPE : Ramen::INode \
{\
	MEMBER_TYPE_0* MEMBER_NAME_0; \
	DEFINE_NODE_COMMON(NODE_TYPE, MEMBER(NODE_TYPE, MEMBER_TYPE_0, MEMBER_NAME_0), 1) \

#define DEFINE_NODE_2(NODE_TYPE, MEMBER_TYPE_0, MEMBER_NAME_0, MEMBER_TYPE_1, MEMBER_NAME_1)\
struct NODE_TYPE : Ramen::INode \
{\
	MEMBER_TYPE_0* MEMBER_NAME_0; \
	MEMBER_TYPE_1* MEMBER_NAME_1; \
	DEFINE_NODE_COMMON(NODE_TYPE, MEMBER(NODE_TYPE, MEMBER_TYPE_0, MEMBER_NAME_0) MEMBER(NODE_TYPE, MEMBER_TYPE_1, MEMBER_NAME_1), 2) \

#define DEFINE_NODE_3(NODE_TYPE, MEMBER_TYPE_0, MEMBER_NAME_0, MEMBER_TYPE_1, MEMBER_NAME_1, MEMBER_TYPE_2, MEMBER_NAME_2)\
struct NODE_TYPE : Ramen::INode \
{\
	MEMBER_TYPE_0* MEMBER_NAME_0; \
	MEMBER_TYPE_1* MEMBER_NAME_1; \
	MEMBER_TYPE_2* MEMBER_NAME_2; \
	DEFINE_NODE_COMMON(NODE_TYPE, MEMBER(NODE_TYPE, MEMBER_TYPE_0, MEMBER_NAME_0) MEMBER(NODE_TYPE, MEMBER_TYPE_1, MEMBER_NAME_1) MEMBER(NODE_TYPE, MEMBER_TYPE_2, MEMBER_NAME_2), 3) \

/* Example output of:
DEFINE_NODE_3( Node, int, var0, bool, var1, float, var2 )

struct Node : Ramen::INode
{
	int* var0;
	bool* var1;
	float* var2;

	struct Member
	{
		char const* m_name;
		size_t m_offset;
		const std::type_info& m_typeInfo;
	};

	struct ReflectionInfo
	{
		inline Member const* GetMembers()
		{
			return m_members;
		}
		inline size_t GetMembersCount()
		{
			return 3;
		}
	private:
		Member const m_members[3] = {
			{ "var0", (size_t)&((Node*)0)->var0, typeid(int) },
			{ "var1", (size_t)&((Node*)0)->var1, typeid(bool) },
			{ "var2", (size_t)&((Node*)0)->var2, typeid(float) },
		};
	};

	static ReflectionInfo & GetReflectionInfo()
	{
		static ReflectionInfo reflectionInfo;
		return s_reflectionInfo;
	}
};

*/