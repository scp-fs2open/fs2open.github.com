// Author: Fabio Alemagna <personal@fabioalemagna.net>
// Source: https://github.com/falemagn/fameta-counter
// Inspired to Filip Roséen's work. See https://stackoverflow.com/questions/60082260/c-compile-time-counters-revisited

#ifndef FAMETA_COUNTER_HPP_
#define FAMETA_COUNTER_HPP_

namespace fameta {
    struct default_counter_tag{};
    
    template <int StartN, int StartValue = 0, int Step = 1, typename Tag = default_counter_tag>
    class counter;
}

template <int StartN, int StartValue, int Step, typename Tag>
class fameta::counter {    
#if defined(__INTEL_COMPILER) || defined(_MSC_VER) || !defined(__cpp_decltype_auto)
    template <int N>
    struct slot {
        #if defined(__INTEL_COMPILER)     
        #   pragma warning push   
        #   pragma warning disable 1624
        #elif defined(__GNUC__) && !defined(__clang__)
        #   pragma GCC diagnostic push
        #   pragma GCC diagnostic ignored "-Wnon-template-friend"
        #endif
        friend constexpr int slot_value(slot<N>);
        #if defined(__INTEL_COMPILER)     
        #   pragma warning pop  
        #elif defined(__GNUC__) && !defined(__clang__)
        #   pragma GCC diagnostic pop
        #endif
    };

    template <int N, int I>
    struct writer {
        friend constexpr int slot_value(slot<N>) {
            return I;
        }

        static constexpr int value = I;
    };

    template <int N, int R = slot_value(slot<N>())>
    static constexpr int reader(int, slot<N>) {
        return R;
    };
#else
    template <int N>
    struct slot {
        #if defined(__GNUC__) && !defined(__clang__)
        #   pragma GCC diagnostic push
        #   pragma GCC diagnostic ignored "-Wnon-template-friend"
        #endif
        friend constexpr auto slot_value(slot<N>);
        #if defined(__GNUC__) && !defined(__clang__)
        #   pragma GCC diagnostic pop
        #endif
    };

    template <int N, int I>
    struct writer {
        friend constexpr auto slot_value(slot<N>) {
            return I;
        }

        static constexpr int value = I;
    };

    template <int N, typename = decltype(slot_value(slot<N>()))>
    static constexpr int reader(int, slot<N>, int R = slot_value(slot<N>())) {
        return R;
    }
#endif

    static_assert(sizeof(writer<StartN, StartValue-Step>), "Base case");

    template <int N>
    static constexpr int reader(float, slot<N>, int R = reader(0, slot<N-1>())) {
        return R;
    }

public:    

#if !defined(__clang_major__) || __clang_major__ > 7
    template <int N>
    static constexpr int next(int R = writer<N, reader(0, slot<N-1>())+Step>::value) {
        return R;
    }

    template <int N>
    static constexpr int current(int R = reader(0, slot<N-1>())) {
        return R;
    }    
#else
    template <int N>
    static constexpr int next(int R = writer<N, reader(0, slot<N>())+Step>::value) {
        return R;
    }
    
    template <int N>
    static constexpr int current(int R = reader(0, slot<N>())) {
        return R;
    }
#endif
};

#endif
