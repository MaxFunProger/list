#pragma once
#include <iostream>
#include <string>
#include <tuple>

template <typename T, typename Allocator = std::allocator<T>>
class List {
    using value_type = T;
    using pointer = value_type*;
    using Self = List<T, Allocator>;

  private:
    struct BaseNode {
        BaseNode()
            : prev(this), next(this) {}
        BaseNode(BaseNode* p, BaseNode* n)
            : prev(p), next(n) {}
        BaseNode* prev;
        BaseNode* next;
        void swap(BaseNode& other) {
            prev->next = &other;
            next->prev = &other;
            other.prev->next = this;
            other.next->prev = this;

            BaseNode* prev_th = prev;
            BaseNode* next_th = next;
            next = other.next;
            prev = other.prev;
            other.next = next_th;
            other.prev = prev_th;
        }
    };
    struct Node : BaseNode {
        Node()
            : BaseNode(), value(T()) {}
        Node(BaseNode* p, BaseNode* n, const T& val)
            : BaseNode(p, n), value(val) {}
        Node(BaseNode* p, BaseNode* n)
            : BaseNode(p, n), value(T()) {}
        value_type value;
    };

  public:
    using allocator_type =
        typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
    using alloc_traits = typename std::allocator_traits<allocator_type>;

  private:
    BaseNode last_;
    BaseNode* first_ = &last_;
    size_t size_;
    allocator_type allocator_;

  public:
    List()
        : first_(&last_),
          size_(0),
          allocator_(allocator_type()),
          end_iter_(iterator(&last_)),
          begin_iter_(end_iter_) {}

    List(Allocator alloc)
        : first_(&last_),
          size_(0),
          allocator_(allocator_type(alloc)),
          end_iter_(iterator(&last_)),
          begin_iter_(end_iter_) {}

    List(size_t sz, Allocator alloc)
        : size_(sz), allocator_(allocator_type(alloc)) {
        BaseNode* allocated;
        for (size_t i = 0; i < size_; ++i) {
            bool first_trigger = false, second_trigger = false, third_trigger = false;
            try {
                first_trigger = true;
                allocated = allocator_.allocate(1);
                if (i == 0) {
                    first_ = allocated;
                    second_trigger = true;
                    alloc_traits::construct(allocator_, static_cast<Node*>(allocated),
                                            &last_, &last_);
                    last_.prev = first_;
                    last_.next = first_;
                } else {
                    third_trigger = true;
                    alloc_traits::construct(allocator_, static_cast<Node*>(allocated),
                                            last_.prev, &last_);
                    // last_.prev = static_cast<Node*>(allocated);
                    last_.prev->next = static_cast<Node*>(allocated);
                    last_.prev = static_cast<Node*>(allocated);
                }
            } catch (...) {
                if (third_trigger) {
                    allocator_.deallocate(static_cast<Node*>(allocated), 1);
                    BaseNode* deleter = first_;
                    while (deleter != &last_) {
                        deleter = deleter->next;
                        alloc_traits::destroy(allocator_,
                                              static_cast<Node*>(deleter->prev));
                        allocator_.deallocate(static_cast<Node*>(deleter->prev), 1);
                    }
                    throw;
                } else if (second_trigger) {
                    allocator_.deallocate(static_cast<Node*>(allocated), 1);
                    throw;
                } else if (first_trigger) {
                    if (i > 0) {
                        while (first_ != &last_) {
                            first_ = first_->next;
                            alloc_traits::destroy(allocator_, static_cast<Node*>(first_->prev));
                            allocator_.deallocate(static_cast<Node*>(first_->prev), 1);
                        }
                    }
                    throw;
                }
            }
        }
        end_iter_ = iterator(&last_);
        begin_iter_ = iterator(first_);
    }

