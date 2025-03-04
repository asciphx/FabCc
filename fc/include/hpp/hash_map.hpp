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
namespace fc {
  template<typename K>
  _FORCE_INLINE static size_t fc_hash(const K& key, size_t mod) noexcept {
    return (static_cast<size_t>(key) * 0X9e3779b1U) % mod;
  }
  // Custom hash function from str_map for std::string
  template<>
  _FORCE_INLINE size_t fc_hash<std::string>(const std::string& key, size_t mod) noexcept {
    static constexpr unsigned long long m = 0xDFDFDFDFDFDFDFDF & ~uint64_t{ 0 };
    size_t n = key.length(); size_t r = n - 1;
    unsigned char const* p = reinterpret_cast<unsigned char const*>(key.c_str());
    while (n >= 8) {
      r = (r * 5 + ((p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24) |
        (static_cast<size_t>(p[4]) << 32) | (static_cast<size_t>(p[5]) << 40) |
        (static_cast<size_t>(p[6]) << 48) | (static_cast<size_t>(p[7]) << 56)) & ~0x2020202020202020));
      p += 8; n -= 8;
    }
    while (n > 0) { r = r * 5 + (*p | 0x20); ++p; --n; } return (r - key.length()) % mod;
  }
  // FNV-1a hash for strings
  template<>
  _FORCE_INLINE size_t fc_hash<std::string_view>(const std::string_view& key, size_t mod) noexcept {
    static const uint32_t FNV_PRIME = 0X1000193U; static const uint32_t FNV_OFFSET = 0X811c9dc5U;
    uint32_t hash = FNV_OFFSET; size_t n = key.size();
    if (n >= 4) { for (size_t i = 0; i < n; ++i) { hash ^= static_cast<uint32_t>(key[i]); hash *= FNV_PRIME; } } else {
      hash ^= n ? key[0] : 0; hash *= FNV_PRIME; if (n > 1) hash ^= key[1]; hash *= FNV_PRIME; if (n > 2) hash ^= key[2];
    }
    return hash % mod;
  }
  // Query-friendly hash table similar to std::unordered_map, suitable for frequent lookups
  template<typename K, typename V, typename T = uint16_t, char LOAD_FACTOR_THRESHOLD = 75>
  class HashMap {
    struct Nod {
      K key; V value; bool occupied;
      Nod() noexcept: key(), value(), occupied(false) {}
      Nod(const K& k, const V& v) noexcept: key(k), value(v), occupied(true) {}
      Nod(const K& k, V&& v) noexcept: key(k), value(std::move(v)), occupied(true) {}
    };
    Nod* table; T* superPointers; size_t totalSize; size_t numEntries; size_t numSubarrays;
    static const size_t SUBARRAY_SIZE = sizeof(K) - sizeof(T) > 16 ? 16 : sizeof(V) > 16 ? 8 : 32; static V dummy;
    bool resize() {
      size_t newTotalSize = totalSize + (totalSize >> 1); size_t newNumSubarrays = newTotalSize / SUBARRAY_SIZE;
      Nod* newTable = nullptr; T* newSuperPointers = nullptr;
      try {
        newTable = new Nod[newTotalSize](); newSuperPointers = new T[newTotalSize]();
      } catch (const std::bad_alloc&) { return false; }
      for (size_t i = 0; i < totalSize; ++i) {
        Nod& entry = table[i]; if (entry.occupied)
          reinsert(newTable, newSuperPointers, newTotalSize, newNumSubarrays, entry.key, entry.value);
      }
      delete[] table; delete[] superPointers; table = newTable; superPointers = newSuperPointers;
      totalSize = newTotalSize; numSubarrays = newNumSubarrays; return true;
    }
    _FORCE_INLINE void reinsert(Nod* targetTable, T* targetPointers, size_t targetSize,
      size_t targetNumSubarrays, const K& key, const V& value) noexcept {
      size_t baseIndex = fc_hash(key, targetSize); size_t subarrayIdx = baseIndex / SUBARRAY_SIZE;
      size_t offset = baseIndex % SUBARRAY_SIZE;
      size_t i = 0; do {
        size_t currentSubarray = (subarrayIdx + i) % targetNumSubarrays;
        size_t startOffset = (i == 0) ? offset : 0;
        Nod* subarray = targetTable + currentSubarray * SUBARRAY_SIZE; size_t j = 0;
        do {
          size_t idx = (startOffset + j) % SUBARRAY_SIZE; Nod& entry = subarray[idx];
          if (!entry.occupied) {
            entry = Nod(key, value);
            targetPointers[baseIndex] = static_cast<T>(currentSubarray); return;
          }
        } while (++j < SUBARRAY_SIZE); ++i;
      } while (i < targetNumSubarrays);
    }
    _FORCE_INLINE bool tryInsert(Nod* targetTable, T* targetPointers, size_t targetSize,
      size_t targetNumSubarrays, size_t baseIndex, const K& key, const V& value, bool& wasUnoccupied) noexcept {
      size_t subarrayIdx = baseIndex / SUBARRAY_SIZE; size_t offset = baseIndex % SUBARRAY_SIZE;
      size_t i = 0; do {
        size_t currentSubarray = (subarrayIdx + i) % targetNumSubarrays;
        size_t startOffset = (i == 0) ? offset : 0;
        Nod* subarray = targetTable + currentSubarray * SUBARRAY_SIZE; size_t j = 0;
        do {
          size_t idx = (startOffset + j) % SUBARRAY_SIZE; Nod& entry = subarray[idx];
          wasUnoccupied = !entry.occupied; if (wasUnoccupied || entry.key == key) {
            entry.key = key; entry.value = value; entry.occupied = true;
            targetPointers[baseIndex] = static_cast<T>(currentSubarray);
            return true;
          }
        } while (++j < SUBARRAY_SIZE); ++i;
      } while (i < targetNumSubarrays); return false;
    }
    bool insertImpl(const K& key, const V& value, size_t baseIndex) noexcept {
      bool wasUnoccupied = false; if (tryInsert(table, superPointers, totalSize, numSubarrays,
        baseIndex, key, value, wasUnoccupied)) {
        if (wasUnoccupied) ++numEntries; return true;
      }
      size_t subarrayIdx = baseIndex / SUBARRAY_SIZE; size_t offset = baseIndex % SUBARRAY_SIZE;
      size_t attempt = 1;
      while (attempt < numSubarrays) {
        size_t newSubarrayIdx = (subarrayIdx + attempt * attempt) % numSubarrays;
        size_t newBaseIndex = newSubarrayIdx * SUBARRAY_SIZE + offset;
        if (tryInsert(table, superPointers, totalSize, numSubarrays, newBaseIndex, key, value, wasUnoccupied)) {
          if (wasUnoccupied) ++numEntries; return true;
        }
        ++attempt;
      }
      return false;
    }
  public:
    HashMap(size_t initialSize = sizeof(K) * sizeof(T) * 8) noexcept: totalSize(initialSize), numEntries(0),
      numSubarrays(initialSize / SUBARRAY_SIZE) {
      table = new Nod[totalSize](); superPointers = new T[totalSize]();
    }
    ~HashMap() { delete[] table; delete[] superPointers; }
    HashMap(const HashMap&) = delete; HashMap(HashMap&&) = default;
    bool emplace(const K& key, const V& value) noexcept {
      if (numEntries * 100 > LOAD_FACTOR_THRESHOLD * totalSize) {
        if (!resize()) return insertImpl(key, value, fc_hash(key, totalSize));
      }
      return insertImpl(key, value, fc_hash(key, totalSize));
    }
    _FORCE_INLINE void clear() noexcept {
      for (size_t i = 0; i < totalSize; ++i) table[i].occupied = false; numEntries = 0;
    }
    V& operator[](const K& key) noexcept {
      size_t baseIndex = fc_hash(key, totalSize); size_t subarrayIdx = superPointers[baseIndex];
      if (subarrayIdx) {
        Nod* subarray = table + subarrayIdx * SUBARRAY_SIZE; for (size_t i = 0; i < SUBARRAY_SIZE; ++i) {
          Nod& entry = subarray[i]; if (entry.occupied && entry.key == key) return entry.value;
        }
      }
      bool resized = false; do {
        if (numEntries * 100 > LOAD_FACTOR_THRESHOLD * totalSize) {
          resized = resize();
          if (!resized) { insertImpl(key, dummy, baseIndex); return table[baseIndex].value; }
          baseIndex = fc_hash(key, totalSize);
        }
        subarrayIdx = baseIndex / SUBARRAY_SIZE; size_t offset = baseIndex % SUBARRAY_SIZE; size_t i = 0;
        do {
          size_t currentSubarray = (subarrayIdx + i) % numSubarrays;
          Nod* subarray = table + currentSubarray * SUBARRAY_SIZE;
          size_t startOffset = (i == 0) ? offset : 0; size_t j = 0;
          do {
            size_t idx = (startOffset + j) % SUBARRAY_SIZE; Nod& entry = subarray[idx];
            if (!entry.occupied) {
              entry.key = key; entry.occupied = true;
              superPointers[baseIndex] = static_cast<T>(currentSubarray); ++numEntries; return entry.value;
            }
          } while (++j < SUBARRAY_SIZE); ++i;
        } while (i < numSubarrays);
      } while (resized); return dummy;
    }
    //Cover up mistakes
    V& at(const K& key) {
      size_t baseIndex = fc_hash(key, totalSize); size_t subarrayIdx = baseIndex / SUBARRAY_SIZE; size_t i = 0;
      do {
        size_t currentSubarray = (subarrayIdx + i) % numSubarrays;
        Nod* subarray = table + currentSubarray * SUBARRAY_SIZE;
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          Nod& entry = subarray[j];
          if (entry.occupied && entry.key == key) return entry.value;
        }
        ++i;
      } while (i < numSubarrays); return dummy;
    }
    //Can throw exceptions
    const V& at(const K& key) const {
      size_t baseIndex = fc_hash(key, totalSize); size_t subarrayIdx = baseIndex / SUBARRAY_SIZE; size_t i = 0;
      do {
        size_t currentSubarray = (subarrayIdx + i) % numSubarrays;
        const Nod* subarray = table + currentSubarray * SUBARRAY_SIZE;
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          const Nod& entry = subarray[j];
          if (entry.occupied && entry.key == key) return entry.value;
        }
        ++i;
      } while (i < numSubarrays); throw std::out_of_range("Not found in HashMap");
    }
    _FORCE_INLINE void remove(const K& key) noexcept {
      size_t baseIndex = fc_hash(key, totalSize); size_t subarrayIdx = baseIndex / SUBARRAY_SIZE; size_t i = 0;
      do {
        size_t currentSubarray = (subarrayIdx + i) % numSubarrays;
        Nod* subarray = table + currentSubarray * SUBARRAY_SIZE;
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          Nod& entry = subarray[j]; if (entry.occupied && entry.key == key) {
            entry.occupied = false;
            --numEntries; superPointers[baseIndex] = 0; return;
          }
        }
        ++i;
      } while (i < numSubarrays);
    }
    _FORCE_INLINE size_t size() const noexcept { return numEntries; }
    _FORCE_INLINE size_t capacity() const noexcept { return totalSize; }
    class iterator {
      friend HashMap; mutable Nod* curr; Nod* end; HashMap* map;
      _FORCE_INLINE void moveToNextOccupied() { while (curr != end) { if (curr->occupied) return; ++curr; } }
    public:
      iterator(HashMap* m, size_t subIdx, size_t slot) noexcept: curr(m->table + subIdx * SUBARRAY_SIZE + slot),
        end(m->table + m->totalSize), map(m) {}
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
          if (entry.occupied && entry.key == key) return iterator(this, currentSubarray, j);
        }
        ++i;
      } while (i < numSubarrays); return end();
    }
    class const_iterator {
      friend HashMap; mutable Nod* curr; Nod* end; const HashMap* map;
      _FORCE_INLINE void moveToNextOccupied() { while (curr != end) { if (curr->occupied) return; ++curr; } }
    public:
      const_iterator(const HashMap* m, size_t subIdx, size_t slot) noexcept:
        curr(m->table + subIdx * SUBARRAY_SIZE + slot), end(m->table + m->totalSize), map(m) {}
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
  template<typename K, typename V, typename T, char LOAD_FACTOR_THRESHOLD>
  _FORCE_INLINE const typename HashMap<K, V, T, LOAD_FACTOR_THRESHOLD>::const_iterator
    HashMap<K, V, T, LOAD_FACTOR_THRESHOLD>::find(const K& key) const noexcept {
    size_t baseIndex = fc_hash(key, totalSize); size_t subarrayIdx = baseIndex / SUBARRAY_SIZE; size_t i = 0;
    do {
      size_t currentSubarray = (subarrayIdx + i) % numSubarrays;
      const Nod* subarray = table + currentSubarray * SUBARRAY_SIZE;
      for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
        const Nod& entry = subarray[j];
        if (entry.occupied && entry.key == key) return const_iterator(this, currentSubarray, j);
      }
      ++i;
    } while (i < numSubarrays); return HashMap<K, V, T, LOAD_FACTOR_THRESHOLD>::end();
  }
  template<typename K, typename V, typename T, char LOAD_FACTOR_THRESHOLD>
  _FORCE_INLINE typename HashMap<K, V, T, LOAD_FACTOR_THRESHOLD>::const_iterator
    HashMap<K, V, T, LOAD_FACTOR_THRESHOLD>::begin() const noexcept {
    const_iterator i(this, 0, 0); i.moveToNextOccupied(); return i;
  }
  template<typename K, typename V, typename T, char LOAD_FACTOR_THRESHOLD>
  _FORCE_INLINE typename HashMap<K, V, T, LOAD_FACTOR_THRESHOLD>::const_iterator
    HashMap<K, V, T, LOAD_FACTOR_THRESHOLD>::end() const noexcept {
    return const_iterator(this, numSubarrays, 0);
  }
  template<typename K, typename V, typename T, char LOAD_FACTOR_THRESHOLD>
  V HashMap<K, V, T, LOAD_FACTOR_THRESHOLD>::dummy;
  template<>
  class HashMap<std::string, std::string, uint32_t, 80>: public HashMap<std::string, std::string> {
    HashMap<std::string, std::string, uint32_t, 80>(int i = 1024): HashMap<std::string, std::string>(i) {}
  };
  template<>
  class HashMap<std::string_view, std::string_view, uint8_t, 70>: public HashMap<std::string_view, std::string_view> {
    HashMap<std::string_view, std::string_view, uint8_t, 70>(int i = 16): HashMap<std::string_view, std::string_view>(i) {}
  };
  struct sv_hash_map: fc::HashMap<std::string_view, std::string_view> {};
  template<typename T>
  struct str_hash_map: fc::HashMap<std::string, std::string, T> {};
}
#endif