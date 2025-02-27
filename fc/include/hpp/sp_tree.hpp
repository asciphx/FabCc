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
  private:
    struct Node {
      K key;
      V value;
      Node* left;
      Node* right;
      Node* parent;
      Node(const K& k, const V& v): key(k), value(v), left(nullptr), right(nullptr), parent(nullptr) {}
      Node(const K& k, V&& v): key(k), value(std::move(v)), left(nullptr), right(nullptr), parent(nullptr) {}
    };
    Node* root;
    inline void rotateRight(Node* x) {
      Node* y = x->left;
      x->left = y->right;
      if (y->right) y->right->parent = x;
      y->parent = x->parent;
      if (!x->parent) root = y;
      else if (x == x->parent->left) x->parent->left = y;
      else x->parent->right = y;
      y->right = x;
      x->parent = y;
    }
    inline void rotateLeft(Node* x) {
      Node* y = x->right;
      x->right = y->left;
      if (y->left) y->left->parent = x;
      y->parent = x->parent;
      if (!x->parent) root = y;
      else if (x == x->parent->left) x->parent->left = y;
      else x->parent->right = y;
      y->left = x;
      x->parent = y;
    }
    void splay(Node* x) {
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
    _FORCE_INLINE Node* minimum(Node* x) {
      while (x->left) x = x->left;
      return x;
    }
    void destroy(Node* node) {
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
      Node* z = new Node(key, value);
      Node* x = root;
      Node* y = nullptr;
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
      Node* x = root;
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
      Node* x = root;
      while (x) {
        if (key < x->key) x = x->left;
        else if (key > x->key) x = x->right;
        else break;
      }
      if (!x) return;
      splay(x);
      if (!x->left) {
        root = x->right;
        if (root) root->parent = nullptr;
      } else {
        Node* new_root = minimum(x->right);
        if (new_root) {
          splay(new_root);
          new_root->left = x->left;
          x->left->parent = new_root;
        } else {
          root = x->left;
          root->parent = nullptr;
        }
      }
      delete x;
    }
    //Similar to RBTree's operator[]
    V& operator[](const K& key) {
      Node* x = root;
      while (x) {
        if (key < x->key) x = x->left;
        else if (x->key < key) x = x->right;
        else {
          splay(x); //Splay finds the node to the root
          return x->value;
        }
      }
      //Not found, insert new node
      Node* z = new Node(key, V());
      Node* y = nullptr;
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
      Node* curr;
      iterator(Node* ptr = nullptr): curr(ptr) {}
      bool operator!=(const iterator& other) const { return curr != other.curr; }
      bool operator==(const iterator& other) const { return curr == other.curr; }
      std::pair<const A, B>* operator->() const { return reinterpret_cast<std::pair<const A, B>*>(&curr->key); }
      iterator& operator++() {
        if (curr->right) {
          curr = curr->right;
          while (curr->left) curr = curr->left;
        } else {
          Node* y = curr->parent;
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
      Node* x = root;
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