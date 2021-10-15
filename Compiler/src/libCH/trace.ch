#ifndef CH_TRACE_CH
#define CH_TRACE_CH 2021'04'27

#include <iostream>
#include <string>

namespace CH {

struct Trace {
    static std::string indent;

    std::string ind;

    template <typename ... TT>
    Trace (TT ... xx) : ind(indent) {
	std::clog << indent << "{";
	((std::clog << " " << xx), ...);
	std::clog << std::endl;
	indent += "  ";
    }

    template <typename T, typename ... TT>
    void operator() (T x, TT ... xx) {
	std::clog << indent << x;
	((std::clog << " " << xx), ...);
	std::clog << std::endl;
    }

    ~Trace () {
	indent = ind;
	std::clog << indent << "}" << std::endl;
    }
};

inline std::string Trace::indent = "";

#ifdef CH_TRACE

#define CH_TRACE_BLOCK(...) \
    CH::Trace __trace(__VA_ARGS__);

#define CH_TRACE_LINE(...) \
    __trace(__VA_ARGS__);

#else

#define CH_TRACE_BLOCK(...) /* leer */
#define CH_TRACE_LINE(...) /* leer */

#endif

#ifdef USING_CH
#define TRACE_BLOCK CH_TRACE_BLOCK
#define TRACE_LINE CH_TRACE_LINE
#endif
}

#endif