    List(size_t sz, const T& val = T(), Allocator alloc = Allocator())
        : size_(sz), allocator_(allocator_type(alloc)) {
        BaseNode* allocated;
        for (size_t i = 0; i < size_; ++i) {
            bool first_trigger = false, second_trigger = false, third_trigger = false;
            try {
                first_trigger = true;
                allocated = allocator_.allocate(1);
                if (i == 0) {
                    first_ = allocated;
                    second_trigger = true;
                    alloc_traits::construct(allocator_, static_cast<Node*>(first_),
                                            &last_, &last_, val);
                    last_.prev = first_;
                    last_.next = first_;
                } else {
                    third_trigger = true;
                    alloc_traits::construct(allocator_, static_cast<Node*>(allocated),
                                            last_.prev, &last_, val);
                    last_.prev->next = static_cast<Node*>(allocated);
                    last_.prev = static_cast<Node*>(allocated);
                }
            } catch (...) {
                if (third_trigger) {
                    allocator_.deallocate(static_cast<Node*>(allocated), 1);
                    BaseNode* deleter = first_;
                    while (deleter != &last_) {
                        deleter = deleter->next;
                        alloc_traits::destroy(allocator_,
                                              static_cast<Node*>(deleter->prev));
                        allocator_.deallocate(static_cast<Node*>(deleter->prev), 1);
                    }
                    throw;
                } else if (second_trigger) {
                    allocator_.deallocate(static_cast<Node*>(allocated), 1);
                    throw;
                } else if (first_trigger) {
                    if (i > 0) {
                        while (first_ != &last_) {
                            first_ = first_->next;
                            alloc_traits::destroy(allocator_, static_cast<Node*>(first_->prev));
                            allocator_.deallocate(static_cast<Node*>(first_->prev), 1);
                        }
                    }
                    throw;
                }
            }
        }
        end_iter_ = iterator(&last_);
        begin_iter_ = end_iter_;
        ++begin_iter_;
    }

    Allocator get_allocator();

    List(const List& other)
        : size_(other.size_),
          allocator_(alloc_traits::select_on_container_copy_construction(
              other.allocator_)) {
        BaseNode* allocated;
        BaseNode* copy_from = other.first_;
        for (size_t i = 0; i < size_; ++i) {
            try {
                allocated = allocator_.allocate(1);
            } catch (...) {
                if (i > 0) {
                    while (first_ != &last_) {
                        first_ = first_->next;
                        alloc_traits::destroy(allocator_, static_cast<Node*>(first_->prev));
                        allocator_.deallocate(static_cast<Node*>(first_->prev), 1);
                    }
                }
                throw;
            }
            if (i == 0) {
                first_ = allocated;
                try {
                    alloc_traits::construct(allocator_, static_cast<Node*>(allocated),
                                            &last_, &last_,
                                            static_cast<Node*>(copy_from)->value);
                } catch (...) {
                    allocator_.deallocate(static_cast<Node*>(allocated), 1);
                    throw;
                }
                last_.prev = first_;
                last_.next = first_;
            } else {
                try {
                    alloc_traits::construct(allocator_, static_cast<Node*>(allocated),
                                            last_.prev, &last_,
                                            static_cast<Node*>(copy_from)->value);
                } catch (...) {
                    allocator_.deallocate(static_cast<Node*>(allocated), 1);
                    BaseNode* deleter = first_;
                    while (deleter != &last_) {
                        deleter = deleter->next;
                        alloc_traits::destroy(allocator_,
                                              static_cast<Node*>(deleter->prev));
                        allocator_.deallocate(static_cast<Node*>(deleter->prev), 1);
                    }
                    throw;
                }
                last_.prev->next = allocated;
                last_.prev = allocated;
            }
            copy_from = copy_from->next;
        }
        end_iter_ = iterator(&last_);
        begin_iter_ = end_iter_;
        ++begin_iter_;
    }

    Self& operator=(const Self& other) {
        if (this == &other) {
            return *this;
        }
        Self copy = other;

        last_.swap(copy.last_);
        first_ = last_.next;
        copy.first_ = copy.last_.next;
        end_iter_ = iterator(&last_);
        begin_iter_ = end_iter_;
        ++begin_iter_;

        copy.end_iter_ = iterator(&copy.last_);
        copy.begin_iter_ = copy.end_iter_;
        ++copy.begin_iter_;
        std::swap(size_, copy.size_);
        if constexpr (std::is_base_of_v<
                          std::true_type,
                          typename alloc_traits::
                              propagate_on_container_copy_assignment>) {
            allocator_ = other.allocator_;
        }

        return *this;
    }

    ~List();

    template <bool Const>
    struct Iterator;

    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    template <bool Const>
    struct Iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = int;
        using value_type = std::conditional_t<Const, const T, T>;
        using pointer = std::conditional_t<Const, const T*, T*>;
        using reference = std::conditional_t<Const, T const&, T&>;
        using Self = Iterator<Const>;

        BaseNode* node;

        Iterator()
            : node(nullptr) {}

        Iterator(BaseNode* bnode)
            : node(bnode) {}

        template <bool OtherConst>
            requires(Const || !OtherConst)
        Iterator(const List<T, Allocator>::Iterator<OtherConst>& iter) {
            node = iter.node;
        }

