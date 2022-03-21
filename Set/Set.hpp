#include <algorithm>
#include <iostream>
#include <iterator>

#define PARTTWO
#define PARTTHREE

const int kInf = 1e9;

template <typename T>
struct Cmp {
  constexpr bool operator()(const T& op1, const T& op2) const {
    return op1 < op2;
  }
};

template <bool statement, typename If, typename Else>
struct Conditional {
  using type = If;
};

template <typename If, typename Else>
struct Conditional<false, If, Else> {
  using type = Else;
};

template <bool statement, typename If, typename Else>
using conditional_t = typename Conditional<statement, If, Else>::type;

template <typename T, typename U, typename Compare = std::less<T>>
class AVL {
 protected:
  struct Node;
  Node* root_ = nullptr;
  Node* begin_ = nullptr;
  Node* end_ = nullptr;
  Node* rbegin_ = nullptr;
  Node* rend_ = nullptr;
  static Compare cmp;

 private:
  size_t size_ = 0;

  static Node* RotateLeft(Node* node);
  static Node* BigRotateLeft(Node* node);
  static Node* RotateRight(Node* node);
  static Node* BigRotateRight(Node* node);
  static Node* Balance(Node* node);
  static int Diff(Node* node);
  static Node* InsertNode(Node* node, const std::pair<T, U>& data);
  static Node* CreateNode(Node* parent, bool left, const T& key = T(),
                          const U& data = U());
  static void ClearSubtree(Node* node);
  static void UpdatePtr(Node* a, Node* b);
  static Node* CopySubTree(Node* node);
  static Node* EraseNode(Node* node, const T& key);
  static Node* EraseNode(Node* node);
  static void Update(Node* node);
  static Node* TwoSonsCase(Node* node);

 protected:
  static Node* FindNode(Node* node, const T& key);
  static Node* UpperBound(Node* node, Node* ub, const T& elem);
  static Node* LowerBound(Node* node, Node* lb, const T& elem);

  void CutMockNode();
  void BindMockNode();
  void UpdateSides();

 public:
  AVL();
  AVL(const AVL<T, U, Compare>& other);
  void Insert(const std::pair<T, U>& data);
  void Erase(const T& key);
  bool Find(const T& key);
  void Clear();
  size_t Size() const;
  U operator[](const T& key);
  AVL& operator=(AVL<T, U, Compare> other);
  ~AVL() {
    delete end_;
    delete rend_;
    Clear();
  }
};

template <typename T, typename U, typename Compare>
Compare AVL<T, U, Compare>::cmp = Compare();

template <typename T, typename U, typename Compare>
struct AVL<T, U, Compare>::Node {
  Node() = default;
  Node(const Node& node)
      : left(node.left),
        right(node.right),
        parent(node.parent),
        height(node.height),
        key(node.key),
        data(node.data) {}
  Node* left = nullptr;
  Node* right = nullptr;
  Node* parent = nullptr;
  int height = 1;
  T key = T();
  U data = U();
};

template <typename T, typename U, typename Compare>
AVL<T, U, Compare>::AVL() {
  end_ = new Node;
  end_->height = 0;
  rend_ = new Node;
  rend_->height = 0;
}

template <typename T, typename U, typename Compare>
AVL<T, U, Compare>::AVL(const AVL<T, U, Compare>& other) : size_(other.size_) {
  if (other.root_ == nullptr) {
    end_ = new Node;
    end_->height = 0;
    rend_ = new Node;
    rend_->height = 0;
    return;
  }
  root_ = CopySubTree(other.root_);
  Node* cur = root_;
  Node* prev = cur;
  while (cur->left != nullptr) {
    prev = cur;
    cur = prev->left;
  }
  rend_ = cur;
  begin_ = rend_->parent;
  cur = root_;
  prev = cur;
  while (cur->right != nullptr) {
    prev = cur;
    cur = prev->right;
  }
  end_ = cur;
  rbegin_ = end_->parent;
}

template <typename T, typename U, typename Compare>
typename AVL<T, U, Compare>::Node* AVL<T, U, Compare>::CopySubTree(Node* node) {
  if (node == nullptr) {
    return nullptr;
  }
  Node* new_node = new Node(*node);
  new_node->left = CopySubTree(node->left);
  new_node->right = CopySubTree(node->right);
  if (new_node->left != nullptr) {
    new_node->left->parent = new_node;
  }
  if (new_node->right != nullptr) {
    new_node->right->parent = new_node;
  }
  return new_node;
}

