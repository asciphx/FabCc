#ifndef RB_TREE_HPP
#define RB_TREE_HPP
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
//Red-Black Tree
namespace fc {
  template<typename K, typename V>
  struct RBTree {
    struct Nod {
      K key; V value;
      Nod* left;
      Nod* right;
      Nod* parent;
      alignas(_PTR_LEN) bool b;
      Nod(const K& k, const V& v) noexcept: key(k), value(v), b(false),
        left(NULL), right(NULL), parent(NULL) {}
      Nod(const K& k, V&& v) noexcept: key(k), value(std::move(v)), b(false),
        left(NULL), right(NULL), parent(NULL) {}
    };
  private:
    Nod* root;
    Nod* nil;
    inline void rotateLeft(Nod* x) noexcept {
      Nod* y = x->right;
      x->right = y->left;
      if (y->left != nil) y->left->parent = x;
      y->parent = x->parent;
      if (x->parent == nil) root = y;
      else if (x == x->parent->left) x->parent->left = y;
      else x->parent->right = y;
      y->left = x;
      x->parent = y;
    }
    inline void rotateRight(Nod* x) noexcept {
      Nod* y = x->left;
      x->left = y->right;
      if (y->right != nil) y->right->parent = x;
      y->parent = x->parent;
      if (x->parent == nil) root = y;
      else if (x == x->parent->right) x->parent->right = y;
      else x->parent->left = y;
      y->right = x;
      x->parent = y;
    }
    void fixInsert(Nod* z) noexcept {
      while (z != root && z->parent->b == false) {
        if (z->parent == z->parent->parent->left) {
          Nod* y = z->parent->parent->right;
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
          Nod* y = z->parent->parent->left;
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
    inline void transplant(Nod* u, Nod* v) noexcept {
      if (u->parent == nil) root = v;
      else if (u == u->parent->left) u->parent->left = v;
      else u->parent->right = v;
      v->parent = u->parent;
    }
    void fixDelete(Nod* x) noexcept {
      while (x != root && x->b == true) {
        if (x == x->parent->left) {
          Nod* w = x->parent->right;
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
          Nod* w = x->parent->left;
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
    void deleteNode(Nod* z) noexcept {
      Nod* y = z;
      bool yColor = y->b;
      Nod* x;
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
    void destroy(Nod* node) noexcept {
      if (node != nil) {
        destroy(node->left);
        destroy(node->right);
        delete node;
      }
    }
  public:
    V& operator[](const K& key) {
      Nod* x = root;
      while (x != nil) {
        if (key < x->key) x = x->left;
        else if (x->key < key) x = x->right;
        else return x->value;
      }
      Nod* z = new Nod(key, V());
      Nod* y = nil; x = root;
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
      return z->value;
    };
    struct iterator {
      const Nod* nil;
      mutable Nod* curr;
    public:
      iterator(Nod* ptr = nullptr, Nod* n = nullptr) noexcept: nil(n), curr(ptr) {}
      _FORCE_INLINE bool operator!=(const iterator& other) const noexcept { return curr != other.curr; }
      _FORCE_INLINE bool operator==(const iterator& other) const noexcept { return !(*this != other); }
      _FORCE_INLINE std::pair<const K, V>* operator->() const noexcept { return reinterpret_cast<std::pair<const K, V>*>(&curr->key); }
      _FORCE_INLINE std::pair<const K, V>& operator*() const noexcept { return reinterpret_cast<std::pair<const K, V>&>(curr->key); }
      iterator& operator++() noexcept {
        if (curr->right != nil) {
          Nod* x = curr->right;
          while (x->left != nil) x = x->left;
          curr = x;
        } else {
          Nod* y = curr->parent;
          while (y != nil && curr == y->right) {
            curr = y;
            y = y->parent;
          }
          curr = y;
        }
        return *this;
      };
      iterator operator++(int) noexcept { iterator tmp = *this; ++(*this); return tmp; }
    };
    _FORCE_INLINE iterator begin() noexcept {
      Nod* x = root; while (x->left != nil) x = x->left; return iterator(x, nil);
    };
    _FORCE_INLINE iterator end() const noexcept {
      return iterator(this->nil, this->nil);
    }
    RBTree() {
      nil = new Nod(K(), V());
      nil->b = true;
      nil->left = nil->right = nil->parent = nil;
      root = nil;
    }
    ~RBTree() {
      destroy(root);
      delete nil;
    }
    void insert(const K& key, const V& value) noexcept {
      Nod* z = new Nod(key, value);
      Nod* y = nil;
      Nod* x = root;
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
    _FORCE_INLINE void remove(const K& key) noexcept {
      Nod* z = find(key);
      if (z != nullptr) deleteNode(z);
    }
    inline Nod* find(const K& key) const noexcept {
      Nod* x = root;
      while (x != nil) {
        if (key < x->key) x = x->left;
        else if (x->key < key) x = x->right;
        else return x;
      }
      return nullptr;
    }
    _FORCE_INLINE bool empty() const noexcept { return root == nil; }
    _FORCE_INLINE void clear() noexcept {
      destroy(root);
      root = nil;
    }
    // Add to RBTree public section:
    _FORCE_INLINE Nod* minimum(Nod* x) const noexcept {
      while (x->left != nil) x = x->left;
      return x;
    }
  };
}
#endif