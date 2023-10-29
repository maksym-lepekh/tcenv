module;
#include <functional>

export module c_api;

export namespace c_api
{
    template <typename T, typename Deleter>
    struct opaque
    {
        opaque(auto alloc_fn, auto free_fn)
        {
            value   = alloc_fn();
            deleter = free_fn;
        }
        opaque(auto alloc_fn, auto free_fn, auto init_fn, auto... init_args)
        {
            value = alloc_fn();
            std::invoke(init_fn, value, std::forward<decltype(init_args)>(init_args)...);
            deleter = free_fn;
        }

        opaque(opaque&& other) noexcept
        {
            value         = other.value;
            deleter       = other.deleter;
            other.deleter = nullptr;
        }

        opaque(const opaque&) = delete;

        operator T*() noexcept
        {
            return value;
        }

        ~opaque() noexcept
        {
            if (deleter)
            {
                deleter(value);
            }
        }

    private:
        T* value;
        Deleter deleter;
    };

    template <typename Alloc, typename Free>
    opaque(Alloc alloc_fn, Free free_fn) -> opaque<std::remove_pointer_t<std::invoke_result_t<Alloc>>, Free>;
}    // namespace c_api
