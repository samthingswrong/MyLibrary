#pragma once
#include <list>
#include <stdexcept>

template <typename T, typename Alloc = std::allocator<T>>
class List {
  public:
  template <bool is_const>
  class CommonIterator;

  using value_type = T;
  using allocator_type = Alloc;

  using size_type = size_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using rvalue_reference = value_type&&;

  using iterator = CommonIterator<false>;
  using const_iterator = CommonIterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  // List() = default;
  List() {
    fictive_allocate();
  }

  explicit List(const allocator_type& alloc) : alloc_(alloc) {
    fictive_allocate();
  }

  explicit List(size_type count, const_reference value,
                const allocator_type& alloc = allocator_type());

  explicit List(size_type count,
                const allocator_type& alloc = allocator_type());

  List(const List& other);

  List(List&& other) noexcept;

  List(std::initializer_list<value_type> init,
       const allocator_type& alloc = allocator_type());

  ~List() {
    clear();
    rebind_alloc_traits::deallocate(alloc_, fictive_, 1);
  }

  [[nodiscard]] size_type size() const {
    return size_;
  }

  [[nodiscard]] bool empty() const {
    return size_ == 0;
  }

  allocator_type get_allocator() const noexcept {
    return alloc_;
  }

  void clear();

  iterator begin() const {
    return iterator(head_ == nullptr ? fictive_ : head_);
  }
  const_iterator cbegin() const {
    return const_iterator(head_ == nullptr ? fictive_ : head_);
  }

  iterator end() const {
    return iterator(fictive_);
  }
  const_iterator cend() const {
    return const_iterator(fictive_);
  }

  /*
  reverse_iterator rbegin() const {
    return reverse_iterator(tail_);
  }
  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(tail_);
  }

  reverse_iterator rend() const {
    return reverse_iterator(fictive_);
  }
  const_reverse_iterator crend() const {
    return const_reverse_iterator(fictive_);
  }
  */

  reference front() {
    return head_->value;
  }
  const_reference front() const {
    return head_->value;
  }

  reference back() {
    return tail_->value;
  }
  const_reference back() const {
    return tail_->value;
  }

  template <typename... Args>
  iterator emplace(iterator it, Args&&... args);

  template <typename U>
  iterator insert(iterator it, U&& value);

  template <typename... Args>
  void emplace_back(Args&&... args);

  template <typename U>
  void push_back(U&& value);

  template <typename... Args>
  void emplace_front(Args&&... args);

  template <typename U>
  void push_front(U&& value);

  void pop_back();

  void pop_front();

  List<T, Alloc>& operator=(const List<T, Alloc>& other);

  List<T, Alloc>& operator=(List<T, Alloc>&& other) noexcept;

  /*
  iterator erase(const_iterator it);
  iterator erase(const_iterator first, const_iterator last);
*/

  private:
  struct Node;

  using alloc_traits = std::allocator_traits<allocator_type>;
  using rebind_allocator_type =
      typename alloc_traits::template rebind_alloc<Node>;
  using rebind_alloc_traits =
      typename alloc_traits::template rebind_traits<Node>;

  size_type size_ = 0;
  rebind_allocator_type alloc_ = allocator_type();
  Node* head_ = nullptr;
  Node* tail_ = nullptr;
  Node* fictive_ = nullptr;

  void swap(List<T, Alloc>& other);

  template <typename... Args>
  void exception_safe_fill(size_type count, Args&&... args);

  void exception_safe_fill(const List<T, Alloc>& other);

  void exception_safe_fill(std::initializer_list<value_type> init);

  template <typename... Args>
  void fill_construct(Args&&... args);

  template <typename... Args>
  static Node* create_node(rebind_allocator_type& alloc, Args&&... args) {
    Node* node = rebind_alloc_traits::allocate(alloc, 1);
    try {
      rebind_alloc_traits::construct(alloc, node, std::forward<Args>(args)...);
    } catch (...) {
      rebind_alloc_traits::deallocate(alloc, node, 1);
      throw;
    }

    return node;
  }

