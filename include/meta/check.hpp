#ifndef RLRPG_META_CHECK_HPP
#define RLRPG_META_CHECK_HPP

#include<type_traits>

namespace meta {
    enum CheckResult { Checked };

    template<bool Cond>
    using Check = std::enable_if_t<Cond, CheckResult>;
};

#endif // RLRPG_META_CHECK_HPP