template <typename T, typename U, typename Compare>
AVL<T, U, Compare>& AVL<T, U, Compare>::operator=(AVL<T, U, Compare> other) {
  std::swap(root_, other.root_);
  std::swap(size_, other.size_);
  std::swap(begin_, other.begin_);
  std::swap(end_, other.end_);
  std::swap(rbegin_, other.rbegin_);
  std::swap(rend_, other.rend_);
  return *this;
}

template <typename T, typename U, typename Compare>
void AVL<T, U, Compare>::CutMockNode() {
  if (begin_ != nullptr) {
    begin_->left = nullptr;
  }
  if (rbegin_ != nullptr) {
    rbegin_->right = nullptr;
  }
}

template <typename T, typename U, typename Compare>
void AVL<T, U, Compare>::BindMockNode() {
  if (begin_ != nullptr) {
    begin_->left = rend_;
  }
  rend_->parent = begin_;
  if (rbegin_ != nullptr) {
    rbegin_->right = end_;
  }
  end_->parent = rbegin_;
}

template <typename T, typename U, typename Compare>
void AVL<T, U, Compare>::UpdateSides() {
  if (root_ == nullptr) {
    begin_ = nullptr;
    return;
  }
  Node* cur = root_;
  Node* prev = cur;
  while (cur->left != rend_ && cur->left != nullptr) {
    prev = cur;
    cur = prev->left;
  }
  begin_ = cur;
  cur = root_;
  prev = cur;
  while (cur->right != end_ && cur->right != nullptr) {
    prev = cur;
    cur = prev->right;
  }
  rbegin_ = cur;
  BindMockNode();
}

template <typename T, typename U, typename Compare>
size_t AVL<T, U, Compare>::Size() const {
  return size_;
}

template <typename T, typename U, typename Compare>
typename AVL<T, U, Compare>::Node* AVL<T, U, Compare>::CreateNode(
    Node* parent, bool left, const T& key, const U& data) {
  Node* new_node = new Node;
  if (left) {
    parent->left = new_node;
  } else {
    parent->right = new_node;
  }
  new_node->parent = parent;
  new_node->key = key;
  new_node->data = data;
  return new_node;
}

template <typename T, typename U, typename Compare>
int AVL<T, U, Compare>::Diff(Node* node) {
  int ans = 0;
  if (node == nullptr) {
    ans = 0;
  } else if (node->left != nullptr && node->right != nullptr) {
    ans = node->left->height - node->right->height;
  } else if (node->left != nullptr) {
    ans = node->left->height;
  } else if (node->right != nullptr) {
    ans = 0 - node->right->height;
  }
  return ans;
}

template <typename T, typename U, typename Compare>
typename AVL<T, U, Compare>::Node* AVL<T, U, Compare>::Balance(Node* node) {
  Node* new_root = node;
  if (Diff(node) == -2) {
    if (Diff(node->right) == 1) {
      new_root = BigRotateLeft(node);
    } else {
      new_root = RotateLeft(node);
    }
  } else if (Diff(node) == 2) {
    if (Diff(node->left) == -1) {
      new_root = BigRotateRight(node);
    } else {
      new_root = RotateRight(node);
    }
  }
  return new_root;
}

template <typename T, typename U, typename Compare>
void AVL<T, U, Compare>::Update(Node* node) {
  if (node == nullptr) {
    return;
  }
  size_t h_l = (node->left == nullptr ? 0 : node->left->height);
  size_t h_r = (node->right == nullptr ? 0 : node->right->height);
  node->height = std::max(h_l, h_r) + 1;
}

template <typename T, typename U, typename Compare>
void AVL<T, U, Compare>::UpdatePtr(Node* a, Node* b) {
  if (a->parent != nullptr) {
    if (a->parent->left == a) {
      a->parent->left = b;
    } else if (a->parent->right == a) {
      a->parent->right = b;
    }
  }
  if (a != nullptr && b != nullptr) {
    b->parent = a->parent;
  }
}

