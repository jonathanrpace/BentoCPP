#pragma once
#pragma once

#include <typeinfo>

namespace Ramen
{
	struct INode
	{

	};
}


struct TypeBase
{
	virtual void Marshal(void *, void const *) const = 0;
	virtual void Demarshal(void const *, void *) const = 0;
	virtual char const *name() const = 0;
	virtual size_t size() const = 0;
	virtual const std::type_info& typeInfo() const = 0;
};

template<typename T> struct Type : TypeBase
{
	static Type<T> instance;
	// custom marshaling is handled by template specialization
	void Marshal(void *dst, void const *src) const { memcpy(dst, src, sizeof(T)); }
	void Demarshal(void const *src, void *dst) const { memcpy(dst, src, sizeof(T)); }
	char const *name() const { return typeid(T).name(); }
	size_t size() const { return sizeof(T); }
	const std::type_info& typeInfo() const { return typeid(T); }
};

template <typename T> T& instance() {
	static T t;
	return t;
}

template<typename T, typename Q>
TypeBase *get_type(Q T::*mem) {
	return &instance<Type<Q> >();
}

struct member_t
{
	char const *name;
	TypeBase *type;
	size_t offset;
};

struct ReflectionBase
{
	void ReflectionConstruct();
	virtual size_t size() const = 0;
	virtual char const *name() const = 0;
	virtual size_t memberCount() const = 0;
	virtual member_t const *members() const = 0;
};

void ReflectionBase::ReflectionConstruct()
{
	members();
	memberCount();
	name();
	size();
}

#define MEMBER(x) \
   { #x, get_type(&T::x), (size_t)&((T*)0)->x },

#define RTTI(_type, _mems) \
template<typename T> struct _info : ReflectionBase \
{ \
    /* overrides used by ReflectionBase */ \
    inline size_t size() const \
	{ \
		return sizeof(T); \
	} \
\
    inline char const *name() const { return #_type; } \
    inline size_t memberCount() const { size_t cnt; get_members(cnt); return cnt; } \
    inline member_t const *members() const { size_t cnt; return get_members(cnt); } \
    _info() { ReflectionConstruct(); } \
    static inline member_t const *get_members(size_t &cnt) { \
      static member_t members[] = { _mems }; \
      cnt = sizeof(members) / sizeof(members[0]); \
      return members; \
    } \
    static inline _info<T> &info() { \
      return instance<_info<T> >(); \
    } \
  }; \
  inline static member_t const * members() { return _info<_type>::info().members(); } \
  static _info<_type> _theInfo; \
  inline static _info<_type> &info() { return _theInfo; }


#define DEFINE_NODE_1( NODE_NAME, TYPE0, NAME0 ) \
struct NODE_NAME : Ramen::INode \
{ \
	TYPE0 NAME0; \
	RTTI( NODE_NAME, MEMBER(NAME0)) \
}; \
NODE_NAME::_info<NODE_NAME> NODE_NAME::_theInfo;

#define DEFINE_NODE_2( NODE_NAME, TYPE0, NAME0, TYPE1, NAME1 ) \
struct NODE_NAME : Ramen::INode \
{ \
	TYPE0 NAME0; \
	TYPE1 NAME1; \
	RTTI( NODE_NAME, MEMBER(NAME0) MEMBER(NAME1)) \
}; \
NODE_NAME::_info<NODE_NAME> NODE_NAME::_theInfo;

#define DEFINE_NODE_3( NODE_NAME, TYPE0, NAME0, TYPE1, NAME1, TYPE2, NAME2 ) \
struct NODE_NAME : Ramen::INode \
{ \
	TYPE0 NAME0; \
	TYPE1 NAME1; \
	TYPE2 NAME2; \
	RTTI( NODE_NAME, MEMBER(NAME0) MEMBER(NAME1) MEMBER(NAME2) ) \
}; \
NODE_NAME::_info<NODE_NAME> NODE_NAME::_theInfo;

#define DEFINE_NODE_4( NODE_NAME, TYPE0, NAME0, TYPE1, NAME1, TYPE2, NAME2, TYPE3, NAME3 ) \
struct NODE_NAME : Ramen::INode \
{ \
	TYPE0 NAME0; \
	TYPE1 NAME1; \
	TYPE2 NAME2; \
	TYPE3 NAME3; \
	RTTI( NODE_NAME, MEMBER(NAME0) MEMBER(NAME1) MEMBER(NAME2) MEMBER(NAME3) ) \
}; \
NODE_NAME::_info<NODE_NAME> NODE_NAME::_theInfo;
