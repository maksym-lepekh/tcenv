#ifndef TYPE_SEQ_HPP
# define TYPE_SEQ_HPP

// Andrei Alexandrescu encouraged me to add this into every project
export template <typename...>
struct type_seq
{
};

template <typename... Ts>
struct inspect;

template <typename T, typename... Ts>
struct inspect<type_seq<T, Ts...>>
{
    using head_t = T;
    using tail_t = type_seq<Ts...>;
};

template <typename, typename>
struct concat
{
};

template <typename T, typename... Ts>
struct concat<T, type_seq<Ts...>>
{
    using result_t = type_seq<T, Ts...>;
};

template <typename T>
using head_t = typename inspect<T>::head_t;

template <typename T>
using tail_t = typename inspect<T>::tail_t;

template <typename T, typename List>
using concat_t = typename concat<T, List>::result_t;

#endif    // TYPE_SEQ_HPP
