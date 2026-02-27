#ifndef IRR_FUNCTION_CAST_INCLUDED__
#define IRR_FUNCTION_CAST_INCLUDED__

namespace irr {

template<typename T, typename T2>
inline T function_cast(T2 ptr) {
	using generic_function_ptr = void (*)(void);
	return reinterpret_cast<T>(reinterpret_cast<generic_function_ptr>(ptr));
}

}

#endif