  static void destroy_node(rebind_allocator_type& alloc, Node* node) {
    rebind_alloc_traits::destroy(alloc, node);
    rebind_alloc_traits::deallocate(alloc, node, 1);
  }

  void fictive_normalize() {
    fictive_->prev = tail_;
    fictive_->next = head_;

    if (head_ != nullptr) {
      head_->prev = fictive_;
    }

    if (tail_ != nullptr) {
      tail_->next = fictive_;
    }
  }

  void fictive_allocate() {
    fictive_ = rebind_alloc_traits::allocate(alloc_, 1);
    fictive_normalize();
  }
};

template <typename T, typename Alloc>
struct List<T, Alloc>::Node {
  Node() = default;
  Node(const Node& other) : value(other.value) {
  }
  Node(Node&& other) noexcept
      : next(other.next), prev(other.prev), value(std::move(other.value)) {
    next = nullptr;
    prev = nullptr;
  }

  explicit Node(const_reference value) : value(value) {
  }
  explicit Node(rvalue_reference value) : value(std::move(value)) {
  }

  template <typename... Args>
  explicit Node(Node* next, Node* prev, Args&&... args)
      : next(next), prev(prev), value(std::forward<Args>(args)...) {
  }

  ~Node() = default;

  Node& operator=(const Node& other) {
    value = other.value;
    next = other.next;
    prev = other.prev;
  }

  Node* next = nullptr;
  Node* prev = nullptr;

  value_type value;
};

template <typename T, typename Alloc>
template <bool is_const>
class List<T, Alloc>::CommonIterator {
  public:
  using difference_type = std::ptrdiff_t;
  using value_type = List<T, Alloc>::value_type;
  using pointer =
      std::conditional_t<is_const, const List<T, Alloc>::value_type*,
                         List<T, Alloc>::value_type*>;
  using reference =
      std::conditional_t<is_const, List<T, Alloc>::const_reference,
                         List<T, Alloc>::reference>;
  using iterator_category = std::bidirectional_iterator_tag;

  friend class List<T, Alloc>;

  private:
  using node_t = std::conditional_t<is_const, const Node, Node>;
  using ptr_t = node_t*;

  ptr_t ptr_ = nullptr;

  public:
  explicit CommonIterator(ptr_t ptr) : ptr_(ptr) {
  }
  CommonIterator() = delete;

  explicit operator CommonIterator<true>() {
    return CommonIterator<true>(ptr_);
  }

  value_type operator*() const {
    return ptr_->value;
  }

  reference operator*() {
    return ptr_->value;
  }

  pointer operator->() {
    return &(ptr_->value);
  }

  constexpr CommonIterator<is_const>& operator++() {
    ptr_ = ptr_->next;
    return *this;
  }

  constexpr CommonIterator<is_const>& operator--() {
    ptr_ = ptr_->prev;
    return *this;
  }

  constexpr const CommonIterator<is_const> operator++(int) {
    CommonIterator<is_const> tmp = *this;
    ++(*this);
    return tmp;
  }

  constexpr const CommonIterator<is_const> operator--(int) {
    CommonIterator<is_const> tmp = *this;
    --(*this);
    return tmp;
  }

  constexpr bool operator==(const CommonIterator<is_const>& other) const {
    return ptr_ == other.ptr_;
  }

  constexpr bool operator!=(const CommonIterator<is_const>& other) const {
    return !(*this == other);
  }
};

template <typename T, typename Alloc>
List<T, Alloc>::List(List&& other) noexcept
    : size_(other.size_)
    , alloc_(std::move(other.alloc_))
    , head_(other.head_)
    , tail_(other.tail_)
    , fictive_(other.fictive_) {
  other.size_ = 0;
  other.head_ = nullptr;
  other.tail_ = nullptr;

  other.fictive_allocate();
}

