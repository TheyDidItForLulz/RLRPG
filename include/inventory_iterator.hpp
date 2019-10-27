#ifndef INVENTORY_ITERATOR_HPP
#define INVENTORY_ITERATOR_HPP

#include<ptr.hpp>

#include<iterator>
#include<unordered_map>
#include<type_traits>
#include<memory>

class Item;

template<class UnderlyingIter, class ItemType, class ValueType = std::pair<char, ItemType *>>
class InventoryIteratorImpl {
    friend class Inventory;

    //using ItemType = typename UnderlyingIter::value_type::second_type::element_type;

    UnderlyingIter iter;
    mutable ValueType value;

    InventoryIteratorImpl(UnderlyingIter iter): iter(iter) {}

public:
    ValueType operator *() const {
        return std::make_pair(iter->first, iter->second.get());
    }

    ValueType * operator ->() const {
        value = std::make_pair(iter->first, iter->second.get());
        return &value;
        //return std::make_pair<*iter->second;
    }

    InventoryIteratorImpl & operator ++() {
        ++iter;
        return *this;
    }

    InventoryIteratorImpl operator ++(int) {
        auto old = *this;
        iter++;
        return old;
    }

    template<class OtherIter, class OtherItemType>
    bool operator ==(InventoryIteratorImpl<OtherIter, OtherItemType> const & other) const {
        return iter == other.iter;
    }

    template<class OtherIter, class OtherItemType>
    bool operator !=(InventoryIteratorImpl<OtherIter, OtherItemType> const & other) const {
        return iter != other.iter;
    }

    template<class OtherIter>
    operator InventoryIteratorImpl<OtherIter, ItemType const>() const {
        return InventoryIteratorImpl<OtherIter, ItemType const>(iter);
    }
};

using InventoryIterator = InventoryIteratorImpl<std::unordered_map<char, Ptr<Item>>::iterator, Item>;
using ConstInventoryIterator = InventoryIteratorImpl<std::unordered_map<char, Ptr<Item>>::const_iterator, Item const>;

#endif // INVENTORY_ITERATOR_HPP

