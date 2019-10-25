#ifndef RLRPG_REGISTRY_HPP
#define RLRPG_REGISTRY_HPP

#include<enable_clone.hpp>

#include<effolkronium/random.hpp>

#include<unordered_map>
#include<functional>

namespace reg {
    using DefaultIDType = std::string;

    template<class T, class ID = DefaultIDType>
    using Registry = std::unordered_map<ID, T>;

    ////////////////////
    // Selectors select some registry item in some way (likely random)
    ////////////////////

    ////////////////////
    // Signatures for selectors for T &, T && and const T &
    ////////////////////

    template<class T, class ID = DefaultIDType>
    using RefSelectorSig = T & (Registry<T, ID> &);

    template<class T, class ID = DefaultIDType>
    using ConstRefSelectorSig = const T & (const Registry<T, ID> &);

    template<class T, class ID = DefaultIDType>
    using CopySelectorSig = T (const Registry<T, ID> &);

    template<class T, class ID = DefaultIDType, meta::Check<IsClonable<T>> = meta::Checked>
    using CloneSelectorSig = T (const Registry<T, ID> &);

    template<class T, class ID = DefaultIDType>
    using MoveSelectorSig = T && (Registry<T, ID> &&);

    ////////////////////
    // std::function<SomeSelector> wrapper types

    template<class T, class ID = DefaultIDType>
    using RefSelector = std::function<RefSelectorSig<T, ID>>;

    template<class T, class ID = DefaultIDType>
    using ConstRefSelector = std::function<ConstRefSelectorSig<T, ID>>;

    template<class T, class ID = DefaultIDType>
    using CopySelector = std::function<CopySelectorSig<T, ID>>;

    template<class T, class ID = DefaultIDType>
    using MoveSelector = std::function<MoveSelectorSig<T, ID>>;

    template<class T, class ID = DefaultIDType, meta::Check<IsClonable<T>> = meta::Checked>
    using CloneSelector = std::function<CloneSelectorSig<T, ID>>;

    ////////////////////
    // Default random selectors

    // CosntRefSelector
    template<class T, class ID = DefaultIDType>
    const T & pickAnyCRef(const Registry<T, ID> & reg) {
        return effolkronium::random_static::get(reg)->second;
    }

    // RefSelector
    template<class T, class ID = DefaultIDType>
    T & pickAnyRef(Registry<T, ID> & reg) {
        return effolkronium::random_static::get(reg)->second;
    }

    // CopySelector
    template<class T, class ID = DefaultIDType>
    T pickAny(const Registry<T, ID> & reg) {
        return effolkronium::random_static::get(reg)->second;
    }

    // MoveSelector
    template<class T, class ID = DefaultIDType>
    T pickAny(Registry<T, ID> && reg) {
        return std::move(effolkronium::random_static::get(reg)->second);
    }

    // CloneSelector
    template<class T, class ID = DefaultIDType, meta::Check<IsClonable<T>> = meta::Checked>
    T pickAny(Registry<T, ID> && reg) {
        return effolkronium::random_static::get(reg)->second.clone();
    }
} // namespace reg

using reg::Registry;

#endif // RLRPG_REGISTRY_HPP

