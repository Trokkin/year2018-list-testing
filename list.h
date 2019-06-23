#pragma once
#include <iterator>

template <typename T>
class list {
  struct node {
    node* next;
    node* prev;
    node() : next(this), prev(this) {}
    node(node* next, node* prev) : next(next), prev(prev) {}
    virtual ~node() = default;
  };
  struct node_v : node {
    T value;
    node_v(T const& v) : node(nullptr, nullptr), value(v) {}
    node_v(T const& v, node* next, node* prev) : node(next, prev), value(v) {}
  };
  node dummy;

  void copyFrom(list const& other) {
    node* t = other.dummy.next;
    while (t != &other.dummy) {
      push_back(static_cast<node_v*>(t)->value);
      t = t->next;
    }
  }

  template <typename C>
  class iterator_t {
   public:
    node* ref;

    typedef std::ptrdiff_t difference_type;
    typedef C value_type;
    typedef C* pointer;
    typedef C& reference;
    typedef std::bidirectional_iterator_tag iterator_category;

    template <typename V>
    iterator_t(iterator_t<V> const& other,
               typename std::enable_if<std::is_same<C, const V>::value>::type* =
                   nullptr)
        : ref(other.ref) {}
    iterator_t(node& v) : ref(&v) {}

    C& operator*() const { return static_cast<node_v*>(ref)->value; }
    C* operator->() const { return &(static_cast<node_v*>(ref)->value); }

    const iterator_t operator++(int) {
      iterator_t t(*this);
      ref = ref->next;
      return t;
    }
    iterator_t& operator++() {
      ref = ref->next;
      return *this;
    }
    iterator_t operator+(int b) {
      if (b < 0) return operator-(-b);
      node* t = ref;
      while (b-- && ref) t = t->next;
      return iterator_t(t);
    }
    const iterator_t operator--(int) {
      iterator_t t(*this);
      ref = ref->prev;
      return t;
    }
    iterator_t& operator--() {
      ref = ref->prev;
      return *this;
    }
    iterator_t operator-(int b) {
      if (b < 0) return operator+(-b);
      node* t = ref;
      while (b-- && ref) t = t->prev;
      return iterator_t(t);
    }
    friend bool operator==(iterator_t const& a, iterator_t const& b) {
      return a.ref == b.ref;
    }
    friend bool operator!=(iterator_t const& a, iterator_t const& b) {
      return a.ref != b.ref;
    }
  };

 public:
  list() {}
  list(list const& other) : list() { copyFrom(other); }
  list& operator=(list const& other) {
    if (&other != this) {
      clear();
      copyFrom(other);
    }
    return *this;
  }

  ~list() { clear(); }

  bool empty() const noexcept { return dummy.next == &dummy; }
  void clear() noexcept {
    node* n = dummy.next;
    while (n != &dummy) {
      node* t = n;
      n = n->next;
      delete t;
    }
    dummy.next = &dummy;
    dummy.prev = &dummy;
  }

  void push_front(T const& val) {
    dummy.next->prev = new node_v(val, dummy.next, &dummy);
    dummy.next = dummy.next->prev;
  }
  void pop_front() {
    if (!empty()) {
      node* t = dummy.next;
      dummy.next = dummy.next->next;
      dummy.next->prev = &dummy;
      delete t;
    } else {
      throw std::runtime_error("Tried to pop_front an empty list!");
    }
  }
  T const& front() const noexcept {
    return static_cast<node_v*>(dummy.next)->value;
  }
  T& front() noexcept { return static_cast<node_v*>(dummy.next)->value; }

  void push_back(T const& val) {
    dummy.prev->next = new node_v(val, &dummy, dummy.prev);
    dummy.prev = dummy.prev->next;
  }
  void pop_back() {
    if (!empty()) {
      auto removed = dummy.prev;
      dummy.prev->prev->next = &dummy;
      dummy.prev = dummy.prev->prev;
      delete removed;
    } else {
      throw std::runtime_error("Tried to pop_back an empty list!");
    }
  }
  T const& back() const noexcept {
    return static_cast<node_v*>(dummy.prev)->value;
  }
  T& back() noexcept { return static_cast<node_v*>(dummy.prev)->value; }

  typedef iterator_t<const T> const_iterator;
  typedef iterator_t<T> iterator;
  typedef std::reverse_iterator<const_iterator> reverse_const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;

  const_iterator begin() const noexcept {
    return const_iterator(*(dummy.next));
  }
  const_iterator end() const noexcept {
    return const_iterator(*(dummy.prev->next));
  }
  iterator begin() noexcept { return iterator(*(dummy.next)); }
  iterator end() noexcept { return iterator(*(dummy.prev->next)); }
  reverse_const_iterator rbegin() const noexcept {
    return reverse_const_iterator(*(dummy.prev->next));
  }
  reverse_const_iterator rend() const noexcept {
    return reverse_const_iterator(*(dummy.next));
  }
  reverse_iterator rbegin() noexcept {
    return reverse_iterator(*(dummy.prev->next));
  }
  reverse_iterator rend() noexcept { return reverse_iterator(*(dummy.next)); }

  iterator insert(const_iterator pos, T const& v) {
    node_v* n = new node_v(v, pos.ref, pos.ref->prev);
    n->next->prev = n;
    n->prev->next = n;
    return iterator(*n);
  }

  iterator erase(const_iterator pos) {
    if (!empty()) {
      pos.ref->next->prev = pos.ref->prev;
      pos.ref->prev->next = pos.ref->next;
      iterator r = iterator(*(pos.ref->next));
      delete pos.ref;
      return r;
    }
    throw std::runtime_error("empty container");
  }
  iterator splice(const_iterator pos, list& other, const_iterator first,
                  const_iterator last) {
    if (first.ref != last.ref) {
      last.ref->prev->next = pos.ref;
      pos.ref->prev->next = first.ref;
      first.ref->prev->next = last.ref;
      node* t = pos.ref->prev;
      pos.ref->prev = last.ref->prev;
      last.ref->prev = first.ref->prev;
      first.ref->prev = t;
    }
    return iterator(*(pos.ref));
  }

  template <typename V>
  friend void swap(list<V>&, list<V>&) noexcept;
};

template <typename V>
void swap(list<V>& a, list<V>& b) noexcept {
  if (a.empty()) {
    a.dummy.prev = &b.dummy;
    a.dummy.next = &b.dummy;
  } else {
    a.dummy.prev->next = &b.dummy;
    a.dummy.next->prev = &b.dummy;
  }
  if (b.empty()) {
    b.dummy.prev = &a.dummy;
    b.dummy.next = &a.dummy;
  } else {
    b.dummy.prev->next = &a.dummy;
    b.dummy.next->prev = &a.dummy;
  }
  std::swap(a.dummy, b.dummy);
}
