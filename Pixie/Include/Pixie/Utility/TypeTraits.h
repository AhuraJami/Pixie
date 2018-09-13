#ifndef PIXIE_UTILITY_TYPE_TRAITS_H
#define PIXIE_UTILITY_TYPE_TRAITS_H

#include <type_traits>
#include <tuple>
#include <string>
#include <string_view>

namespace pixie::type_traits
{

/** Detection idiom */
template<class...>
using void_t = void;

struct nonesuch
{
	nonesuch() = delete;

	~nonesuch() = delete;

	nonesuch(nonesuch const &) = delete;

	void operator=(nonesuch const &) = delete;
};

template<class Default, class AlwaysVoid, template<class...> class Op, class... Args>
struct DETECTOR
{
	using value_t = std::false_type;
	using type = Default;
};

template<class Default, template<class...> class Op, class... Args>
struct DETECTOR<Default, void_t<Op<Args...>>, Op, Args...>
{
	using value_t = std::true_type;
	using type = Op<Args...>;
};

template<template<class...> class Op, class... Args>
using is_detected = typename DETECTOR<nonesuch, void, Op, Args...>::value_t;

template<template<class...> class Op, class... Args>
constexpr bool is_detected_v = is_detected<Op, Args...>::value;

template<template<class...> class Op, class... Args>
using detected_t = typename DETECTOR<nonesuch, void, Op, Args...>::type;

template<class Default, template<class...> class Op, class... Args>
using detected_or = DETECTOR<Default, void, Op, Args...>;

template<class Expected, template<class...> class Op, class... Args>
using is_detected_exact = std::is_same<Expected, detected_t<Op, Args...>>;

template<class Expected, template<class...> class Op, class... Args>
constexpr bool is_detected_exact_v = is_detected_exact<Expected, Op, Args...>::value;

template<class To, template<class...> class Op, class... Args>
using is_detected_convertible = std::is_convertible<detected_t<Op, Args...>, To>;


template<class T>
using copy_assign = decltype(std::declval<T &>() = std::declval<T const &>());



#define GENERATE_HAS_MEMBER(member)                                                \
																				   \
template<class T> 															       \
class HasMember_##member														   \
{																				   \
private:																		   \
	using Yes = char[2];														   \
	using  No = char[1];														   \
																				   \
    struct Fallback { int member; };                                               \
    struct Derived : T, Fallback { };											   \
    																			   \
    template<class U>															   \
	static No& Test(decltype(U::member)*);										   \
    																			   \
    template<typename U>														   \
    static Yes& Test( U* );														   \
    																			   \
public:																			   \
	static constexpr bool RESULT = sizeof(Test<Derived>(nullptr)) == sizeof(Yes);  \
};																				   \
																				   \
template<class T>																   \
struct has_member_##member 														   \
	: public std::integral_constant<bool, HasMember_##member<T>::RESULT>		   \
{																				   \
};

namespace experimental
{
template<class F>
struct function_traits;

// function pointer
template<class R, class... Args>
struct function_traits<R(*)(Args...)> : public function_traits<R(Args...)>
{
};

template<class R, class... Args>
struct function_traits<R(Args...)>
{
	using return_type = R;

	static constexpr std::size_t arity = sizeof...(Args);

	template<std::size_t N>
	struct argument
	{
		static_assert(N < arity, "error: invalid parameter index.");
		using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
	};
};


template<class T>
constexpr std::string_view type_name()
{
	using namespace std;
#ifdef __clang__
	string_view p = __PRETTY_FUNCTION__;
	return string_view(p.data() + 34, p.size() - 34 - 1);
#elif defined(__GNUC__)
	string_view p = __PRETTY_FUNCTION__;
#  if __cplusplus < 201402
	return string_view(p.data() + 36, p.size() - 36 - 1);
#  else
	return string_view(p.data() + 49, p.find(';', 49) - 49);
#  endif
#elif defined(_MSC_VER)
	string_view p = __FUNCSIG__;
	return string_view(p.data() + 84, p.size() - 84 - 7);
#endif
}

} // namespace pixie::type_traits::experimental
} // namespace pixie::type_traits

#endif //PIXIE_UTILITY_TYPE_TRAITS_H
