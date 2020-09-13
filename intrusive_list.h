#pragma once

#include <type_traits>
#include <iterator>

namespace intrusive
{
    struct default_tag;

    template <typename Tag = default_tag>
    struct list_element {
        list_element() noexcept {
            prev = next = nullptr;
        }
        ~list_element() {
            unlink();
        };

        void unlink() {
            if (prev) {
                prev->next = next;
            }
            if (next) {
                next->prev = prev;
            }
            prev = nullptr;
            next = nullptr;
        }
        template <typename _T, typename _Tag>
        friend struct list;
        template <typename _T, typename _Tag>
        friend struct list_iterator;
    private:
        list_element* prev;
        list_element* next;
    };

    template <typename T, typename Tag = default_tag>
    struct list_iterator {
        template <typename _T, typename _Tag>
        friend struct list_iterator;
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = std::remove_const_t<T>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        list_iterator() = default;

        template <typename NonConstIterator>
        list_iterator(NonConstIterator other,
                      std::enable_if_t<
                              std::is_same_v<NonConstIterator, list_iterator<std::remove_const_t<T>, Tag>> &&
                              std::is_const_v<T>>* = nullptr) noexcept
                : element(other.element) {}

        list_iterator(list_iterator const& other) noexcept : element(other.element) {}


        T& operator*() const noexcept {
            return *static_cast<T*>(element);
        }

        T* operator->() const noexcept {
            return static_cast<T*>(element);
        }

        bool operator ==(list_iterator const& other) const& noexcept {
            return element == other.element;
        }

        bool operator!=(list_iterator const& other) const& noexcept {
            return element != other.element;
        }

        list_iterator& operator++() & noexcept {
            element = element->next;
            return *this;
        }

        list_iterator operator++(int) & noexcept {
            list_iterator old(*this);
            element = element->next;
            return old;
        }

        list_iterator& operator--() & noexcept {
            element = element->prev;
            return *this;
        }

        list_iterator operator--(int) & noexcept {
            list_iterator old(*this);
            element = element->prev;
            return old;
        }
        template<typename _T, typename _Tag>
        friend struct list;
    private:
        explicit list_iterator(list_element<Tag>* el) noexcept : element(el) {}
        list_element<Tag>* element;
    };


    template <typename T, typename Tag = default_tag>
    struct list {
        typedef list_iterator<T, Tag> iterator;
        typedef list_iterator<T const, Tag> const_iterator;

        static_assert(std::is_convertible_v<T&, list_element<Tag>&>,
                      "value type is not convertible to list_element");

        list() {
            node = new list_element<Tag>();
            node->next = node;
            node->prev = node;
        }

        list(list const&) = delete;

        list(list&& r_list) noexcept : list() {
            std::swap(node, r_list.node);
        }

        ~list() {
            delete(node);
        }

        list& operator=(list const&) = delete;

        list& operator=(list&& r_list) noexcept {
            clear();
            std::swap(node, r_list.node);
            return *this;
        }

        void clear() noexcept {
            node->prev = node->next = node;
        }

        void push_back(T& element) noexcept {
            node->prev->next = static_cast<list_element<Tag>*>(&element);
            element.list_element<Tag>::prev = node->prev;
            element.list_element<Tag>::next = node;
            node->prev = static_cast<list_element<Tag>*>(&element);
        }

        void pop_back() noexcept {
            node->prev = node->prev->prev;
            node->prev->next = node;
        }

        T& back() noexcept {
            return *static_cast<T*>(node->prev);
        }

        T const& back() const noexcept {
            return *static_cast<T*>(node->prev);
        }

        void push_front(T& element) noexcept {
            node->next->prev = static_cast<list_element<>*>(&element);
            element.next = node->next;
            node->next = static_cast<list_element<>*>(&element);;
            element.prev = node;
        };

        void pop_front() noexcept {
            node->next = node->next->next;
            node->next->prev = node;
        }

        T& front() noexcept {
            return *static_cast<T*>(node->next);
        }

        T const& front() const noexcept {
            return *static_cast<T*>(node->next);
        }

        bool empty() const noexcept {
            return node->next == node;
        }

        iterator begin() noexcept {
            return iterator(node->next);
        }

        const_iterator begin() const noexcept {
            return iterator(node->next);
        }

        iterator end() noexcept {
            return iterator(node);
        }

        const_iterator end() const noexcept {
            return iterator(node);
        }

        iterator insert(const_iterator pos, T& element) noexcept {
            iterator mut_pos = iterator(pos->next->prev);
            mut_pos->prev->next = static_cast<list_element<>*>(&element);
            element.prev = &*mut_pos->prev;
            mut_pos->prev = static_cast<list_element<>*>(&element);
            element.next = &*mut_pos;
            return iterator(mut_pos->prev);
        }

        iterator erase(const_iterator pos) noexcept {
            iterator mut_pos = iterator(pos->next->prev);
            mut_pos->prev->next = mut_pos->next;
            mut_pos->next->prev = mut_pos->prev;
            iterator it = iterator(mut_pos->next);
            mut_pos->unlink();
            return it;
        }

        void splice(const_iterator pos, list& other, const_iterator first, const_iterator last) noexcept {
            if (first == last) {
                return;
            }
            iterator mut_pos = iterator(pos->next->prev);
            iterator prev_last = iterator(last->prev);
            iterator mut_last = iterator(last->prev->next);
            iterator mut_first = iterator(first->prev->next);
            mut_last->prev = mut_first->prev;
            mut_first->prev->next = &*mut_last;
            prev_last->next = &*mut_pos;
            mut_first->prev = &*mut_pos->prev;
            prev_last->next->prev = &*prev_last;
            mut_first->prev->next = &*mut_first;
        }
    private:
        list_element<Tag>* node;
    };


}