template <typename T, typename U, typename Compare>
typename AVL<T, U, Compare>::Node* AVL<T, U, Compare>::RotateLeft(Node* node) {
  if (node == nullptr) {
    return nullptr;
  }
  Node* a = node;
  Node* b = a->right;
  Node* p = a->left;
  Node* q = (b != nullptr ? b->left : nullptr);
  Node* r = (b != nullptr ? b->right : nullptr);
  UpdatePtr(a, b);
  if (a != nullptr && b != nullptr) {
    a->parent = b;
    b->left = a;
    a->right = q;
    if (q != nullptr) {
      q->parent = a;
    }
    int h_p = (p == nullptr ? 0 : p->height);
    int h_q = (q == nullptr ? 0 : q->height);
    int h_r = (r == nullptr ? 0 : r->height);
    a->height = std::max(h_p, h_q) + 1;
    b->height = std::max(a->height, h_r) + 1;
  }
  return b;
}

template <typename T, typename U, typename Compare>
typename AVL<T, U, Compare>::Node* AVL<T, U, Compare>::RotateRight(Node* node) {
  if (node == nullptr) {
    return nullptr;
  }
  Node* a = node;
  Node* b = a->left;
  Node* p = a->right;
  Node* q = (b != nullptr ? b->left : nullptr);
  Node* r = (b != nullptr ? b->right : nullptr);
  UpdatePtr(a, b);
  if (a != nullptr && b != nullptr) {
    a->parent = b;
    b->right = a;
    a->left = r;
    if (r != nullptr) {
      r->parent = a;
    }
    int h_p = (p == nullptr ? 0 : p->height);
    int h_q = (q == nullptr ? 0 : q->height);
    int h_r = (r == nullptr ? 0 : r->height);
    a->height = std::max(h_r, h_p) + 1;
    b->height = std::max(h_q, a->height) + 1;
  }
  return b;
}

template <typename T, typename U, typename Compare>
typename AVL<T, U, Compare>::Node* AVL<T, U, Compare>::BigRotateLeft(
    Node* node) {
  RotateRight(node->right);
  return RotateLeft(node);
}

template <typename T, typename U, typename Compare>
typename AVL<T, U, Compare>::Node* AVL<T, U, Compare>::BigRotateRight(
    Node* node) {
  RotateLeft(node->left);
  return RotateRight(node);
}

template <typename T, typename U, typename Compare>
typename AVL<T, U, Compare>::Node* AVL<T, U, Compare>::FindNode(Node* node,
                                                                const T& key) {
  Node* ans = nullptr;
  if (node == nullptr) {
    return ans;
  }
  if (!cmp(key, node->key) && !cmp(node->key, key)) {
    ans = node;
  } else if (cmp(key, node->key)) {
    ans = FindNode(node->left, key);
  } else if (cmp(node->key, key)) {
    ans = FindNode(node->right, key);
  }
  return ans;
}

template <typename T, typename U, typename Compare>
bool AVL<T, U, Compare>::Find(const T& key) {
  CutMockNode();
  bool ans = FindNode(root_, key) != nullptr;
  BindMockNode();
  return ans;
}

template <typename T, typename U, typename Compare>
typename AVL<T, U, Compare>::Node* AVL<T, U, Compare>::InsertNode(
    Node* node, const std::pair<T, U>& data) {
  Node* subtree_root = node;
  if (cmp(data.first, node->key)) {
    if (node->left == nullptr) {
      CreateNode(node, true, data.first, data.second);
    } else {
      InsertNode(node->left, data);
    }
  } else if (cmp(node->key, data.first)) {
    if (node->right == nullptr) {
      CreateNode(node, false, data.first, data.second);
    } else {
      InsertNode(node->right, data);
    }
  }
  Update(node);
  return Balance(subtree_root);
}

template <typename T, typename U, typename Compare>
void AVL<T, U, Compare>::Insert(const std::pair<T, U>& data) {
  if (Find(data.first)) {
    return;
  }
  CutMockNode();
  ++size_;
  if (root_ == nullptr) {
    root_ = new Node;
    root_->key = data.first;
    root_->data = data.second;
  } else {
    root_ = InsertNode(root_, data);
  }
  UpdateSides();
}

