#ifndef HashMap_HPP
#define HashMap_HPP
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
#include "hpp/string_view.hpp"
#include <stdexcept>
#include <immintrin.h>
#include <functional>
namespace fc {
  // （2³² × (√5 - 1) / 2）
  template<typename K>
  _FORCE_INLINE static size_t fc_hash(const K& key, size_t mod) noexcept {
    return (static_cast<size_t>(key) * 0X9e3779b1U) % mod;
  }
  // Optimized string hash using SIMD where possible
  template<>
  _FORCE_INLINE size_t fc_hash<std::string>(const std::string& key, size_t mod) noexcept {
    const unsigned char* p = reinterpret_cast<const unsigned char*>(key.c_str());
    size_t n = key.length(), r = n - 1;
    if (n >= 16) {
      __m128i hash_vec = _mm_set1_epi32(static_cast<int>(r)); const unsigned char* end = p + (n & ~15);
      while (p < end) {
        __m128i chunk = _mm_and_si128(_mm_loadu_si128(reinterpret_cast<const __m128i*>(p)), mask_case);
        hash_vec = _mm_add_epi32(_mm_add_epi32(_mm_mul_epu32(hash_vec, mul_factor), _mm_unpacklo_epi32(chunk, zero)),
          _mm_add_epi32(_mm_mul_epu32(_mm_srli_si128(hash_vec, 8), mul_factor), _mm_unpackhi_epi32(chunk, zero)));
        p += 16;
      }
      r = _mm_cvtsi128_si32(hash_vec); n &= 15;
    }
    if (n >= 8) {
      r = (r * 5 + ((p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24) |
        (static_cast<size_t>(p[4]) << 32) | (static_cast<size_t>(p[5]) << 40) |
        (static_cast<size_t>(p[6]) << 48) | (static_cast<size_t>(p[7]) << 56)) & ~0x2020202020202020));
      p += 8; n -= 8;
    }
    while (n >= 4) {
      uint32_t chunk = (p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24)) & _um; r = r * 5 + chunk; p += 4; n -= 4;
    }
    while (n--) r = r * 5 + (*p++ & 0xDF); return (r - key.length()) % mod;
  }
  template<>
  _FORCE_INLINE size_t fc_hash<std::string_view>(const std::string_view& key, size_t mod) noexcept {
    size_t n = key.size(); size_t r = 0;
    unsigned char const* p = reinterpret_cast<unsigned char const*>(key.data());
    if (n >= 16) {
      __m128i hash_vec = _mm_set1_epi32(static_cast<int>(r)); const unsigned char* end = p + (n & ~15);
      while (p < end) {
        __m128i chunk = _mm_and_si128(_mm_loadu_si128(reinterpret_cast<const __m128i*>(p)), mask_case);
        hash_vec = _mm_add_epi32(_mm_add_epi32(_mm_mul_epu32(hash_vec, mul_factor), _mm_unpacklo_epi32(chunk, zero)),
          _mm_add_epi32(_mm_mul_epu32(_mm_srli_si128(hash_vec, 8), mul_factor), _mm_unpackhi_epi32(chunk, zero)));
        p += 16;
      }
      r = _mm_cvtsi128_si32(hash_vec); n &= 15;
    }
    while (n >= 8) {
      r = (r * 5 + ((p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24) |
        (static_cast<size_t>(p[4]) << 32) | (static_cast<size_t>(p[5]) << 40) |
        (static_cast<size_t>(p[6]) << 48) | (static_cast<size_t>(p[7]) << 56)) & ~0x2020202020202020));
      p += 8; n -= 8;
    }
    while (n >= 4) {
      uint32_t chunk = (p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24)) & _um;
      r = r * 5 + chunk; p += 4; n -= 4;
    }
    while (n--) r = r * 5 + (*p++ & 0xDF); return r % mod;
  }
  template<typename Z>
  _FORCE_INLINE const uint64_t fc_HashResize(size_t _) { return static_cast<uint64_t>(_ << 1); };
  template<>
  _FORCE_INLINE const uint64_t fc_HashResize<uint64_t>(size_t _) { return static_cast<uint64_t>(_ > 0x100000 ? _ + (_ >> 1) : _ << 1); };
  template<>
  _FORCE_INLINE const uint64_t fc_HashResize<uint32_t>(size_t _) { return static_cast<uint64_t>(_ > 0x10000 ? _ + (_ >> 1) : _ << 1); };
  template<>
  _FORCE_INLINE const uint64_t fc_HashResize<uint16_t>(size_t _) { return static_cast<uint64_t>(_ > 0x1000 ? _ + (_ >> 1) : _ << 1); };
  template<typename Z>
  static constexpr const uint64_t fc_HashMax(size_t _) { return static_cast<uint64_t>(static_cast<Z>(-1)) * _; };
  template<>
  constexpr const uint64_t fc_HashMax<uint64_t>(size_t _) { return static_cast<uint64_t>(-1); };
  // Query-friendly hash table similar to std::unordered_map, T = superPointers, E = std::equal_to<K>
  template<typename K, typename V, typename T = uint16_t, typename E = std::equal_to<K>, char LOAD_FACTOR_THRESHOLD = 75>
  class HashMap {
    static int const constexpr size_dummy = sizeof(K) >= sizeof(V) * 2 ? sizeof(V) : sizeof(K) > sizeof(V) ? sizeof(K) : sizeof(V);
    struct Nod {
      K key; alignas(size_dummy) V value; alignas(size_dummy) bool occupied;
      Nod() noexcept: key(), value(), occupied(false) {}
      Nod(const K& k, const V& v) noexcept: key(k), value(v), occupied(true) {}
      Nod(const K& k, V&& v) noexcept: key(k), value(std::move(v)), occupied(true) {}
    };
    E equal; Nod* table; T* superPointers; size_t totalSize; size_t numEntries; size_t numSubarrays;
    static const constexpr size_t SUBARRAY_SIZE = sizeof(K) - sizeof(T) > 16 ? 0x20 : sizeof(V) < 8 ? 16 : sizeof(K) < 16 ? 0x20 : 16;
    static V dummy;
    static const constexpr uint64_t MaxSize = fc_HashMax<T>(SUBARRAY_SIZE);
    // Optimization: Reduced exception path overhead by using noexcept and manual cleanup
    bool resize() noexcept {
      size_t newTotalSize = fc_HashResize<T>(totalSize);
      if (newTotalSize > MaxSize || newTotalSize <= totalSize) {
        if (totalSize == MaxSize) return false; newTotalSize = MaxSize;
      }
      size_t newNumSubarrays = newTotalSize / SUBARRAY_SIZE;
      Nod* newTable = nullptr;
      T* newSuperPointers = nullptr;
      try {
        newTable = new Nod[newTotalSize]();
        newSuperPointers = new T[newTotalSize]();
      } catch (const std::bad_alloc&) {
        delete[] newTable;
        delete[] newSuperPointers;
        return false;
      }
      for (size_t i = 0; i < numSubarrays; ++i) {
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          const Nod& entry = table[i * SUBARRAY_SIZE + j];
          if (entry.occupied) {
            reinsert(newTable, newSuperPointers, newTotalSize, newNumSubarrays, entry.key, entry.value);
          }
        }
      }
      delete[] table;
      delete[] superPointers;
      table = newTable;
      superPointers = newSuperPointers;
      totalSize = newTotalSize;
      numSubarrays = newNumSubarrays;
      return true;
    }
    void reinsert(Nod* targetTable, T* targetPointers, size_t targetSize, size_t targetNumSubarrays,
      const K& key, const V& value) noexcept {
      size_t baseIndex = fc_hash(key, targetSize);
      size_t subarrayIdx = baseIndex / SUBARRAY_SIZE;
      size_t offset = baseIndex % SUBARRAY_SIZE;
      for (size_t i = 0; i < targetNumSubarrays; ++i) {
        size_t currentSubarray = (subarrayIdx + i) % targetNumSubarrays;
        size_t startOffset = (i == 0) ? offset : 0;
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          size_t idx = (startOffset + j) % SUBARRAY_SIZE;
          Nod& entry = targetTable[currentSubarray * SUBARRAY_SIZE + idx];
          if (!entry.occupied) {
            entry = Nod(key, std::move(value)); targetPointers[baseIndex] = static_cast<T>(currentSubarray);
            return;
          }
        }
      }
    }
    bool insertImpl(const K& key, const V& value, size_t baseIndex) noexcept {
      size_t subarrayIdx = baseIndex / SUBARRAY_SIZE;
      size_t offset = baseIndex % SUBARRAY_SIZE;
      for (size_t i = 0; i < numSubarrays; ++i) {
        size_t currentSubarray = (subarrayIdx + i) % numSubarrays;
        size_t startOffset = (i == 0) ? offset : 0;
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          size_t idx = (startOffset + j) % SUBARRAY_SIZE;
          Nod& entry = table[currentSubarray * SUBARRAY_SIZE + idx];
          bool wasUnoccupied = !entry.occupied;
          if (wasUnoccupied || equal(entry.key, key)) {
            entry.key = key;
            entry.value = std::move(value);
            entry.occupied = true;
            superPointers[baseIndex] = static_cast<T>(currentSubarray);
            if (wasUnoccupied) ++numEntries;
            return true;
          }
        }
      }
      return false;
    }
  public:
    HashMap(size_t initialSize = sizeof(T) * sizeof(V) * 16) noexcept
      : totalSize(initialSize > MaxSize ? MaxSize : initialSize), numEntries(0), numSubarrays(totalSize / SUBARRAY_SIZE) {
      table = new Nod[totalSize](); static_assert(std::is_integral<T>::value, "Super pointer must be integral!");
      superPointers = new T[totalSize]();
    }
    ~HashMap() {
      delete[] table;
      delete[] superPointers;
    }
    HashMap(const HashMap&) = delete;
    HashMap(HashMap&&) = default;
    bool emplace(const K& key, const V& value) noexcept {
      if (numEntries * 100 > LOAD_FACTOR_THRESHOLD * totalSize) {
        if (!resize()) {
          if (!insertImpl(key, value, fc_hash(key, totalSize))) return false;
          return true;
        }
      }
      return insertImpl(key, value, fc_hash(key, totalSize));
    }
    _FORCE_INLINE void clear() noexcept {
      for (size_t i = 0; i < totalSize; ++i) table[i].occupied = false;
      numEntries = 0;
    }
    V& operator[](const K& key) noexcept {
      size_t baseIndex = fc_hash(key, totalSize);
      size_t subarrayIdx = superPointers[baseIndex];
      for (size_t i = 0; i < SUBARRAY_SIZE; ++i) {
        Nod& entry = table[subarrayIdx * SUBARRAY_SIZE + i];
        if (entry.occupied && equal(entry.key, key)) {
          return entry.value;
        }
      }
      bool resized = false;
      do {
        if (numEntries * 100 > LOAD_FACTOR_THRESHOLD * totalSize) {
          resized = resize();
          if (!resized) {
            return insertImpl(key, dummy, fc_hash(key, totalSize)) ?
              table[fc_hash(key, totalSize)].value : dummy;
          }
          baseIndex = fc_hash(key, totalSize);
        }
        subarrayIdx = baseIndex / SUBARRAY_SIZE;
        size_t offset = baseIndex % SUBARRAY_SIZE;
        for (size_t i = 0; i < numSubarrays; ++i) {
          size_t currentSubarray = (subarrayIdx + i) % numSubarrays;
          size_t startOffset = (i == 0) ? offset : 0;
          for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
            size_t idx = (startOffset + j) % SUBARRAY_SIZE;
            Nod& entry = table[currentSubarray * SUBARRAY_SIZE + idx];
            if (!entry.occupied) {
              entry.key = key;
              entry.occupied = true;
              superPointers[baseIndex] = static_cast<T>(currentSubarray);
              ++numEntries;
              return entry.value;
            }
          }
        }
      } while (resized);
      return dummy;
    }
    //Cover up mistakes
    V& at(const K& key) {
      size_t baseIndex = fc_hash(key, totalSize);
      size_t subarrayIdx = baseIndex / SUBARRAY_SIZE;
      for (size_t i = 0; i < numSubarrays; ++i) {
        size_t currentSubarray = (subarrayIdx + i) % numSubarrays;
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          Nod& entry = table[currentSubarray * SUBARRAY_SIZE + j];
          if (entry.occupied && equal(entry.key, key)) {
            return entry.value;
          }
        }
      }
      return dummy;
    }
    //Can throw exceptions
    const V& at(const K& key) const {
      size_t baseIndex = fc_hash(key, totalSize);
      size_t subarrayIdx = baseIndex / SUBARRAY_SIZE;
      for (size_t i = 0; i < numSubarrays; ++i) {
        size_t currentSubarray = (subarrayIdx + i) % numSubarrays;
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          const Nod& entry = table[currentSubarray * SUBARRAY_SIZE + j];
          if (entry.occupied && equal(entry.key, key)) {
            return entry.value;
          }
        }
      }
      throw std::out_of_range("Not found in HashMap");
    }
    inline void remove(const K& key) noexcept {
      size_t baseIndex = fc_hash(key, totalSize);
      size_t subarrayIdx = baseIndex / SUBARRAY_SIZE;
      for (size_t i = 0; i < numSubarrays; ++i) {
        size_t currentSubarray = (subarrayIdx + i) % numSubarrays;
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          Nod& entry = table[currentSubarray * SUBARRAY_SIZE + j];
          if (entry.occupied && equal(entry.key, key)) {
            entry.occupied = false;
            --numEntries;
            superPointers[baseIndex] = 0;
            return;
          }
        }
      }
    }
    _FORCE_INLINE size_t size() const noexcept { return numEntries; }
    _FORCE_INLINE size_t capacity() const noexcept { return totalSize; }
    class iterator {
      friend HashMap; mutable Nod* curr; Nod* end;
      _FORCE_INLINE void moveToNextOccupied() { while (curr != end) { if (curr->occupied) return; ++curr; } }
    public:
      iterator(HashMap* m, size_t subIdx, size_t slot) noexcept: curr(m->table + subIdx * SUBARRAY_SIZE + slot),
        end(m->table + m->totalSize) {}
      _FORCE_INLINE std::pair<const K, V>* operator->() const noexcept { return reinterpret_cast<std::pair<const K, V>*>(curr); }
      _FORCE_INLINE std::pair<const K, V>& operator*() const noexcept { return reinterpret_cast<std::pair<const K, V>&>(*curr); }
      _FORCE_INLINE iterator& operator++() noexcept { while (++curr != end) { if (curr->occupied) return *this; } return *this; }
      _FORCE_INLINE iterator operator++(int) noexcept { iterator tmp = *this; ++(*this); return tmp; }
      _FORCE_INLINE bool operator==(const iterator& other) const noexcept { return curr == other.curr; }
      _FORCE_INLINE bool operator!=(const iterator& other) const noexcept { return !(*this == other); }
    };
    _FORCE_INLINE iterator begin() noexcept { iterator i(this, 0, 0); i.moveToNextOccupied(); return i; }
    _FORCE_INLINE iterator end() noexcept { return iterator(this, numSubarrays, 0); }
    _FORCE_INLINE iterator find(const K& key) noexcept {
      size_t baseIndex = fc_hash(key, totalSize); size_t subarrayIdx = baseIndex / SUBARRAY_SIZE; size_t i = 0;
      do {
        size_t currentSubarray = (subarrayIdx + i) % numSubarrays;
        Nod* subarray = table + currentSubarray * SUBARRAY_SIZE;
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          Nod& entry = subarray[j];
          if (entry.occupied && equal(entry.key, key)) return iterator(this, currentSubarray, j);
        }
        ++i;
      } while (i < numSubarrays); return end();
    }
    class const_iterator {
      friend HashMap; mutable Nod* curr; Nod* end;
      _FORCE_INLINE void moveToNextOccupied() { while (curr != end) { if (curr->occupied) return; ++curr; } }
    public:
      const_iterator(const HashMap* m, size_t subIdx, size_t slot) noexcept:
        curr(m->table + subIdx * SUBARRAY_SIZE + slot), end(m->table + m->totalSize) {}
      _FORCE_INLINE std::pair<const K, V>* operator->() const noexcept { return reinterpret_cast<std::pair<const K, V>*>(curr); }
      _FORCE_INLINE std::pair<const K, V>& operator*() const noexcept { return reinterpret_cast<std::pair<const K, V>&>(*curr); }
      _FORCE_INLINE const_iterator& operator++() noexcept {
        while (++curr != end) { if (curr->occupied) return *this; } return *this;
      }
      _FORCE_INLINE const_iterator operator++(int) noexcept { const_iterator tmp = *this; ++(*this); return tmp; }
      _FORCE_INLINE bool operator==(const const_iterator& other) const noexcept { return curr == other.curr; }
      _FORCE_INLINE bool operator!=(const const_iterator& other) const noexcept { return !(*this == other); }
    };
    _FORCE_INLINE const const_iterator find(const K& key) const noexcept;
    _FORCE_INLINE const_iterator begin() const noexcept;
    _FORCE_INLINE const_iterator end() const noexcept;
  };
  template<typename K, typename V, typename T, typename E, char LOAD_FACTOR_THRESHOLD>
  _FORCE_INLINE const typename HashMap<K, V, T, E, LOAD_FACTOR_THRESHOLD>::const_iterator
    HashMap<K, V, T, E, LOAD_FACTOR_THRESHOLD>::find(const K& key) const noexcept {
    size_t baseIndex = fc_hash(key, totalSize); size_t subarrayIdx = baseIndex / SUBARRAY_SIZE; size_t i = 0;
    do {
      size_t currentSubarray = (subarrayIdx + i) % numSubarrays;
      const Nod* subarray = table + currentSubarray * SUBARRAY_SIZE;
      for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
        const Nod& entry = subarray[j];
        if (entry.occupied && equal(entry.key, key)) return const_iterator(this, currentSubarray, j);
      }
      ++i;
    } while (i < numSubarrays); return HashMap<K, V, T, E, LOAD_FACTOR_THRESHOLD>::end();
  }
  template<typename K, typename V, typename T, typename E, char LOAD_FACTOR_THRESHOLD>
  _FORCE_INLINE typename HashMap<K, V, T, E, LOAD_FACTOR_THRESHOLD>::const_iterator
    HashMap<K, V, T, E, LOAD_FACTOR_THRESHOLD>::begin() const noexcept {
    const_iterator i(this, 0, 0); i.moveToNextOccupied(); return i;
  }
  template<typename K, typename V, typename T, typename E, char LOAD_FACTOR_THRESHOLD>
  _FORCE_INLINE typename HashMap<K, V, T, E, LOAD_FACTOR_THRESHOLD>::const_iterator
    HashMap<K, V, T, E, LOAD_FACTOR_THRESHOLD>::end() const noexcept {
    return const_iterator(this, numSubarrays, 0);
  }
  template<typename K, typename V, typename T, typename E, char LOAD_FACTOR_THRESHOLD>
  V HashMap<K, V, T, E, LOAD_FACTOR_THRESHOLD>::dummy;
  template<typename T>
  class HashMap<std::string, std::string, T, str_key_eq, 80>: public HashMap<std::string, std::string, T, str_key_eq> {
  public: HashMap(int i = 1024) noexcept: HashMap<std::string, std::string, T, str_key_eq>(i) {}
  };
  template<typename T>
  class HashMap<std::string_view, std::string_view, T, sv_key_eq, 75>: public HashMap<std::string_view, std::string_view, T, sv_key_eq, 80> {
  public: HashMap(int i = 16) noexcept: HashMap<std::string_view, std::string_view, T, sv_key_eq, 80>(i) {}
  };
  template<typename T = uint8_t>
  using sv_hash_map = HashMap<std::string_view, std::string_view, T, sv_key_eq>;
  template<typename T = uint16_t>
  using str_hash_map = HashMap<std::string, std::string, T, str_key_eq, 80>;
  // template<typename T = uint8_t>
  // struct sv_hash_map: fc::HashMap<std::string_view, std::string_view, T, sv_key_eq> {
  //   sv_hash_map(int i = 16) noexcept: HashMap<std::string_view, std::string_view, T, sv_key_eq>(i) {}
  // };
  // template<typename T = uint16_t>
  // struct str_hash_map: fc::HashMap<std::string, std::string, T, str_key_eq, 80> {
  //   str_hash_map(int i = 1024) noexcept: HashMap<std::string, std::string, T, str_key_eq, 80>(i) {}
  // };
}
#endif