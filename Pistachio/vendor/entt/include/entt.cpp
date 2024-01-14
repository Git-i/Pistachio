#include "entt.hpp"
namespace entt
{
    template<typename... Args>
    [[nodiscard]] meta_any meta_type::construct(Args &&...args) const {
        meta_any arguments[sizeof...(Args) + 1u]{ {*ctx, std::forward<Args>(args)}... };
        return construct(arguments, sizeof...(Args));
    }
}