template <typename T, typename U, typename Compare>
typename AVL<T, U, Compare>::Node* AVL<T, U, Compare>::TwoSonsCase(Node* node) {
  Node* cur = node->right;
  Node* prev = cur;
  while (cur->left != nullptr) {
    prev = cur;
    cur = prev->left;
  }
  node->key = cur->key;
  node->data = cur->data;
  cur = EraseNode(cur);
  prev = cur;
  if (cur != node) {
    while (prev != node->right) {
      prev = Balance(cur);
      cur = prev->parent;
    }
  }
  return cur;
}

template <typename T, typename U, typename Compare>
typename AVL<T, U, Compare>::Node* AVL<T, U, Compare>::EraseNode(Node* node,
                                                                 const T& key) {
  if (node == nullptr) {
    return nullptr;
  }
  Node* subtree_root = node;
  if (cmp(key, node->key)) {
    EraseNode(node->left, key);
  } else if (cmp(node->key, key)) {
    EraseNode(node->right, key);
  } else {
    if (node->left != nullptr && node->right != nullptr) {
      subtree_root = TwoSonsCase(node);
    } else {
      subtree_root = EraseNode(node);
    }
  }
  return Balance(subtree_root);
}

template <typename T, typename U, typename Compare>
typename AVL<T, U, Compare>::Node* AVL<T, U, Compare>::EraseNode(Node* node) {
  if (node->left != nullptr && node->right != nullptr) {
    return node;
  }
  if (node->left == nullptr && node->right == nullptr) {
    Node* par = node->parent;
    if (par != nullptr) {
      (par->left == node ? par->left : par->right) = nullptr;
    }
    Update(par);
    delete node;
    return par;
  }
  Node* kid = (node->left != nullptr ? node->left : node->right);
  kid->parent = node->parent;
  if (node->parent != nullptr) {
    (node->parent->left == node ? node->parent->left : node->parent->right) =
        kid;
  }
  Update(node->parent);
  delete node;
  return kid;
}

template <typename T, typename U, typename Compare>
void AVL<T, U, Compare>::Erase(const T& key) {
  if (!Find(key)) {
    return;
  }
  if (root_ == nullptr) {
    return;
  }
  --size_;
  if (size_ == 0) {
    delete root_;
    root_ = nullptr;
    begin_ = nullptr;
    rbegin_ = nullptr;
    return;
  }
  CutMockNode();
  root_ = EraseNode(root_, key);
  UpdateSides();
}

template <typename T, typename U, typename Compare>
void AVL<T, U, Compare>::ClearSubtree(Node* node) {
  if (node == nullptr) {
    return;
  }
  if (node->left != nullptr) {
    ClearSubtree(node->left);
  }
  if (node->right != nullptr) {
    ClearSubtree(node->right);
  }
  delete node;
}

template <typename T, typename U, typename Compare>
void AVL<T, U, Compare>::Clear() {
  CutMockNode();
  ClearSubtree(root_);
  size_ = 0;
  root_ = nullptr;
  begin_ = nullptr;
  rbegin_ = nullptr;
}

template <typename T, typename U, typename Compare>
U AVL<T, U, Compare>::operator[](const T& key) {
  Node* ans = FindNode(root_, key);
  if (ans == nullptr) {
    return U();
  }
  return ans->data;
}

template <typename T, typename U, typename Compare>
typename AVL<T, U, Compare>::Node* AVL<T, U, Compare>::LowerBound(
    Node* node, Node* lb, const T& elem) {
  if (node == nullptr) {
    return lb;
  }

  Node* ans = nullptr;
  if (!cmp(node->key, elem)) {
    if (!cmp(lb->key, node->key)) {
      lb = node;
    }
    ans = LowerBound(node->left, lb, elem);
  } else {
    ans = LowerBound(node->right, lb, elem);
  }
  return ans;
}

template <typename T, typename U, typename Compare>
typename AVL<T, U, Compare>::Node* AVL<T, U, Compare>::UpperBound(
    Node* node, Node* ub, const T& elem) {
  if (node == nullptr) {
    return ub;
  }
  Node* ans = nullptr;
  if (cmp(elem, node->key)) {
    if (cmp(node->key, ub->key)) {
      ub = node;
    }
    ans = UpperBound(node->left, ub, elem);
  } else {
    ans = UpperBound(node->right, ub, elem);
  }
  return ans;
}

