
#ifndef RB_TREE_HPP
#define RB_TREE_HPP
#include "tp/c++.h"
#include <functional>
template<typename K, typename V>
struct Node {
  K key; V value;
  Node* left;
  Node* right;
  Node* parent;
  bool b;
  Node(const K& k, const V& v): key(k), value(v), b(false),
    left(NULL), right(NULL), parent(NULL) {}
};

template<typename K, typename V>
class RBTree {
  Node<K, V>* root;
  Node<K, V>* nil;
  inline void rotateLeft(Node<K, V>* x) {
    Node<K, V>* y = x->right;
    x->right = y->left;
    if (y->left != nil) y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == nil) root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
  }
  inline void rotateRight(Node<K, V>* x) {
    Node<K, V>* y = x->left;
    x->left = y->right;
    if (y->right != nil) y->right->parent = x;
    y->parent = x->parent;
    if (x->parent == nil) root = y;
    else if (x == x->parent->right) x->parent->right = y;
    else x->parent->left = y;
    y->right = x;
    x->parent = y;
  }
  void fixInsert(Node<K, V>* z) {
    while (z != root && z->parent->b == false) {
      if (z->parent == z->parent->parent->left) {
        Node<K, V>* y = z->parent->parent->right;
        if (y->b == false) {
          z->parent->b = true;
          y->b = true;
          z->parent->parent->b = false;
          z = z->parent->parent;
        } else {
          if (z == z->parent->right) {
            z = z->parent;
            rotateLeft(z);
          }
          z->parent->b = true;
          z->parent->parent->b = false;
          rotateRight(z->parent->parent);
        }
      } else {
        Node<K, V>* y = z->parent->parent->left;
        if (y->b == false) {
          z->parent->b = true;
          y->b = true;
          z->parent->parent->b = false;
          z = z->parent->parent;
        } else {
          if (z == z->parent->left) {
            z = z->parent;
            rotateRight(z);
          }
          z->parent->b = true;
          z->parent->parent->b = false;
          rotateLeft(z->parent->parent);
        }
      }
    }
    root->b = true;
  }
  inline void transplant(Node<K, V>* u, Node<K, V>* v) {
    if (u->parent == nil) root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent;
  }
  _FORCE_INLINE Node<K, V>* minimum(Node<K, V>* x) const {
    while (x->left != nil) x = x->left;
    return x;
  }
  void fixDelete(Node<K, V>* x) {
    while (x != root && x->b == true) {
      if (x == x->parent->left) {
        Node<K, V>* w = x->parent->right;
        if (w->b == false) {
          w->b = true;
          x->parent->b = false;
          rotateLeft(x->parent);
          w = x->parent->right;
        }
        if (w->left->b == true && w->right->b == true) {
          w->b = false;
          x = x->parent;
        } else {
          if (w->right->b == true) {
            w->left->b = true;
            w->b = false;
            rotateRight(w);
            w = x->parent->right;
          }
          w->b = x->parent->b;
          x->parent->b = true;
          w->right->b = true;
          rotateLeft(x->parent);
          x = root;
        }
      } else {
        Node<K, V>* w = x->parent->left;
        if (w->b == false) {
          w->b = true;
          x->parent->b = false;
          rotateRight(x->parent);
          w = x->parent->left;
        }
        if (w->right->b == true && w->left->b == true) {
          w->b = false;
          x = x->parent;
        } else {
          if (w->left->b == true) {
            w->right->b = true;
            w->b = false;
            rotateLeft(w);
            w = x->parent->left;
          }
          w->b = x->parent->b;
          x->parent->b = true;
          w->left->b = true;
          rotateRight(x->parent);
          x = root;
        }
      }
    }
    x->b = true;
  }
  void deleteNode(Node<K, V>* z) {
    Node<K, V>* y = z;
    bool yColor = y->b;
    Node<K, V>* x;
    if (z->left == nil) {
      x = z->right;
      transplant(z, z->right);
    } else if (z->right == nil) {
      x = z->left;
      transplant(z, z->left);
    } else {
      y = minimum(z->right);
      yColor = y->b;
      x = y->right;
      if (y->parent == z) {
        x->parent = y;
      } else {
        transplant(y, y->right);
        y->right = z->right;
        y->right->parent = y;
      }
      transplant(z, y);
      y->left = z->left;
      y->left->parent = y;
      y->b = z->b;
    }
    delete z;
    if (yColor == true) fixDelete(x);
  }
  void destroy(Node<K, V>* node) {
    if (node != nil) {
      destroy(node->left);
      destroy(node->right);
      delete node;
    }
  }

public:
  RBTree() {
    nil = new Node<K, V>(K(), V());
    nil->b = true;
    nil->left = nil->right = nil->parent = nil;
    root = nil;
  }
  ~RBTree() {
    destroy(root);
    delete nil;
  }
  void insert(const K& key, const V& value) {
    Node<K, V>* z = new Node<K, V>(key, value);
    Node<K, V>* y = nil;
    Node<K, V>* x = root;
    while (x != nil) {
      y = x;
      if (z->key < x->key) x = x->left;
      else x = x->right;
    }
    z->parent = y;
    if (y == nil) root = z;
    else if (z->key < y->key) y->left = z;
    else y->right = z;
    z->left = nil;
    z->right = nil;
    fixInsert(z);
  }
  _FORCE_INLINE void remove(const K& key) {
    Node<K, V>* z = find(key);
    if (z != nullptr) deleteNode(z);
  }
  inline Node<K, V>* find(const K& key) const {
    Node<K, V>* x = root;
    while (x != nil) {
      if (key < x->key) x = x->left;
      else if (x->key < key) x = x->right;
      else return x;
    }
    return nullptr;
  }
  _FORCE_INLINE bool empty() const { return root == nil; }
  _FORCE_INLINE void clear() {
    destroy(root);
    root = nil;
  }
  // New method to find timer by ID (specialized for Timer class)
  inline Node<K, V>* find_by_id(uint64_t id) const {
    Node<K, V>* x = root;
    while (x != nil) {
      if (id < x->key.id) x = x->left;
      else if (x->key.id < id) x = x->right;
      else return x;
    }
    return nullptr;
  }
  // Add to RBTree public section:
  _FORCE_INLINE Node<K, V>* get_minimum() const {
    return minimum(root);
  }
};
#endif