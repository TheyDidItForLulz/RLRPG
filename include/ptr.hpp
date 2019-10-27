#ifndef RLRPG_PTR_HPP
#define RLRPG_PTR_HPP

#include<memory>

template<class T>
using Ptr = std::unique_ptr<T>;

#endif // RLRPG_PTR_HPP

