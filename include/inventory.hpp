#ifndef INVENTORY_HPP
#define INVENTORY_HPP

#include<inventory_iterator.hpp>
#include<ptr.hpp>

#include<optional>
#include<unordered_map>
#include<variant>
#include<memory>

class Item;

class AddStatus {
public:
    struct AddError {};

    struct New {
        char at;
    };

    struct Stacked {
        char at;
        int pickedCount;
    };

    template<class St>
    AddStatus(St && status): value(std::forward<St>(status)) {}

    explicit operator bool() const {
        return not std::holds_alternative<AddError>(value);
    }

    template<class St, class Fn>
    AddStatus const & doIf(Fn func) const {
        St * state = std::get_if<St>(&value);
        if (state) {
            func(*state);
        }
        return *this;
    }

    template<class St, class Fn>
    AddStatus & doIf(Fn func) {
        St * state = std::get_if<St>(&value);
        if (state) {
            func(*state);
        }
        return *this;
    }

private:
    std::variant<AddError, New, Stacked> value;
};

class Inventory {
public:
    Inventory() = default;
    Inventory(Inventory const & other);
    Inventory & operator =(Inventory const & other);

    // template because unique_ptr actually moves if needs to cast Ptr<Derived> to Ptr<Base>
    template<class ItemType>
    AddStatus add(Ptr<ItemType> && item);

    // template because unique_ptr actually moves if needs to cast Ptr<Derived> to Ptr<Base>
    template<class ItemType>
    AddStatus add(Ptr<ItemType> && item, char at);

    Ptr<Item> remove(char id);

    InventoryIterator erase(ConstInventoryIterator iter);

    int size() const;
    bool isFull() const;
    bool isEmpty() const;
    bool hasID(char id) const;
    Item       & operator [](char id);
    Item const & operator [](char id) const;

    InventoryIterator      find(char id);
    ConstInventoryIterator find(char id) const;

    InventoryIterator      begin();
    ConstInventoryIterator begin()  const;
    ConstInventoryIterator cbegin() const;

    InventoryIterator      end();
    ConstInventoryIterator end()  const;
    ConstInventoryIterator cend() const;

private:
    std::unordered_map<char, Ptr<Item>> items;
};

#include<inventory.inl>

#endif // INVENTORY_HPP

