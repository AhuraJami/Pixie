#ifndef PIXIE_CONCEPTS_POBJECT_H
#define PIXIE_CONCEPTS_POBJECT_H

#ifdef __APPLE_CLANG__
#include <mpark/variant.hpp>
#else
#include <variant>
#endif

#include <vector>

#include "Pixie/Concepts/Object.h"
#include "Pixie/Concepts/Tickable.h"

namespace pixie
{

class PObject final
{
	friend class Tree;
	friend class Forest;

	/// Supported object types
#ifdef __APPLE_CLANG__
	using object_type = mpark::variant<Object, Tickable>;
#else
	using object_type = std::variant<Object, Tickable>;
#endif

public:
	/** Default constructor */
	PIXIE_EXPORT PObject() = default;

	template<class T>
	PIXIE_EXPORT PObject(T x)
	{
		using std::move;

		if(HasTick<T>)
		{
			Tickable obj = x;
			data = move(obj);
		}
		else
		{
			Object obj = x;
			data = move(obj);
		}
	}

	template<class T>
	PIXIE_EXPORT void Create()
	{
		using std::move;

		if(HasTick<T>)
		{
			Tickable obj;
			obj.Create<T>();
			data = move(obj);
		}
		else
		{
			Object obj;
			obj.Create<T>();
			data = move(obj);
		}
	}

	/** Copy constructor */
	PIXIE_EXPORT PObject(const PObject& object) = default;

	/** Default move constructor */
	PIXIE_EXPORT PObject(PObject&&) noexcept = default;

	/** Copy assignment operator */
	PIXIE_EXPORT PObject& operator=(const PObject& object)
	{ *this = PObject(object);	return *this; }

	/** Move assignment operator */
	PIXIE_EXPORT PObject& operator=(PObject&&) noexcept = default;

public:
	/**
	 * Returns a pointer to the type erased object that is stored here
	 * @tparam T (Required) Type of the object that is stored here
	 * @return On successful cast a pointer to the stored object;
	 * otherwise undefined behavior
	 * @warning Do NOT delete this pointer
	 * @remark Use this method only if you are certain that an object of
	 * type T resides in this class; else use DynamicCast<T> and check
	 * for nullptr access before unpacking the returned value
	 */
	template<class T>
	T* StaticCast()
	{
		using std::get;

		if(HasTick<T>)
		{
			return get<Tickable>(data).template StaticCast<T>();
		}
		else
		{
			return get<Object>(data).template StaticCast<T>();
		}
	}

private:
#ifdef __APPLE_CLANG__
#define VISIT_VARIANT(Visitor, Variant) mpark::visit(Visitor, Variant)
#else
#define VISIT_VARIANT(Visitor, Variant) std::visit(Visitor, Variant)
#endif
	friend void Begin(PObject& pobject)
	{
		VISIT_VARIANT([](auto&& arg) { Begin(arg); }, pobject.data);
	}

	friend void End(PObject& pobject)
	{
		VISIT_VARIANT([](auto&& arg) { End(arg); }, pobject.data);
	}

	struct TickVisitor
	{
		void operator()(Object&)
		{
		}

		void operator()(Tickable& object)
		{
			Tick(object);
		}
	};

	friend void Tick(PObject& pobject)
	{
		VISIT_VARIANT(TickVisitor(), pobject.data);
	}
#undef VISIT_VARIANT

private:
	/// type of the object this PObject holds
	object_type data{};
};

}
#endif //PIXIE_CONCEPTS_POBJECT_H
