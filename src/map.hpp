/**
* implement a container like std::map
*/
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
   class Key,
   class T,
   class Compare = std::less <Key>
   > class map {
  public:
   /**
  * the internal type of data.
  * it should have a default constructor, a copy constructor.
  * You can use sjtu::map as value_type by typedef.
    */
   typedef pair<const Key, T> value_type;
   // Red-Black Tree Node
   struct Node {
       value_type *data; // pointer to allow null for header
       Node *left, *right, *parent;
       bool color; // true for red, false for black

       Node(value_type *val, Node *l = nullptr, Node *r = nullptr, Node *p = nullptr, bool c = true)
           : data(val), left(l), right(r), parent(p), color(c) {}

       // Constructor for regular nodes
       Node(const value_type &val, Node *l = nullptr, Node *r = nullptr, Node *p = nullptr, bool c = true)
           : data(new value_type(val)), left(l), right(r), parent(p), color(c) {}

       // Constructor for header node
       Node() : data(nullptr), left(nullptr), right(nullptr), parent(nullptr), color(false) {}

       ~Node() {
           if (data) delete data;
       }
   };

   Node *root;
   Node *header; // dummy node for end() iterator
   size_t map_size;
   Compare comp;

   public:
   // Helper functions (implemented inline below)
   /**
  * see BidirectionalIterator at CppReference for help.
  *
  * if there is anything wrong throw invalid_iterator.
  *     like it = map.begin(); --it;
  *       or it = map.end(); ++end();
    */
   class const_iterator;
   class iterator {
      private:
       Node *node;
       const map *container;

      public:
       iterator() : node(nullptr), container(nullptr) {}

       iterator(Node *n, const map *c) : node(n), container(c) {}

       iterator(const iterator &other) : node(other.node), container(other.container) {}

       /**
    * TODO iter++
        */
       iterator operator++(int) {
           iterator temp = *this;
           ++(*this);
           return temp;
       }

       /**
    * TODO ++iter
        */
       iterator &operator++() {
           if (!node || !container) throw invalid_iterator();
           if (node == container->header) throw invalid_iterator();
           node = container->successor(node);
           if (!node) node = container->header;
           return *this;
       }

       /**
    * TODO iter--
        */
       iterator operator--(int) {
           iterator temp = *this;
           --(*this);
           return temp;
       }

       /**
    * TODO --iter
        */
       iterator &operator--() {
           if (!node || !container) throw invalid_iterator();
           if (node == container->header) {
               node = container->maximum(container->root);
               if (!node) throw invalid_iterator();
               return *this;
           }
           node = container->predecessor(node);
           if (!node) throw invalid_iterator();
           return *this;
       }

       /**
    * a operator to check whether two iterators are same (pointing to the same memory).
        */
       value_type &operator*() const {
           if (!node || node == container->header || !node->data) throw invalid_iterator();
           return *(node->data);
       }

       bool operator==(const iterator &rhs) const {
           return node == rhs.node && container == rhs.container;
       }

       bool operator==(const const_iterator &rhs) const {
           return node == rhs.node && container == rhs.container;
       }

       /**
    * some other operator for iterator.
        */
       bool operator!=(const iterator &rhs) const {
           return !(*this == rhs);
       }

       bool operator!=(const const_iterator &rhs) const {
           return !(*this == rhs);
       }

       /**
    * for the support of it->first.
    * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
        */
       value_type *operator->() const noexcept {
           // Don't throw in noexcept function
           return node->data;
       }

       friend class map;
       friend class const_iterator;
   };
   class const_iterator {
      private:
       const Node *node;
       const map *container;

      public:
       const_iterator() : node(nullptr), container(nullptr) {}

       const_iterator(const const_iterator &other) : node(other.node), container(other.container) {}

       const_iterator(const iterator &other) : node(other.node), container(other.container) {}

       const_iterator(const Node *n, const map *c) : node(n), container(c) {}

       const_iterator &operator++() {
           if (!node || !container) throw invalid_iterator();
           if (node == container->header) throw invalid_iterator();
           node = container->successor(const_cast<Node*>(node));
           if (!node) node = container->header;
           return *this;
       }

       const_iterator operator++(int) {
           const_iterator temp = *this;
           ++(*this);
           return temp;
       }

       const_iterator &operator--() {
           if (!node || !container) throw invalid_iterator();
           if (node == container->header) {
               node = container->maximum(container->root);
               if (!node) throw invalid_iterator();
               return *this;
           }
           node = container->predecessor(const_cast<Node*>(node));
           if (!node) throw invalid_iterator();
           return *this;
       }

       const_iterator operator--(int) {
           const_iterator temp = *this;
           --(*this);
           return temp;
       }

       const value_type &operator*() const {
           if (!node || node == container->header || !node->data) throw invalid_iterator();
           return *(node->data);
       }

       const value_type *operator->() const noexcept {
           // Don't throw in noexcept function
           return node->data;
       }

       bool operator==(const iterator &rhs) const {
           return node == rhs.node && container == rhs.container;
       }

       bool operator==(const const_iterator &rhs) const {
           return node == rhs.node && container == rhs.container;
       }

       bool operator!=(const iterator &rhs) const {
           return !(*this == rhs);
       }

       bool operator!=(const const_iterator &rhs) const {
           return !(*this == rhs);
       }

       friend class map;
   };

   /**
  * TODO two constructors
    */
   map() : root(nullptr), map_size(0), comp() {
       // Create a dummy header node without constructing value_type
       header = new Node();
       header->left = header->right = header->parent = nullptr;
   }

   map(const map &other) : root(nullptr), map_size(0), comp(other.comp) {
       header = new Node();
       header->left = header->right = header->parent = nullptr;
       root = copyTree(other.root, nullptr);
       map_size = other.map_size;
   }

   /**
  * TODO assignment operator
    */
   map &operator=(const map &other) {
       if (this == &other) return *this;
       clear();
       root = copyTree(other.root, nullptr);
       map_size = other.map_size;
       comp = other.comp;
       return *this;
   }

   /**
  * TODO Destructors
    */
   ~map() {
       clearTree(root);
       delete header;
   }

   /**
  * TODO
  * access specified element with bounds checking
  * Returns a reference to the mapped value of the element with key equivalent to key.
  * If no such element exists, an exception of type `index_out_of_bound'
    */
   T &at(const Key &key) {
       Node *node = findNode(key);
       if (!node || !node->data) throw index_out_of_bound();
       return node->data->second;
   }

   const T &at(const Key &key) const {
       Node *node = findNode(key);
       if (!node || !node->data) throw index_out_of_bound();
       return node->data->second;
   }

   /**
  * TODO
  * access specified element
  * Returns a reference to the value that is mapped to a key equivalent to key,
  *   performing an insertion if such key does not already exist.
    */
   T &operator[](const Key &key) {
       Node *node = findNode(key);
       if (node && node->data) return node->data->second;

       // Insert new element with default constructed value
       T default_value{};
       value_type new_pair(key, default_value);
       auto result = insert(value_type(key, T()));
       return result.first->second;
   }

   /**
  * behave like at() throw index_out_of_bound if such key does not exist.
    */
   const T &operator[](const Key &key) const {
       return at(key);
   }

   /**
  * return a iterator to the beginning
    */
   iterator begin() {
       Node *min_node = minimum(root);
       if (!min_node) return iterator(header, this);
       return iterator(min_node, this);
   }

   const_iterator cbegin() const {
       Node *min_node = minimum(root);
       if (!min_node) return const_iterator(header, this);
       return const_iterator(min_node, this);
   }

   /**
  * return a iterator to the end
  * in fact, it returns past-the-end.
    */
   iterator end() {
       return iterator(header, this);
   }

   const_iterator cend() const {
       return const_iterator(header, this);
   }

   /**
  * checks whether the container is empty
  * return true if empty, otherwise false.
    */
   bool empty() const {
       return map_size == 0;
   }

   /**
  * returns the number of elements.
    */
   size_t size() const {
       return map_size;
   }

   /**
  * clears the contents
    */
   void clear() {
       clearTree(root);
       root = nullptr;
       map_size = 0;
   }

   /**
  * insert an element.
  * return a pair, the first of the pair is
  *   the iterator to the new element (or the element that prevented the insertion),
  *   the second one is true if insert successfully, or false.
    */
   pair<iterator, bool> insert(const value_type &value) {
       Node *current = root;
       Node *parent = nullptr;

       while (current) {
           parent = current;
           if (comp(value.first, current->data->first)) {
               current = current->left;
           } else if (comp(current->data->first, value.first)) {
               current = current->right;
           } else {
               // Key already exists
               return pair<iterator, bool>(iterator(current, this), false);
           }
       }

       Node *new_node = new Node(value, nullptr, nullptr, parent, true);
       if (!parent) {
           root = new_node;
       } else if (comp(value.first, parent->data->first)) {
           parent->left = new_node;
       } else {
           parent->right = new_node;
       }

       fixInsert(new_node);
       map_size++;
       return pair<iterator, bool>(iterator(new_node, this), true);
   }

   /**
  * erase the element at pos.
  *
  * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
    */
   void erase(iterator pos) {
       if (pos == end() || pos.container != this) throw invalid_iterator();

       Node *z = pos.node;
       Node *y = z;
       Node *x = nullptr;
       bool y_original_color = y->color;

       if (!z->left) {
           x = z->right;
           transplant(z, z->right);
       } else if (!z->right) {
           x = z->left;
           transplant(z, z->left);
       } else {
           y = minimum(z->right);
           y_original_color = y->color;
           x = y->right;

           if (y->parent == z) {
               if (x) x->parent = y;
           } else {
               transplant(y, y->right);
               y->right = z->right;
               if (y->right) y->right->parent = y;
           }

           transplant(z, y);
           y->left = z->left;
           if (y->left) y->left->parent = y;
           y->color = z->color;
       }

       delete z;
       map_size--;

       if (!y_original_color && x) {
           fixDelete(x);
       }
   }

   /**
  * Returns the number of elements with key
  *   that compares equivalent to the specified argument,
  *   which is either 1 or 0
  *     since this container does not allow duplicates.
  * The default method of check the equivalence is !(a < b || b > a)
    */
   size_t count(const Key &key) const {
       return findNode(key) ? 1 : 0;
   }

   /**
  * Finds an element with key equivalent to key.
  * key value of the element to search for.
  * Iterator to an element with key equivalent to key.
  *   If no such element is found, past-the-end (see end()) iterator is returned.
    */
   iterator find(const Key &key) {
       Node *node = findNode(key);
       if (!node) return iterator(header, this);
       return iterator(node, this);
   }

   const_iterator find(const Key &key) const {
       Node *node = findNode(key);
       if (!node) return const_iterator(header, this);
       return const_iterator(node, this);
   }

  private:
   // Helper function implementations
   void rotateLeft(Node *x) {
       Node *y = x->right;
       x->right = y->left;
       if (y->left) y->left->parent = x;
       y->parent = x->parent;

       if (!x->parent) root = y;
       else if (x == x->parent->left) x->parent->left = y;
       else x->parent->right = y;

       y->left = x;
       x->parent = y;
   }

   void rotateRight(Node *x) {
       Node *y = x->left;
       x->left = y->right;
       if (y->right) y->right->parent = x;
       y->parent = x->parent;

       if (!x->parent) root = y;
       else if (x == x->parent->right) x->parent->right = y;
       else x->parent->left = y;

       y->right = x;
       x->parent = y;
   }

   void fixInsert(Node *z) {
       while (z->parent && z->parent->color) {
           if (z->parent == z->parent->parent->left) {
               Node *y = z->parent->parent->right;
               if (y && y->color) {
                   z->parent->color = false;
                   y->color = false;
                   z->parent->parent->color = true;
                   z = z->parent->parent;
               } else {
                   if (z == z->parent->right) {
                       z = z->parent;
                       rotateLeft(z);
                   }
                   z->parent->color = false;
                   z->parent->parent->color = true;
                   rotateRight(z->parent->parent);
               }
           } else {
               Node *y = z->parent->parent->left;
               if (y && y->color) {
                   z->parent->color = false;
                   y->color = false;
                   z->parent->parent->color = true;
                   z = z->parent->parent;
               } else {
                   if (z == z->parent->left) {
                       z = z->parent;
                       rotateRight(z);
                   }
                   z->parent->color = false;
                   z->parent->parent->color = true;
                   rotateLeft(z->parent->parent);
               }
           }
       }
       root->color = false;
   }

   void transplant(Node *u, Node *v) {
       if (!u->parent) root = v;
       else if (u == u->parent->left) u->parent->left = v;
       else u->parent->right = v;

       if (v) v->parent = u->parent;
   }

   Node* minimum(Node *x) const {
       if (!x) return nullptr;
       while (x->left) x = x->left;
       return x;
   }

   Node* maximum(Node *x) const {
       if (!x) return nullptr;
       while (x->right) x = x->right;
       return x;
   }

   Node* successor(Node *x) const {
       if (!x) return nullptr;
       if (x->right) return minimum(x->right);

       Node *y = x->parent;
       while (y && x == y->right) {
           x = y;
           y = y->parent;
       }
       return y;
   }

   Node* predecessor(Node *x) const {
       if (!x) return nullptr;
       if (x->left) return maximum(x->left);

       Node *y = x->parent;
       while (y && x == y->left) {
           x = y;
           y = y->parent;
       }
       return y;
   }

   void clearTree(Node *node) {
       if (!node || !node->data) return; // header node has null data
       clearTree(node->left);
       clearTree(node->right);
       delete node;
   }

   Node* copyTree(Node *node, Node *parent) {
       if (!node || !node->data) return nullptr;

       Node *new_node = new Node(*(node->data), nullptr, nullptr, parent, node->color);
       new_node->left = copyTree(node->left, new_node);
       new_node->right = copyTree(node->right, new_node);
       return new_node;
   }

   Node* findNode(const Key &key) const {
       Node *current = root;
       while (current) {
           if (comp(key, current->data->first)) {
               current = current->left;
           } else if (comp(current->data->first, key)) {
               current = current->right;
           } else {
               return current;
           }
       }
       return nullptr;
   }

   void fixDelete(Node *x) {
       Node *w;
       while (x != root && (!x || x->color == false)) {
           if (x == nullptr) {
               // x is null (black), we're done
               break;
           }

           if (x == x->parent->left) {
               Node *w = x->parent->right;
               if (w && w->color) {
                   w->color = false;
                   x->parent->color = true;
                   rotateLeft(x->parent);
                   w = x->parent->right;
               }
               if (!w || ((!w->left || !w->left->color) && (!w->right || !w->right->color))) {
                   if (w) w->color = true;
                   x = x->parent;
               } else {
                   if (!w->right || !w->right->color) {
                       if (w->left) w->left->color = false;
                       w->color = true;
                       rotateRight(w);
                       w = x->parent->right;
                   }
                   w->color = x->parent->color;
                   x->parent->color = false;
                   if (w->right) w->right->color = false;
                   rotateLeft(x->parent);
                   x = root;
               }
           } else {
               Node *w = x->parent->left;
               if (w && w->color) {
                   w->color = false;
                   x->parent->color = true;
                   rotateRight(x->parent);
                   w = x->parent->left;
               }
               if (!w || ((!w->right || !w->right->color) && (!w->left || !w->left->color))) {
                   if (w) w->color = true;
                   x = x->parent;
               } else {
                   if (!w->left || !w->left->color) {
                       if (w->right) w->right->color = false;
                       w->color = true;
                       rotateLeft(w);
                       w = x->parent->left;
                   }
                   w->color = x->parent->color;
                   x->parent->color = false;
                   if (w->left) w->left->color = false;
                   rotateRight(x->parent);
                   x = root;
               }
           }
       }
       if (x) x->color = false;
   }
};

}

#endif