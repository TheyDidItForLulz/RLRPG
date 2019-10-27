#ifndef RLRPG_ENABLE_CLONE_HPP
#define RLRPG_ENABLE_CLONE_HPP

#include<memory>
#include<meta/check.hpp>
#include<type_traits>

template<class T>
struct EnableClone {
    std::unique_ptr<T> clone() const {
        return std::make_unique<T>(static_cast<T const &>(*this));
    }
};

template<class T>
constexpr bool IsClonable = std::is_base_of_v<EnableClone<T>, T>;

#endif // RLRPG_ENABLE_CLONE_HPP