template <typename T, typename Alloc>
void List<T, Alloc>::exception_safe_fill(const List<T, Alloc>& other) {
  size_ = other.size();
  Node* prev = fictive_;
  Node* cur = nullptr;
  Node* other_cur = other.head_;

  try {
    while (other_cur != other.fictive_) {
      cur = create_node(alloc_, *other_cur);
      prev->next = cur;
      cur->prev = prev;
      prev = cur;
      other_cur = other_cur->next;
    }
  } catch (...) {
    cur = prev;

    while (cur != fictive_) {
      prev = prev->prev;
      destroy_node(alloc_, cur);
      cur = prev;
    }

    rebind_alloc_traits::deallocate(alloc_, fictive_, 1);
    throw;
  }

  head_ = fictive_->next;
  tail_ = prev;
  fictive_normalize();
}

template <typename T, typename Alloc>
template <typename... Args>
void List<T, Alloc>::exception_safe_fill(size_type count, Args&&... args) {
  size_ = count;
  Node* prev = fictive_;
  Node* cur = nullptr;
  size_t i = 0;

  try {
    for (i = 0; i < count; ++i) {
      cur = create_node(alloc_, std::forward<Args>(args)...);
      cur->prev = prev;
      prev->next = cur;
      prev = cur;
    }
  } catch (...) {
    cur = prev;

    while (cur != fictive_) {
      prev = prev->prev;
      destroy_node(alloc_, cur);
      cur = prev;
    }
    rebind_alloc_traits::deallocate(alloc_, fictive_, 1);
    throw;
  }

  head_ = fictive_->next;
  tail_ = prev;
  fictive_normalize();
}

template <typename T, typename Alloc>
void List<T, Alloc>::exception_safe_fill(
    std::initializer_list<value_type> init) {
  size_ = init.size();
  Node* prev = fictive_;
  Node* cur = nullptr;

  try {
    for (auto it = init.begin(); it != init.end(); ++it) {
      cur = create_node(alloc_, *it);
      cur->prev = prev;
      prev->next = cur;
      prev = cur;
    }
  } catch (...) {
    cur = prev;

    while (cur != fictive_) {
      prev = prev->prev;
      destroy_node(alloc_, cur);
      cur = prev;
    }
    rebind_alloc_traits::deallocate(alloc_, fictive_, 1);
    throw;
  }

  head_ = fictive_->next;
  tail_ = prev;
  fictive_normalize();
}

template <typename T, typename Alloc>
template <typename... Args>
void List<T, Alloc>::fill_construct(Args&&... args) {
  if (empty()) {
    return;
  }

  fictive_allocate();
  exception_safe_fill(std::forward<Args>(args)...);
}

template <typename T, typename Alloc>
List<T, Alloc>::List(const List& other)
    : size_(other.size_)
    , alloc_(
          alloc_traits::select_on_container_copy_construction(other.alloc_)) {
  fill_construct(other);
}

template <typename T, typename Alloc>
List<T, Alloc>::List(size_type count, const_reference value,
                     const allocator_type& alloc /*= allocator_type()*/)
    : size_(count), alloc_(alloc) {
  fill_construct(count, value);
}

template <typename T, typename Alloc>
List<T, Alloc>::List(size_type count,
                     const allocator_type& alloc /*= allocator_type()*/)
    : size_(count), alloc_(alloc) {
  fill_construct(count);
}

template <typename T, typename Alloc>
List<T, Alloc>::List(std::initializer_list<value_type> init,
                     const allocator_type& alloc /*= allocator_type()*/)
    : size_(init.size()), alloc_(alloc) {
  fill_construct(init);
}

template <typename T, typename Alloc>
void List<T, Alloc>::clear() {
  if (empty()) {
    return;
  }

  Node* cur = head_;
  Node* next = cur->next;
  while (cur != fictive_) {
    destroy_node(alloc_, cur);
    cur = next;
    next = next->next;
  }

  size_ = 0;
  head_ = nullptr;
  tail_ = nullptr;
  fictive_->next = nullptr;
  fictive_->prev = nullptr;
}

