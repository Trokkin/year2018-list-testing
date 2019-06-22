#pragma once
#include <iterator>

template <typename T>
class list {
  struct node {
    node* next;
    node* prev;
    node() : next(nullptr), prev(nullptr) {}
    node(node* next, node* prev) : next(next), prev(prev) {}
  };
  struct node_v : node {
    T value;
    node_v(T const& v) : node(nullptr, nullptr), value(v) {}
    node_v(T const& v, node* next, node* prev) : node(next, prev), value(v) {}
  };
  node* dummy;

  void copyFrom(list const& other) {
    dummy->next = dummy;
    dummy->prev = dummy;
    node* t = other.dummy->next;
    while (t != other.dummy) {
      push_back(static_cast<node_v*>(t)->value);
      t = t->next;
    }
  }

  template <typename C>
  class iterator_t {
    node* ref;

   public:
    typedef std::ptrdiff_t difference_type;
    typedef C value_type;
    typedef C* pointer;
    typedef C& reference;
    typedef std::bidirectional_iterator_tag iterator_category;

    template <typename V>
    iterator_t(const iterator_t<V>& other,
               typename std::enable_if<std::is_same<C, const V>::value>::type* =
                   nullptr)
        : ref(other.ref) {}
    iterator_t(node const& v) : ref(&v) {}

    C& operator*() const { return static_cast<node_v*>(ref)->value; }
    C* operator->() const { return &(static_cast<node_v*>(ref)->value); }

    iterator_t operator++(int) {
      iterator_t<C> t(*this);
      ref = ref->next;
      return t;
    }
    iterator_t operator++() {
      ref = ref->next;
      return *this;
    }
    iterator_t operator+(int b) {
      if (b < 0) return operator-(-b);
      node* t = ref;
      while (b-- && ref) t = t->next;
      return iterator_t(t);
    }
    iterator_t operator--(int) {
      iterator_t<C> t(*this);
      ref = ref->prev;
      return t;
    }
    iterator_t operator--() {
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
  list() : dummy(new node) {
    dummy->next = dummy;
    dummy->prev = dummy;
  }
  list(list const& other) : dummy(new node) { copyFrom(other); }
  list& operator=(list const& other) {
    clear();
    copyFrom(other);
  }

  ~list() {
    clear();
    delete dummy;
  }

  bool empty() const noexcept { return dummy->next != dummy; };
  void clear() noexcept {
    node* n = dummy->next;
    while (n != dummy) {
      node* t = n;
      n = n->next;
      delete t;
    }
  }

  void push_front(T const& val) noexcept {
    dummy->next = new node_v(val, dummy->next, dummy);
    dummy->next->next->prev = dummy->next;
  }
  void pop_front() noexcept {
    if (dummy->next != dummy) {
      node* t = dummy->next;
      dummy->next = dummy->next->next;
      dummy->next->prev = dummy;
      delete t;
    }
  }
  T& front() const {  // strong
    return static_cast<node_v*>(dummy->next)->value;
  }

  void push_back(T const& val) noexcept {
    dummy->prev = new node_v(val, dummy, dummy->prev);
    dummy->prev->prev->next = dummy->prev;
  }
  void pop_back() noexcept {
    if (dummy->prev != dummy) {
      node* t = dummy->prev;
      dummy->prev = dummy->prev->prev;
      dummy->prev->next = dummy;
      delete t;
    }
  }
  T& back() const {  // strong
    return static_cast<node_v*>(dummy->prev)->value;
  }

  typedef iterator_t<const T> const_iterator;
  typedef iterator_t<T> iterator;
  typedef std::reverse_iterator<const_iterator> reverse_const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;

  const_iterator begin() const noexcept {
    return const_iterator(*(dummy->next));
  }
  const_iterator end() const noexcept { return const_iterator(*(dummy->prev)); }
  iterator begin() noexcept { return iterator(*(dummy->next)); }
  iterator end() noexcept { return iterator(*(dummy->prev)); }
  reverse_const_iterator rbegin() const noexcept {
    return reverse_const_iterator(*(dummy->next));
  }
  reverse_const_iterator rend() const noexcept {
    return reverse_const_iterator(*(dummy->prev));
  }
  reverse_iterator rbegin() noexcept {
    return reverse_iterator(*(dummy->next));
  }
  reverse_iterator rend() noexcept { return reverse_iterator(*(dummy->prev)); }

  iterator insert(const_iterator pos, T const& v) {
    node* n = new node_v(v, pos.ref->next, pos.ref);
    n->next->prev = n;
    n->prev->next = n;
    return iterator(n);
  }

  iterator erase(const_iterator pos) {
    if (pos.ref != dummy) {
      pos.ref->next->prev = pos.ref->prev;
      pos.ref->prev->next = pos.ref->next;
      iterator r(pos.ref->prev);
      delete pos.ref;
      return r;
    }
    return iterator(pos.ref);
  }
  void splice(const_iterator pos, list& other, const_iterator first,
              const_iterator last) {
    pos.ref->next->prev = last.ref->prev;
    last.ref->prev->next = pos.ref->next;
    last.ref->prev = first.ref->prev;
    first.ref->prev->next = last.ref;
    pos.ref->next = first.ref;
    first.ref->prev = pos.ref;
  }

  template <typename V>
  friend void swap(list<V>&, list<V>&) noexcept;
};

template <typename T>
void swap(list<T>& a, list<T>& b) noexcept {
  swap(a.dummy, b.dummy);
}