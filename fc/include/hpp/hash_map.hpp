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
#if 1
  // （2³² × (√5 - 1) / 2）
  template<typename K>
  _FORCE_INLINE static size_t fc_hash(const K& key, size_t mod) noexcept {
    return (static_cast<size_t>(key) * 0X9e3779b1U) & (mod - 1);
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
    while (n--) r = r * 5 + (*p++ & 0xDF); return (r - key.length()) & (mod - 1);
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
    while (n--) r = r * 5 + (*p++ & 0xDF); return r & (mod - 1);
  }
  static const unsigned long long fc_2x[0x100] = {
      16ULL,
      32ULL,                 64ULL,                 128ULL,                256ULL,
      512ULL,                1024ULL,               2048ULL,               4096ULL,
      8192ULL,               16384ULL,              32768ULL,              65536ULL,
      131072ULL,             262144ULL,             524288ULL,             1048576ULL,
      2097152ULL,            4194304ULL,            8388608ULL,            16777216ULL,
      33554432ULL,           67108864ULL,           134217728ULL,          268435456ULL,
      536870912ULL,          1073741824ULL,         2147483648ULL,         4294967296ULL,
      8589934592ULL,         17179869184ULL,        34359738368ULL,        68719476736ULL,
      137438953472ULL,       274877906944ULL,       549755813888ULL,       1099511627776ULL,
      2199023255552ULL,      4398046511104ULL,      8796093022208ULL,      17592186044416ULL,
      35184372088832ULL,     70368744177664ULL,     140737488355328ULL,    281474976710656ULL,
      562949953421312ULL,    1125899906842624ULL,   2251799813685248ULL,   4503599627370496ULL,
      9007199254740992ULL,   18014398509481984ULL,  36028797018963968ULL,  72057594037927936ULL,
      144115188075855872ULL, 288230376151711744ULL, 576460752303423488ULL, 1152921504606846976ULL,
      2305843009213693952ULL,4611686018427387904ULL,9223372036854775808ULL,256ULL,
      256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,
      256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,
      256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,
      256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,
      256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,
      256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,
      256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,
      256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,
      256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,
      256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,
      256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,
      256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,
      256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,
      256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,
      256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL,256ULL
  };
  template<typename Z>
  static constexpr const uint64_t fc_HashMax(size_t _) { return (static_cast<uint64_t>(static_cast<Z>(-1)) >> 1) << _; };
  template<> constexpr const uint64_t fc_HashMax<uint64_t>(size_t _) { return 9223372036854775808ULL; };
  template<> constexpr const uint64_t fc_HashMax<uint32_t>(size_t _) { return 2147483648ULL << _; };
  template<> constexpr const uint64_t fc_HashMax<uint16_t>(size_t _) { return 32768ULL << _; };
  template<> constexpr const uint64_t fc_HashMax<uint8_t>(size_t _) { return 128ULL << _; };
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
    static const constexpr size_t SUBARRAY_DIV = SUBARRAY_SIZE == 0x10 ? 0x4 : 0x5;
    static V dummy;
    static const constexpr uint64_t MaxSize = fc_HashMax<T>(SUBARRAY_DIV);
    // Optimization: Reduced exception path overhead by using noexcept and manual cleanup
    bool resize() noexcept {
      size_t newTotalSize = totalSize << 1;
      if (newTotalSize > MaxSize || newTotalSize <= totalSize) {
        if (totalSize == MaxSize) return false; newTotalSize = MaxSize;
      }
      size_t newNumSubarrays = newTotalSize >> SUBARRAY_DIV;
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
          const Nod& entry = table[(i << SUBARRAY_DIV) + j];
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
      size_t subarrayIdx = baseIndex >> SUBARRAY_DIV;
      size_t offset = baseIndex & (SUBARRAY_SIZE - 1);
      for (size_t i = 0; i < targetNumSubarrays; ++i) {
        size_t currentSubarray = (subarrayIdx + i) & (targetNumSubarrays - 1);
        size_t startOffset = (i == 0) ? offset : 0;
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          size_t idx = (startOffset + j) & (SUBARRAY_SIZE - 1);
          Nod& entry = targetTable[(currentSubarray << SUBARRAY_DIV) + idx];
          if (!entry.occupied) {
            entry = Nod(key, std::move(value)); targetPointers[baseIndex] = static_cast<T>(currentSubarray);
            return;
          }
        }
      }
    }
    bool insertImpl(const K& key, const V& value, size_t baseIndex) noexcept {
      size_t subarrayIdx = baseIndex >> SUBARRAY_DIV;
      size_t offset = baseIndex & (SUBARRAY_SIZE - 1);
      for (size_t i = 0; i < numSubarrays; ++i) {
        size_t currentSubarray = (subarrayIdx + i) & (numSubarrays - 1);
        size_t startOffset = (i == 0) ? offset : 0;
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          size_t idx = (startOffset + j) & (SUBARRAY_SIZE - 1);
          Nod& entry = table[(currentSubarray << SUBARRAY_DIV) + idx];
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
    HashMap(uint8_t init = sizeof(T) + 3) noexcept
      : totalSize(fc_2x[init]), numEntries(0) {
      if (totalSize > MaxSize)totalSize = MaxSize; numSubarrays = totalSize >> SUBARRAY_DIV;
      table = new Nod[totalSize](); static_assert(std::is_unsigned<T>::value, "Super pointer must be unsigned!");
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
        Nod& entry = table[(subarrayIdx << SUBARRAY_DIV) + i];
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
        subarrayIdx = baseIndex >> SUBARRAY_DIV;
        size_t offset = baseIndex & (SUBARRAY_SIZE - 1);
        for (size_t i = 0; i < numSubarrays; ++i) {
          size_t currentSubarray = (subarrayIdx + i) & (numSubarrays - 1);
          size_t startOffset = (i == 0) ? offset : 0;
          for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
            size_t idx = (startOffset + j) & (SUBARRAY_SIZE - 1);
            Nod& entry = table[(currentSubarray << SUBARRAY_DIV) + idx];
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
      size_t subarrayIdx = baseIndex >> SUBARRAY_DIV;
      for (size_t i = 0; i < numSubarrays; ++i) {
        size_t currentSubarray = (subarrayIdx + i) & (numSubarrays - 1);
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          Nod& entry = table[(currentSubarray << SUBARRAY_DIV) + j];
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
      size_t subarrayIdx = baseIndex >> SUBARRAY_DIV;
      for (size_t i = 0; i < numSubarrays; ++i) {
        size_t currentSubarray = (subarrayIdx + i) & (numSubarrays - 1);
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          const Nod& entry = table[(currentSubarray << SUBARRAY_DIV) + j];
          if (entry.occupied && equal(entry.key, key)) {
            return entry.value;
          }
        }
      }
      throw std::out_of_range("Not found in HashMap");
    }
    inline void remove(const K& key) noexcept {
      size_t baseIndex = fc_hash(key, totalSize);
      size_t subarrayIdx = baseIndex >> SUBARRAY_DIV;
      for (size_t i = 0; i < numSubarrays; ++i) {
        size_t currentSubarray = (subarrayIdx + i) & (numSubarrays - 1);
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          Nod& entry = table[(currentSubarray << SUBARRAY_DIV) + j];
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
      iterator(HashMap* m, size_t subIdx, size_t slot) noexcept: curr(m->table + (subIdx << SUBARRAY_DIV) + slot),
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
      size_t baseIndex = fc_hash(key, totalSize); size_t subarrayIdx = baseIndex >> SUBARRAY_DIV; size_t i = 0;
      do {
        size_t currentSubarray = (subarrayIdx + i) & (numSubarrays - 1);
        Nod* subarray = table + (currentSubarray << SUBARRAY_DIV);
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
        curr(m->table + (subIdx << SUBARRAY_DIV) + slot), end(m->table + m->totalSize) {}
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
  template<typename T>
  class HashMap<std::string, std::string, T, str_key_eq, 80>: public HashMap<std::string, std::string, T, str_key_eq> {
  public: HashMap(uint8_t i = 6) noexcept: HashMap<std::string, std::string, T, str_key_eq>(i) {}
  };
  template<typename T>
  class HashMap<std::string_view, std::string_view, T, sv_key_eq, 75>: public HashMap<std::string_view, std::string_view, T, sv_key_eq, 80> {
  public: HashMap(uint8_t i = 0) noexcept: HashMap<std::string_view, std::string_view, T, sv_key_eq, 80>(i) {}
  };
#else
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
    static const constexpr size_t SUBARRAY_DIV = SUBARRAY_SIZE == 0x10 ? 0x4 : 0x5;
    static V dummy;
    static const constexpr uint64_t MaxSize = fc_HashMax<T>(SUBARRAY_SIZE);
    // Optimization: Reduced exception path overhead by using noexcept and manual cleanup
    bool resize() noexcept {
      size_t newTotalSize = fc_HashResize<T>(totalSize);
      if (newTotalSize > MaxSize || newTotalSize <= totalSize) {
        if (totalSize == MaxSize) return false; newTotalSize = MaxSize;
      }
      size_t newNumSubarrays = newTotalSize >> SUBARRAY_DIV;
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
          const Nod& entry = table[(i << SUBARRAY_DIV) + j];
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
      size_t subarrayIdx = baseIndex >> SUBARRAY_DIV;
      size_t offset = baseIndex % SUBARRAY_SIZE;
      for (size_t i = 0; i < targetNumSubarrays; ++i) {
        size_t currentSubarray = (subarrayIdx + i) % targetNumSubarrays;
        size_t startOffset = (i == 0) ? offset : 0;
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          size_t idx = (startOffset + j) % SUBARRAY_SIZE;
          Nod& entry = targetTable[(currentSubarray << SUBARRAY_DIV) + idx];
          if (!entry.occupied) {
            entry = Nod(key, std::move(value)); targetPointers[baseIndex] = static_cast<T>(currentSubarray);
            return;
          }
        }
      }
    }
    bool insertImpl(const K& key, const V& value, size_t baseIndex) noexcept {
      size_t subarrayIdx = baseIndex >> SUBARRAY_DIV;
      size_t offset = baseIndex % SUBARRAY_SIZE;
      for (size_t i = 0; i < numSubarrays; ++i) {
        size_t currentSubarray = (subarrayIdx + i) % numSubarrays;
        size_t startOffset = (i == 0) ? offset : 0;
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          size_t idx = (startOffset + j) % SUBARRAY_SIZE;
          Nod& entry = table[(currentSubarray << SUBARRAY_DIV) + idx];
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
      : totalSize(initialSize > MaxSize ? MaxSize : initialSize), numEntries(0), numSubarrays(totalSize >> SUBARRAY_DIV) {
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
        Nod& entry = table[(subarrayIdx << SUBARRAY_DIV) + i];
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
        subarrayIdx = baseIndex >> SUBARRAY_DIV;
        size_t offset = baseIndex % SUBARRAY_SIZE;
        for (size_t i = 0; i < numSubarrays; ++i) {
          size_t currentSubarray = (subarrayIdx + i) % numSubarrays;
          size_t startOffset = (i == 0) ? offset : 0;
          for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
            size_t idx = (startOffset + j) % SUBARRAY_SIZE;
            Nod& entry = table[(currentSubarray << SUBARRAY_DIV) + idx];
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
      size_t subarrayIdx = baseIndex >> SUBARRAY_DIV;
      for (size_t i = 0; i < numSubarrays; ++i) {
        size_t currentSubarray = (subarrayIdx + i) % numSubarrays;
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          Nod& entry = table[(currentSubarray << SUBARRAY_DIV) + j];
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
      size_t subarrayIdx = baseIndex >> SUBARRAY_DIV;
      for (size_t i = 0; i < numSubarrays; ++i) {
        size_t currentSubarray = (subarrayIdx + i) % numSubarrays;
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          const Nod& entry = table[(currentSubarray << SUBARRAY_DIV) + j];
          if (entry.occupied && equal(entry.key, key)) {
            return entry.value;
          }
        }
      }
      throw std::out_of_range("Not found in HashMap");
    }
    inline void remove(const K& key) noexcept {
      size_t baseIndex = fc_hash(key, totalSize);
      size_t subarrayIdx = baseIndex >> SUBARRAY_DIV;
      for (size_t i = 0; i < numSubarrays; ++i) {
        size_t currentSubarray = (subarrayIdx + i) % numSubarrays;
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          Nod& entry = table[(currentSubarray << SUBARRAY_DIV) + j];
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
      iterator(HashMap* m, size_t subIdx, size_t slot) noexcept: curr(m->table + (subIdx << SUBARRAY_DIV) + slot),
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
      size_t subarrayIdx = fc_hash(key, totalSize) >> SUBARRAY_DIV; size_t i = 0;
      do {
        size_t currentSubarray = (subarrayIdx + i) % numSubarrays;
        Nod* subarray = table + (currentSubarray << SUBARRAY_DIV);
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
        curr(m->table + (subIdx << SUBARRAY_DIV) + slot), end(m->table + m->totalSize) {}
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
  template<typename T>
  class HashMap<std::string, std::string, T, str_key_eq, 80>: public HashMap<std::string, std::string, T, str_key_eq> {
  public: HashMap(int i = 1024) noexcept: HashMap<std::string, std::string, T, str_key_eq>(i) {}
  };
  template<typename T>
  class HashMap<std::string_view, std::string_view, T, sv_key_eq, 75>: public HashMap<std::string_view, std::string_view, T, sv_key_eq, 80> {
  public: HashMap(int i = 16) noexcept: HashMap<std::string_view, std::string_view, T, sv_key_eq, 80>(i) {}
  };
#endif
  template<typename K, typename V, typename T, typename E, char LOAD_FACTOR_THRESHOLD>
  _FORCE_INLINE const typename HashMap<K, V, T, E, LOAD_FACTOR_THRESHOLD>::const_iterator
    HashMap<K, V, T, E, LOAD_FACTOR_THRESHOLD>::find(const K& key) const noexcept {
    size_t subarrayIdx = fc_hash(key, totalSize) >> SUBARRAY_DIV; size_t i = 0;
    do {
      size_t currentSubarray = (subarrayIdx + i) & (numSubarrays - 1);
      const Nod* subarray = table + (currentSubarray << SUBARRAY_DIV);
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
  template<typename T = uint8_t>
  using sv_hash_map = HashMap<std::string_view, std::string_view, T, sv_key_eq>;
  template<typename T = uint16_t>
  using str_hash_map = HashMap<std::string, std::string, T, str_key_eq, 80>;
}
#endif