template <typename Iterator>
class MyReverseIterator {
 private:
  using iterator_type = Iterator;
  using iterator_concept = typename Iterator::iterator_category;
  using iterator_category = typename Iterator::iterator_category;
  using value_type = typename Iterator::value_type;
  using difference_type = typename Iterator::difference_type;
  using pointer = typename Iterator::pointer;
  using reference = typename Iterator::reference;

  Iterator it_;

 public:
  constexpr MyReverseIterator();

  constexpr explicit MyReverseIterator(const iterator_type& it);

  template <typename T>
  constexpr MyReverseIterator(const MyReverseIterator<T>& other);

  constexpr MyReverseIterator<Iterator>& operator++();

  constexpr MyReverseIterator<Iterator>& operator--();

  constexpr MyReverseIterator<Iterator> operator++(int);

  constexpr MyReverseIterator<Iterator> operator--(int);

  template <class U>
  constexpr MyReverseIterator& operator=(const MyReverseIterator<U>& other);

  constexpr reference operator*();

  constexpr pointer operator->();

  constexpr bool operator==(const MyReverseIterator<Iterator>& other) const;

  constexpr bool operator!=(const MyReverseIterator<Iterator>& other) const;

  constexpr Iterator base() const;  // NOLINT
};

template <typename Iterator>
constexpr MyReverseIterator<Iterator>::MyReverseIterator() : it_() {}

template <typename Iterator>
constexpr MyReverseIterator<Iterator>::MyReverseIterator(
    const iterator_type& it)
    : it_(it) {}

template <typename Iterator>
template <typename T>
constexpr MyReverseIterator<Iterator>::MyReverseIterator(
    const MyReverseIterator<T>& other) {}

template <typename Iterator>
constexpr MyReverseIterator<Iterator>&
MyReverseIterator<Iterator>::operator++() {
  --it_;
  return *this;
}

template <typename Iterator>
constexpr MyReverseIterator<Iterator>&
MyReverseIterator<Iterator>::operator--() {
  ++it_;
  return *this;
}

template <typename Iterator>
constexpr MyReverseIterator<Iterator> MyReverseIterator<Iterator>::operator++(
    int) {
  return MyReverseIterator<Iterator>(it_--);
}

template <typename Iterator>
constexpr MyReverseIterator<Iterator> MyReverseIterator<Iterator>::operator--(
    int) {
  return MyReverseIterator<Iterator>(it_++);
}

template <typename Iterator>
template <typename U>
constexpr MyReverseIterator<Iterator>& MyReverseIterator<Iterator>::operator=(
    const MyReverseIterator<U>& other) {
  it_ = other.it_;
}

template <typename Iterator>
constexpr typename MyReverseIterator<Iterator>::reference
MyReverseIterator<Iterator>::operator*() {
  return *it_;
}

template <typename Iterator>
constexpr typename MyReverseIterator<Iterator>::pointer
MyReverseIterator<Iterator>::operator->() {
  return &(*it_);
}

template <typename Iterator>
constexpr bool MyReverseIterator<Iterator>::operator==(
    const MyReverseIterator<Iterator>& other) const {
  return it_ == other.it_;
}

template <typename Iterator>
constexpr bool MyReverseIterator<Iterator>::operator!=(
    const MyReverseIterator<Iterator>& other) const {
  return it_ != other.it_;
}

template <typename Iterator>
constexpr Iterator MyReverseIterator<Iterator>::base() const {  // NOLINT
  return it_;
}

namespace std {        // NOLINT
template <typename T>  // NOLINT
inline constexpr bool
    is_same_v<reverse_iterator<T>, MyReverseIterator<T>> =  // NOLINT
    true;                                                   // NOLINT
}  // namespace std

template <typename Key, typename C = std::less<Key>>
class Set : public AVL<Key, Key, C> {
 public:
  Set() = default;
  Set(const Set<Key, C>& set);

  bool Empty() const;
  void Insert(const Key& elem);

  using value_type = Key;
  using value_compare = C;
  using key_compare = C;

  // Bidirectional iterator
  template <bool is_const>
  class BidirectionalIterator;

  using iterator = BidirectionalIterator<true>;
  using const_iterator = BidirectionalIterator<true>;

