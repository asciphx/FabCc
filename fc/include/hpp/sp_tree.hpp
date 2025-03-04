#ifndef SP_TREE_HPP
#define SP_TREE_HPP
/*
 * This software is licensed under the AGPL-3.0 License.
 *
 * Copyright (C) 2025 Asciphx
 *
 * Permissions of this strongest copyleft license are conditioned on making available
 * complete source code of licensed works and modifications, which include larger works
 * using a licensed work, under the same license. Copyright and license notices must be
 * preserved. Contributors provide an express grant of patent rights. When a modified
 * version is used to provide a service over a network, the complete source code of
 * the modified version must be made available.
 */
#include "tp/c++.h"
#include <functional>
//Splay Tree
namespace fc {
  template<typename K, typename V>
  class SPTree {
    struct Nod {
      K key;
      V value;
      Nod* left;
      Nod* right;
      Nod* parent;
      Nod(const K& k, const V& v): key(k), value(v), left(nullptr), right(nullptr), parent(nullptr) {}
      Nod(const K& k, V&& v): key(k), value(std::move(v)), left(nullptr), right(nullptr), parent(nullptr) {}
    };
    Nod* root;
    inline void rotateRight(Nod* x) {
      Nod* y = x->left;
      x->left = y->right;
      if (y->right) y->right->parent = x;
      y->parent = x->parent;
      if (!x->parent) root = y;
      else if (x == x->parent->left) x->parent->left = y;
      else x->parent->right = y;
      y->right = x;
      x->parent = y;
    }
    inline void rotateLeft(Nod* x) {
      Nod* y = x->right;
      x->right = y->left;
      if (y->left) y->left->parent = x;
      y->parent = x->parent;
      if (!x->parent) root = y;
      else if (x == x->parent->left) x->parent->left = y;
      else x->parent->right = y;
      y->left = x;
      x->parent = y;
    }
    void splay(Nod* x) {
      while (x->parent) {
        if (!x->parent->parent) {
          if (x == x->parent->left) rotateRight(x->parent);
          else rotateLeft(x->parent);
        } else if (x == x->parent->left && x->parent == x->parent->parent->left) {
          rotateRight(x->parent->parent);
          rotateRight(x->parent);
        } else if (x == x->parent->right && x->parent == x->parent->parent->right) {
          rotateLeft(x->parent->parent);
          rotateLeft(x->parent);
        } else if (x == x->parent->right && x->parent == x->parent->parent->left) {
          rotateLeft(x->parent);
          rotateRight(x->parent);
        } else {
          rotateRight(x->parent);
          rotateLeft(x->parent);
        }
      }
    }
    void destroy(Nod* node) {
      if (node) {
        destroy(node->left);
        destroy(node->right);
        delete node;
      }
    }
  public:
    SPTree(): root(nullptr) {}
    ~SPTree() {
      destroy(root);
    }
    void insert(const K& key, const V& value) {
      Nod* z = new Nod(key, value);
      Nod* x = root;
      Nod* y = nullptr;
      while (x) {
        y = x;
        if (key < x->key) x = x->left;
        else if (key > x->key) x = x->right;
        else {
          x->value = value;
          splay(x);
          delete z;
          return;
        }
      }
      z->parent = y;
      if (!y) root = z;
      else if (key < y->key) y->left = z;
      else y->right = z;
      splay(z);
    }
    inline V* find(const K& key) {
      Nod* x = root;
      while (x) {
        if (key < x->key) x = x->left;
        else if (key > x->key) x = x->right;
        else {
          splay(x);
          return &x->value;
        }
      }
      return nullptr;
    }
    void remove(const K& key) {
      Nod* x = root;
      while (x) {
        if (key < x->key) x = x->left;
        else if (key > x->key) x = x->right;
        else break;
      }
      if (!x) return;
      splay(x);
      Nod* l = x->left;
      Nod* r = x->right;
      if (l) l->parent = nullptr;
      if (r) r->parent = nullptr;
      delete x;
      if (!l) {
        root = r;
      } else {
        while (l->right) l = l->right;
        splay(l);
        l->right = r;
        if (r) r->parent = l;
        root = l;
      }
    }
    //Similar to RBTree's operator[]
    V& operator[](const K& key) {
      Nod* x = root;
      while (x) {
        if (key < x->key) x = x->left;
        else if (x->key < key) x = x->right;
        else {
          splay(x); //Splay finds the node to the root
          return x->value;
        }
      }
      //Not found, insert new node
      Nod* z = new Nod(key, V());
      Nod* y = nullptr;
      x = root;
      while (x) {
        y = x;
        if (key < x->key) x = x->left;
        else x = x->right;
      }
      z->parent = y;
      if (!y) root = z;
      else if (key < y->key) y->left = z;
      else y->right = z;
      splay(z); //Splay new node to root
      return z->value;
    }
    // Iterator implementation
    template<typename A, typename B>
    struct iterator {
      Nod* curr;
      iterator(Nod* ptr = nullptr): curr(ptr) {}
      bool operator!=(const iterator& other) const { return curr != other.curr; }
      bool operator==(const iterator& other) const { return curr == other.curr; }
      std::pair<const A, B>* operator->() const { return reinterpret_cast<std::pair<const A, B>*>(&curr->key); }
      iterator& operator++() {
        if (curr->right) {
          curr = curr->right;
          while (curr->left) curr = curr->left;
        } else {
          Nod* y = curr->parent;
          while (y && curr == y->right) {
            curr = y;
            y = y->parent;
          }
          curr = y;
        }
        return *this;
      }
    };
    _FORCE_INLINE iterator<K, V> begin() {
      Nod* x = root;
      if (!x) return iterator<K, V>(nullptr);
      while (x->left) x = x->left;
      return iterator<K, V>(x);
    }
    _FORCE_INLINE iterator<K, V> end() {
      return iterator<K, V>(nullptr);
    }
    _FORCE_INLINE bool empty() const { return root == nullptr; }
    _FORCE_INLINE void clear() { destroy(root); root = nullptr; }
  };
}
#endif