        ~Iterator() {}

        template <bool OtherConst>
            requires(Const || !OtherConst)
        Self& operator=(const Iterator<OtherConst>& iter) {
            node = iter.node;
            return *this;
        }

        template <typename IterType>
        bool operator==(const IterType& iter) const {
            return (node == iter.node);
        }

        bool operator!=(const const_iterator& iter) const {
            return !(node == iter.node);
        }

        Self& operator++() {
            node = node->next;
            return *this;
        }

        Self& operator--() {
            node = node->prev;
            return *this;
        }

        Self operator--(int) {
            Self tmp = *this;
            node = node->prev;
            return tmp;
        }

        Self operator++(int) {
            Self tmp = *this;
            node = node->next;
            return tmp;
        }

        reference operator*() {
            return (*static_cast<Node*>(node)).value;
        }

        pointer operator->() {
            return static_cast<Node*>(node)->value;
        }
    };

    size_t size() const;

    void push_back(const T& /*val*/);
    void pop_back();
    void push_front(const T& /*val*/);
    void pop_front();
    void insert(const_iterator /*iter*/, const T& /*val*/);
    void erase(const_iterator /*iter*/);

    iterator begin();
    const_iterator begin() const;
    reverse_iterator rbegin();
    const_reverse_iterator rbegin() const;
    const_iterator cbegin() const;
    const_reverse_iterator crbegin() const;
    iterator end();
    const_iterator end() const;
    reverse_iterator rend();
    const_reverse_iterator rend() const;
    const_iterator cend() const;
    const_reverse_iterator crend() const;

  private:
    iterator end_iter_, begin_iter_;
};

template <typename T, typename Allocator>
Allocator List<T, Allocator>::get_allocator() {
    return typename alloc_traits::template rebind_alloc<T>(allocator_);
}

template <typename T, typename Allocator>
size_t List<T, Allocator>::size() const {
    return size_;
}

template <typename T, typename Allocator>
List<T, Allocator>::~List() {
    while (size_ != 0) {
        pop_back();
    }
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::begin() const {
    return const_iterator(begin_iter_);
}
template <typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::begin() {
    return begin_iter_;
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::rbegin()
    const {
    return const_reverse_iterator(end_iter_);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::reverse_iterator List<T, Allocator>::rbegin() {
    return reverse_iterator(end_iter_);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::cbegin() const {
    return const_iterator(begin_iter_);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator
List<T, Allocator>::crbegin() const {
    return const_reverse_iterator(end_iter_);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::end() {
    return end_iter_;
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::end() const {
    return const_iterator(end_iter_);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::rend()
    const {
    return const_reverse_iterator(begin_iter_);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::reverse_iterator List<T, Allocator>::rend() {
    return reverse_iterator(begin_iter_);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::cend() const {
    return const_iterator(end_iter_);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::crend()
    const {
    return const_reverse_iterator(begin_iter_);
}

template <typename T, typename Allocator>
void List<T, Allocator>::insert(const_iterator iter, const T& val) {
    Node* new_node;
    try {
        new_node = allocator_.allocate(1);
    } catch (...) {
        throw;
    }
    try {
        alloc_traits::construct(allocator_, new_node, iter.node->prev, iter.node,
                                val);
    } catch (...) {
        allocator_.deallocate(static_cast<Node*>(new_node), 1);
        throw;
    }
    iter.node->prev = new_node;
    new_node->prev->next = new_node;
    if (iter.node == first_) {
        first_ = new_node;
    }
    begin_iter_ = end_iter_;
    ++begin_iter_;
    ++size_;
}

template <typename T, typename Allocator>
void List<T, Allocator>::erase(const_iterator iter) {
    iter.node->prev->next = iter.node->next;
    iter.node->next->prev = iter.node->prev;
    if (iter.node == first_) {
        first_ = iter.node->next;
    }
    alloc_traits::destroy(allocator_, static_cast<Node*>(iter.node));
    allocator_.deallocate(static_cast<Node*>(iter.node), 1);
    begin_iter_ = end_iter_;
    ++begin_iter_;
    --size_;
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_back(const T& val) {
    insert(end(), val);
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_front(const T& val) {
    insert(begin(), val);
}

template <typename T, typename Allocator>
void List<T, Allocator>::pop_back() {
    erase(iterator(end().node->prev));
}

template <typename T, typename Allocator>
void List<T, Allocator>::pop_front() {
    erase(begin());
}