  using const_reverse_iterator = MyReverseIterator<const_iterator>;
  using reverse_iterator = MyReverseIterator<iterator>;

  iterator begin() const;         // NOLINT
  iterator end() const;           // NOLINT
  const_iterator cbegin() const;  // NOLINT
  const_iterator cend() const;    // NOLINT

  iterator Find(const Key& elem);
  const_iterator Find(const Key& elem) const;

  iterator UpperBound(const Key& elem);
  const_iterator Upperbound(const Key& elem) const;

  iterator LowerBound(const Key& elem);
  const_iterator LowerBound(const Key& elem) const;

  bool operator<(const Set<Key, C>& s) const;

  reverse_iterator rbegin() const;         // NOLINT
  reverse_iterator rend() const;           // NOLINT
  const_reverse_iterator crbegin() const;  // NOLINT
  const_reverse_iterator crend() const;    // NOLINT
};

template <typename Key, typename C>
Set<Key, C>::Set(const Set<Key, C>& set) : AVL<Key, Key, C>(set) {}

template <typename Key, typename C>
bool Set<Key, C>::Empty() const {
  return AVL<Key, Key, C>::Size() == 0;
}

template <typename Key, typename C>
void Set<Key, C>::Insert(const Key& elem) {
  AVL<Key, Key, C>::Insert({elem, elem});
}

template <typename Key, typename C>
typename Set<Key, C>::iterator Set<Key, C>::begin() const {  // NOLINT
  if (Empty()) {
    return iterator(AVL<Key, Key, C>::end_);
  }
  return iterator(AVL<Key, Key, C>::begin_);
}

template <typename Key, typename C>
typename Set<Key, C>::iterator Set<Key, C>::end() const {  // NOLINT
  return iterator(AVL<Key, Key, C>::end_);
}

template <typename Key, typename C>
typename Set<Key, C>::const_iterator Set<Key, C>::cbegin() const {  // NOLINT
  if (Empty()) {
    return const_iterator(AVL<Key, Key, C>::end_);
  }
  return const_iterator(AVL<Key, Key, C>::begin_);
}

template <typename Key, typename C>
typename Set<Key, C>::const_iterator Set<Key, C>::cend() const {  // NOLINT
  return const_iterator(AVL<Key, Key, C>::end_);
}

template <typename Key, typename C>
typename Set<Key, C>::iterator Set<Key, C>::Find(const Key& elem) {
  AVL<Key, Key, C>::CutMockNode();
  typename AVL<Key, Key, C>::Node* ptr =
      AVL<Key, Key, C>::FindNode(AVL<Key, Key, C>::root_, elem);
  AVL<Key, Key, C>::BindMockNode();
  if (ptr == nullptr) {
    return end();
  }
  return iterator(ptr);
}

template <typename Key, typename C>
typename Set<Key, C>::const_iterator Set<Key, C>::Find(const Key& elem) const {
  AVL<Key, Key, C>::CutMockNode();
  const typename AVL<Key, Key, C>::Node* ptr =
      AVL<Key, Key, C>::FindNode(AVL<Key, Key, C>::root_, elem);
  AVL<Key, Key, C>::BindMockNode();
  if (ptr == nullptr) {
    return cend();
  }
  return iterator(ptr);
}

template <typename Key, typename C>
typename Set<Key, C>::iterator Set<Key, C>::UpperBound(const Key& elem) {
  AVL<Key, Key, C>::end_->key = kInf;
  typename AVL<Key, Key, C>::Node* ptr = AVL<Key, Key, C>::UpperBound(
      AVL<Key, Key, C>::root_, AVL<Key, Key, C>::end_, elem);
  AVL<Key, Key, C>::end_->key = Key();
  return iterator(ptr);
}

template <typename Key, typename C>
typename Set<Key, C>::const_iterator Set<Key, C>::Upperbound(
    const Key& elem) const {
  AVL<Key, Key, C>::end_->key = kInf;
  const typename AVL<Key, Key, C>::Node* ptr = AVL<Key, Key, C>::UpperBound(
      AVL<Key, Key, C>::root_, AVL<Key, Key, C>::end_, elem);
  AVL<Key, Key, C>::end_->key = Key();
  return const_iterator(ptr);
}

