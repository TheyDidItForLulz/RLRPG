#ifndef INVENTORY_ITERATOR_HPP
#define INVENTORY_ITERATOR_HPP

#include<iterator>
#include<unordered_map>
#include<type_traits>
#include<memory>

class Item;
using ItemPtr = std::unique_ptr<Item>;

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
    bool operator ==(const InventoryIteratorImpl<OtherIter, OtherItemType> & other) const {
        return iter == other.iter;
    }

    template<class OtherIter, class OtherItemType>
    bool operator !=(const InventoryIteratorImpl<OtherIter, OtherItemType> & other) const {
        return iter != other.iter;
    }

    template<class OtherIter>
    operator InventoryIteratorImpl<OtherIter, const ItemType>() const {
        return InventoryIteratorImpl<OtherIter, const ItemType>(iter);
    }
};

using InventoryIterator = InventoryIteratorImpl<std::unordered_map<char, ItemPtr>::iterator, Item>;
using ConstInventoryIterator = InventoryIteratorImpl<std::unordered_map<char, ItemPtr>::const_iterator, const Item>;

#endif // INVENTORY_ITERATOR_HPP

