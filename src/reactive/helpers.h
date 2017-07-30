#ifndef REACTIVE_PROPERTYOBSERVER_HELPERS_H
#define REACTIVE_PROPERTYOBSERVER_HELPERS_H

#include <tuple>
#include <utility>

namespace reactive{
    namespace utils{
        // helper functions
        template<int i, class Closure>
        static void foreach(Closure &&closure) {}
        template<int i, class Closure, class Arg, class ...Args>
        static void foreach(Closure &&closure, Arg &&arg, Args &&... args) {
            closure(
                    std::integral_constant<int, i>{},
                    std::forward<Arg>(arg)
            );
            foreach<i + 1>(std::forward<Closure>(closure), std::forward<Args>(args)...);
        }
        template<class Closure, class ...Args>
        static void foreach(Closure &&closure, Args &&... args) {
            foreach<0>(std::forward<Closure>(closure), std::forward<Args>(args)...);
        }

        template<class F, class Tuple, std::size_t... I>
        static constexpr decltype(auto) apply_impl(F &&f, Tuple &&t, std::index_sequence<I...>) {
            return f(std::get<I>(std::forward<Tuple>(t))...);
        }
        template<class F, class Tuple>
        static constexpr decltype(auto) apply(F &&f, Tuple &&t) {
            return apply_impl(std::forward<F>(f), std::forward<Tuple>(t),
                              std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>{});
        }

        template<class Closure, class Tuple>
        static void foreach_tuple(Closure &&closure, Tuple &&tuple) {
            reactive::utils::apply([&](auto &&... args) {
                foreach(closure, std::forward<decltype(args)>(args)...);
            }, tuple);
        }


        template<typename ... V>
        static constexpr bool and_all(const V &... v) {
            bool result = true;
            (void) std::initializer_list<int>{(result = result && v, 0)...};
            return result;
        }
        template<typename ... V>
        static constexpr bool or_all(const V &... v) {
            bool result = false;
            (void)std::initializer_list<int>{(result = result || v, 0)...};
            return result;
        }
    }
}

#endif //REACTIVE_PROPERTYOBSERVER_HELPERS_H