template <typename Key, typename C>
typename Set<Key, C>::iterator Set<Key, C>::LowerBound(const Key& elem) {
  AVL<Key, Key, C>::rend_->key = kInf;
  typename AVL<Key, Key, C>::Node* ptr = AVL<Key, Key, C>::LowerBound(
      AVL<Key, Key, C>::root_, AVL<Key, Key, C>::rend_, elem);
  AVL<Key, Key, C>::rend_->key = Key();
  if (ptr == AVL<Key, Key, C>::rend_) {
    return end();
  }
  return iterator(ptr);
}

template <typename Key, typename C>
typename Set<Key, C>::const_iterator Set<Key, C>::LowerBound(
    const Key& elem) const {
  AVL<Key, Key, C>::rend_->key = kInf;
  const typename AVL<Key, Key, C>::Node* ptr = AVL<Key, Key, C>::LowerBound(
      AVL<Key, Key, C>::root_, AVL<Key, Key, C>::rend_, elem);
  AVL<Key, Key, C>::end_->key = Key();
  if (ptr == AVL<Key, Key, C>::rend_) {
    return cend();
  }
  return const_iterator(ptr);
}

template <typename Key, typename C>
bool Set<Key, C>::operator<(const Set<Key, C>& s) const {
  bool less = false;
  auto it1 = begin();
  auto it2 = s.begin();
  for (; it1 != end() && it2 != s.end(); ++it1, ++it2) {
    if (AVL<Key, Key, C>::cmp(*it1, *it2)) {
      less = true;
      break;
    }
  }
  if (!less && it1 == end() && it2 != end()) {
    less = true;
  }
  return less;
}

template <typename Key, typename C>
typename Set<Key, C>::reverse_iterator Set<Key, C>::rbegin() const {  // NOLINT
  if (Empty()) {
    return reverse_iterator(AVL<Key, Key, C>::rend_);
  }
  return reverse_iterator(iterator(AVL<Key, Key, C>::rbegin_));
}

template <typename Key, typename C>
typename Set<Key, C>::reverse_iterator Set<Key, C>::rend() const {  // NOLINT
  return reverse_iterator(iterator(AVL<Key, Key, C>::rend_));
}

template <typename Key, typename C>
typename Set<Key, C>::const_reverse_iterator Set<Key, C>::crbegin()  // NOLINT
    const {                                                          // NOLINT
  if (Empty()) {
    return const_reverse_iterator(AVL<Key, Key, C>::rend_);
  }
  return const_reverse_iterator(const_iterator(AVL<Key, Key, C>::rbegin_));
}

template <typename Key, typename C>
typename Set<Key, C>::const_reverse_iterator Set<Key, C>::crend()  // NOLINT
    const {                                                        // NOLINT
  return const_reverse_iterator(
      const_reverse_iterator(AVL<Key, Key, C>::rend_));
}

template <typename Key, typename C>
template <bool is_const>
class Set<Key, C>::BidirectionalIterator {
 public:
  using difference_type = std::ptrdiff_t;
  using value_type = Key;
  using pointer = conditional_t<is_const, const Key*, Key*>;
  using reference = conditional_t<is_const, const Key&, Key&>;
  using iterator_category = std::bidirectional_iterator_tag;

 private:
  using avl_node = typename AVL<Key, Key, C>::Node;
  using node_ptr = conditional_t<is_const, const avl_node*, avl_node*>;

  node_ptr ptr_;
  BidirectionalIterator() = delete;

 public:
  BidirectionalIterator(node_ptr ptr);
  BidirectionalIterator(const BidirectionalIterator<is_const>& it);

  constexpr BidirectionalIterator<is_const>& operator++();

  constexpr BidirectionalIterator<is_const>& operator--();

  constexpr BidirectionalIterator<is_const> operator++(int);

  constexpr BidirectionalIterator<is_const> operator--(int);

  constexpr reference operator*() const;

  constexpr pointer operator->() const;

  constexpr bool operator==(const BidirectionalIterator<is_const>& it) const;

  constexpr bool operator!=(const BidirectionalIterator<is_const>& it) const;

  constexpr bool operator<(const BidirectionalIterator<is_const>& it) const;
};