template <typename T, typename Alloc>
template <typename... Args>
typename List<T, Alloc>::iterator List<T, Alloc>::emplace(iterator it,
                                                          Args&&... args) {
  Node* new_node = nullptr;
  try {
    new_node =
        create_node(alloc_, nullptr, nullptr, std::forward<Args>(args)...);
  } catch (...) {
    destroy_node(alloc_, new_node);
    rebind_alloc_traits::deallocate(alloc_, fictive_, 1);
    throw;
  }
  ++size_;
  if (size_ == 1) {
    head_ = new_node;
    tail_ = new_node;
    fictive_normalize();
    return iterator(new_node);
  }
  Node* next = it.ptr_;
  Node* prev = next->prev;
  if (next == fictive_) {
    tail_ = new_node;
  }
  if (next == head_) {
    head_ = new_node;
  }
  prev->next = new_node;
  next->prev = new_node;
  new_node->next = next;
  new_node->prev = prev;
  return iterator(new_node);
}

template <typename T, typename Alloc>
template <typename U>
typename List<T, Alloc>::iterator List<T, Alloc>::insert(iterator it,
                                                         U&& value) {
  static_assert(std::is_same_v<T, std::remove_reference_t<U>>);

  return emplace(it, std::forward<U>(value));
}

template <typename T, typename Alloc>
template <typename... Args>
void List<T, Alloc>::emplace_back(Args&&... args) {
  emplace(end(), std::forward<Args>(args)...);
}

template <typename T, typename Alloc>
template <typename U>
void List<T, Alloc>::push_back(U&& value) {
  emplace_back(std::forward<U>(value));
}

template <typename T, typename Alloc>
template <typename... Args>
void List<T, Alloc>::emplace_front(Args&&... args) {
  emplace(begin(), std::forward<Args>(args)...);
}

template <typename T, typename Alloc>
template <typename U>
void List<T, Alloc>::push_front(U&& value) {
  emplace_front(std::forward<U>(value));
}

template <typename T, typename Alloc>
void List<T, Alloc>::pop_back() {
  if (empty()) {
    throw std::out_of_range("Pop empty list\n");
  }

  --size_;

  Node* tmp = tail_;
  Node* new_tail = tmp->prev;
  new_tail->next = fictive_;
  fictive_->prev = tail_;
  tail_ = new_tail;

  destroy_node(alloc_, tmp);

  if (empty()) {
    head_ = nullptr;
    tail_ = nullptr;
  }
}

template <typename T, typename Alloc>
void List<T, Alloc>::pop_front() {
  if (empty()) {
    throw std::out_of_range("Pop empty list\n");
  }

  --size_;

  Node* tmp = head_;
  Node* new_head = tmp->next;
  new_head->prev = fictive_;
  fictive_->next = new_head;
  head_ = new_head;

  destroy_node(alloc_, tmp);

  if (empty()) {
    head_ = nullptr;
    tail_ = nullptr;
  }
}

template <typename T, typename Alloc>
void List<T, Alloc>::swap(List<T, Alloc>& other) {
  std::swap(size_, other.size_);
  std::swap(head_, other.head_);
  std::swap(tail_, other.tail_);
  std::swap(fictive_, other.fictive_);
  std::swap(alloc_, other.alloc_);
}

template <typename T, typename Alloc>
List<T, Alloc>& List<T, Alloc>::operator=(List<T, Alloc>&& other) noexcept {
  List<T, Alloc> tmp = std::move(other);
  swap(tmp);
  return *this;
}

template <typename T, typename Alloc>
List<T, Alloc>& List<T, Alloc>::operator=(const List<T, Alloc>& other) {
  List<T, Alloc> tmp = other;
  swap(tmp);
  if constexpr (rebind_alloc_traits::propagate_on_container_copy_assignment::
                    value) {
    if (alloc_ != other.alloc_) {
      alloc_ = other.alloc_;
    }
  }

  return *this;
}
