#pragma once

#include <iostream>
#include <iterator>
#include <type_traits>

namespace intrusive {

/*
Тег по-умолчанию чтобы пользователям не нужно было
придумывать теги, если они используют лишь одну базу
list_element.
*/
struct default_tag;

template <typename Tag = default_tag>
struct list_element {
    list_element() = default;

    list_element(list_element* prev, list_element* next) {
        this.prev = prev;
        this.next = next;
    }

    list_element(const list_element& other) {
        prev = other.prev;
        next = other.next;
    }

    /* Отвязывает элемент из списка в котором он находится. */
    void unlink() {
        if (prev != nullptr)
            prev->next = next;

        if (next != nullptr)
            next->prev = prev;

        next = nullptr;
        prev = nullptr;
    };

    bool operator==(list_element const& elem) const& noexcept {
        return elem.prev == prev && elem.next == elem.next;
    }

    bool operator!=(list_element const& elem) const& noexcept {
        return elem.prev != prev || elem.next != elem.next;
    };

    list_element* prev = nullptr;
    list_element* next = nullptr;
};

template <typename T, typename Tag = default_tag>
struct list {

    struct list_iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = std::remove_const_t<T>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        list_iterator() = default;

        template <typename NonConstIterator>
        list_iterator(NonConstIterator other,
                      std::enable_if_t<std::is_same_v<NonConstIterator, list_iterator> &&
                                       std::is_const_v<T>>* = nullptr) noexcept
            : current(other.current) {
        }

        // list_iterator(list_element<Tag>* current_) noexcept : current(current_) {
        // }

        // list_iterator(NonConstIterator other) : public std::iterator<iterator_category,
        // value_type, difference_type, pointer, reference>(
        //   other.
        // ) {

        // };

        T& operator*() const noexcept {
            return *static_cast<T*>(current);
        };

        T* operator->() const noexcept {
            return static_cast<T*>(current);
        };

        list_iterator& operator++() & noexcept {
            current = current->next;
            return *this;
        }

        list_iterator& operator--() & noexcept {
            current = current->prev;
            return *this;
        }

        list_iterator operator++(int) & noexcept {
            auto this_cpy = *this;
            current = current->next;
            return this_cpy;
        };

        list_iterator operator--(int) & noexcept {
            auto this_cpy = *this;
            current = current->next;
            return this_cpy;
        };

        bool operator==(list_iterator const& rhs) const& noexcept {
            return rhs.current == current;
        };

        bool operator!=(list_iterator const& rhs) const& noexcept {
            return rhs.current != current;
        };

        //   private:
        /*
    Это важно иметь этот конструктор private, чтобы итератор нельзя было создать
    от nullptr.
    */
        explicit list_iterator(list_element<Tag>* current_) noexcept : current(current_) {
        }

      private:
        /*
    Хранить list_element_base*, а не T* важно.
    Иначе нельзя будет создать list_iterator для
    end().
    */
        list_element<Tag>* current;
    };

    typedef list_iterator iterator;
    typedef list_iterator const_iterator;

    static_assert(std::is_convertible_v<T&, list_element<Tag>&>,
                  "value type is not convertible to list_element");

    int                size = 0;
    list_element<Tag>* head = nullptr;
    list_element<Tag>* tail = nullptr;

    list() noexcept {
        tail = list_element();
        head = &tail;
    };

    list(list const&) = delete;
    list(list&& l) noexcept
        : head(std::move(l.head)), tail(std::move(l.tail)), size(std::move(l.size)) {
    }
    ~list() {
        clear();
    };

    list& operator=(list const&) = delete;
    list& operator=(list&& l_) noexcept {
        return l_;
    };

    void clear() noexcept {
        auto front = head;

        while (front != nullptr) {
            auto front_next = front->next;
            front->unlink();

            front = front_next;
        }
    };

    /*
    Поскольку вставка изменяет данные в list_element
    мы принимаем неконстантный T&.
    */
    void push_back(T& elem) noexcept {
        if (size == 0) {
            head = &elem;

            tail->prev = head;
            head->next = tail;

            size = 1;
            return;
        }

        tail->prev->next = &elem;
        static_cast<list_element<Tag>*>(&elem)->prev = tail->prev;
        // elem.prev = tail;
        tail->prev = &elem;

        size++;
    }

    void pop_back() noexcept {
        if (size == 0)
            return;

        if (size == 1) {
            head->next = nullptr;
            head = tail;

            size = 0;
            return;
        }

        tail = tail->prev;
        tail->next->prev = nullptr;
        tail->next = nullptr;

        size--;
    }

    T& back() noexcept {
        return *static_cast<T*>(tail);
    }

    T const& back() const noexcept {
        return *static_cast<T*>(tail);
    };

    void push_front(T& elem) noexcept {
        if (size == 0) {
            tail = &elem;
            head = &elem;

            tail->prev = head;
            head->next = tail;

            size = 1;
            return;
        }

        head->prev = &elem;
        static_cast<list_element<Tag>*>(&elem)->next = head;
        // elem.next = head;
        head = &elem;
    };

    void pop_front() noexcept {
        if (size == 0)
            return;

        if (size == 1) {
            tail->prev = nullptr;
            head->next = nullptr;

            tail = nullptr;
            head = nullptr;

            size = 0;
            return;
        }

        head = head->next;
        head->prev->next = nullptr;
        head->prev = nullptr;

        size--;
    }

    T& front() noexcept {
        return *static_cast<T*>(head);
    }

    T const& front() const noexcept {
        return *static_cast<T*>(head);
    }

    bool empty() const noexcept {
        return size == 0;
    }

    iterator begin() noexcept {
        return list_iterator(head);
    }

    const_iterator begin() const noexcept {
        return list_iterator(head);
    }

    iterator end() noexcept {
        return list_iterator(tail)++;
    }
    const_iterator end() const noexcept {
        return list_iterator(tail)++;
    }

    iterator insert(const_iterator pos, T& elem) noexcept {
        if (pos->prev != nullptr)
            pos->prev->next = static_cast<list_element<Tag>*>(&elem);

        if (pos->next != nullptr)
            pos->next->prev = static_cast<list_element<Tag>*>(&elem);

        elem.prev = pos->prev;
        elem.next = pos->next;

        return list_iterator(&elem);
    }

    iterator erase(const_iterator pos) noexcept {
        auto prev_cpy = pos->prev;
        auto next_cpy = pos->next;

        pos->unlink();

        if (prev_cpy != nullptr)
            return list_iterator(prev_cpy);

        if (next_cpy != nullptr)
            return list_iterator(next_cpy);

        return nullptr;
    }

    void splice(const_iterator pos, list&, const_iterator first, const_iterator last) noexcept {
    }
};
} // namespace intrusive