template <typename Key, typename C>
template <bool is_const>
Set<Key, C>::BidirectionalIterator<is_const>::BidirectionalIterator(
    node_ptr ptr)
    : ptr_(ptr) {}

template <typename Key, typename C>
template <bool is_const>
Set<Key, C>::BidirectionalIterator<is_const>::BidirectionalIterator(
    const BidirectionalIterator<is_const>& it)
    : ptr_(it.ptr_) {}

template <typename Key, typename C>
template <bool is_const>
constexpr typename Set<Key, C>::template BidirectionalIterator<is_const>&
Set<Key, C>::BidirectionalIterator<is_const>::operator++() {
  if (ptr_->right != nullptr) {
    node_ptr cur = ptr_->right;
    node_ptr prev = cur;
    while (cur->left != nullptr) {
      prev = cur;
      cur = prev->left;
    }
    ptr_ = cur;
  } else if (ptr_->parent != nullptr) {
    if (ptr_->parent->left == ptr_) {
      ptr_ = ptr_->parent;
    } else {
      node_ptr cur = ptr_->parent;
      node_ptr prev = cur;
      while (cur->parent != nullptr && prev != cur->left) {
        prev = cur;
        cur = prev->parent;
      }
      if (cur->parent == nullptr && prev != cur->left) {
        ptr_ = nullptr;
      } else {
        ptr_ = cur;
      }
    }
  } else {
    ptr_ = nullptr;
  }
  return *this;
}

template <typename Key, typename C>
template <bool is_const>
constexpr typename Set<Key, C>::template BidirectionalIterator<is_const>&
Set<Key, C>::BidirectionalIterator<is_const>::operator--() {
  if (ptr_->left != nullptr) {
    node_ptr cur = ptr_->left;
    node_ptr prev = cur;
    while (cur->right != nullptr) {
      prev = cur;
      cur = prev->right;
    }
    ptr_ = cur;
  } else if (ptr_->parent != nullptr) {
    if (ptr_->parent->right == ptr_) {
      ptr_ = ptr_->parent;
    } else {
      node_ptr cur = ptr_->parent;
      node_ptr prev = cur;
      while (cur->parent != nullptr && prev != cur->right) {
        prev = cur;
        cur = prev->parent;
      }
      if (cur->parent == nullptr && prev != cur->right) {
        ptr_ = nullptr;
      } else {
        ptr_ = cur;
      }
    }
  } else {
    ptr_ = nullptr;
  }
  return *this;
}

template <typename Key, typename C>
template <bool is_const>
constexpr typename Set<Key, C>::template BidirectionalIterator<is_const>
Set<Key, C>::BidirectionalIterator<is_const>::operator++(int) {
  BidirectionalIterator copy = *this;
  ++*this;
  return copy;
}

template <typename Key, typename C>
template <bool is_const>
constexpr typename Set<Key, C>::template BidirectionalIterator<is_const>
Set<Key, C>::BidirectionalIterator<is_const>::operator--(int) {
  BidirectionalIterator copy = *this;
  --*this;
  return copy;
}

template <typename Key, typename C>
template <bool is_const>
constexpr
    typename Set<Key, C>::template BidirectionalIterator<is_const>::reference
    Set<Key, C>::BidirectionalIterator<is_const>::operator*() const {
  return ptr_->key;
}

template <typename Key, typename C>
template <bool is_const>
constexpr
    typename Set<Key, C>::template BidirectionalIterator<is_const>::pointer
    Set<Key, C>::BidirectionalIterator<is_const>::operator->() const {
  return &(ptr_->key);
}

template <typename Key, typename C>
template <bool is_const>
constexpr bool Set<Key, C>::BidirectionalIterator<is_const>::operator==(
    const BidirectionalIterator<is_const>& it) const {
  return ptr_ == it.ptr_;
}

template <typename Key, typename C>
template <bool is_const>
constexpr bool Set<Key, C>::BidirectionalIterator<is_const>::operator!=(
    const BidirectionalIterator<is_const>& it) const {
  return ptr_ != it.ptr_;
}

template <typename Key, typename C>
template <bool is_const>
constexpr bool Set<Key, C>::BidirectionalIterator<is_const>::operator<(
    const BidirectionalIterator<is_const>& it) const {
  return *(*this) < *it;
}
