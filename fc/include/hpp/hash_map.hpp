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
  inline static size_t fc_hash(const K& key, size_t mod) noexcept {
    return (static_cast<size_t>(key) * 0X9e3779b1U) % mod;
  }
  // Custom hash function from str_map for std::string
  template<>
  inline size_t fc_hash<std::string>(const std::string& key, size_t mod) noexcept {
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
  template<>
  inline size_t fc_hash<std::string_view>(const std::string_view& key, size_t mod) noexcept {
    // FNV-1a hash for strings
    static const uint32_t FNV_PRIME = 0X1000193U;
    static const uint32_t FNV_OFFSET = 0X811c9dc5U;
    uint32_t hash = FNV_OFFSET;
    for (size_t i = 0; i < key.size(); ++i) {
      hash ^= static_cast<uint32_t>(key[i]);
      hash *= FNV_PRIME;
    }
    return hash % mod;
  }
  template<typename K, typename V, typename T = uint16_t, char LOAD_FACTOR_THRESHOLD = 75>
  class HashMap {
    struct Nod {
      K key;
      V value;
      bool occupied;
      Nod() noexcept: key(), value(), occupied(false) {}
      Nod(const K& k, const V& v) noexcept: key(k), value(v), occupied(true) {}
    };
    Nod* table;
    T* superPointers;
    size_t totalSize;
    size_t numEntries;
    size_t numSubarrays;
    static const size_t SUBARRAY_SIZE = sizeof(T) > 3 ? 16 : 8;
    static V dummy;
    bool resize() {
      size_t newTotalSize = totalSize + (totalSize >> 1);
      size_t newNumSubarrays = newTotalSize / SUBARRAY_SIZE;
      Nod* newTable = nullptr;
      T* newSuperPointers = nullptr;
      try {
        newTable = new Nod[newTotalSize]();
        newSuperPointers = new T[newTotalSize]();
      } catch (const std::bad_alloc&) {
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
    void reinsert(Nod* targetTable, T* targetPointers, size_t targetSize, size_t targetNumSubarrays, const K& key, const V& value) noexcept {
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
            entry = Nod(key, value);
            targetPointers[baseIndex] = static_cast<T>(currentSubarray);
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
          if (wasUnoccupied || entry.key == key) {
            entry.key = key;
            entry.value = value;
            entry.occupied = true;
            superPointers[baseIndex] = static_cast<T>(currentSubarray);
            if (wasUnoccupied) ++numEntries;
            return true;
          }
        }
      }
      size_t attempt = 1;
      while (attempt < numSubarrays) {
        size_t newSubarrayIdx = (subarrayIdx + attempt * attempt) % numSubarrays;
        size_t newBaseIndex = newSubarrayIdx * SUBARRAY_SIZE + offset;
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          size_t idx = (offset + j) % SUBARRAY_SIZE;
          Nod& entry = table[newSubarrayIdx * SUBARRAY_SIZE + idx];
          bool wasUnoccupied = !entry.occupied;
          if (wasUnoccupied || entry.key == key) {
            entry.key = std::move(key);
            entry.value = std::move(value);
            entry.occupied = true;
            superPointers[newBaseIndex] = static_cast<T>(newSubarrayIdx);
            if (wasUnoccupied) ++numEntries;
            return true;
          }
        }
        ++attempt;
      }
      return false;
    }
  public:
    HashMap(size_t initialSize = sizeof(K) * 32) noexcept
      : totalSize(initialSize), numEntries(0), numSubarrays(initialSize / SUBARRAY_SIZE) {
      table = new Nod[totalSize]();
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
          return insertImpl(key, value, fc_hash(key, totalSize));
        }
      }
      return insertImpl(key, value, fc_hash(key, totalSize));
    }
    _FORCE_INLINE void clear() noexcept {
      for (numEntries = 0; numEntries < totalSize; ++numEntries) table[numEntries].occupied = false; numEntries = 0;
    }
    V& operator[](const K& key) noexcept {
      size_t baseIndex = fc_hash(key, totalSize);
      size_t subarrayIdx = superPointers[baseIndex];
      for (size_t i = 0; i < SUBARRAY_SIZE; ++i) {
        Nod& entry = table[subarrayIdx * SUBARRAY_SIZE + i];
        if (entry.occupied && entry.key == key) {
          return entry.value;
        }
      }
      bool resized = false;
      do {
        if (numEntries * 100 > LOAD_FACTOR_THRESHOLD * totalSize) {
          resized = resize();
          if (!resized) {
            return insertImpl(key, dummy, fc_hash(key, totalSize)) ? table[fc_hash(key, totalSize)].value : dummy;
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
              entry.key = std::move(key);
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
          if (entry.occupied && entry.key == key) {
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
          if (entry.occupied && entry.key == key) {
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
          if (entry.occupied && entry.key == key) {
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
      friend HashMap;
      Nod* table;
      size_t subarrayIdx, slotIdx, numSubarrays;
      inline void moveToNextOccupied() noexcept {
        while (subarrayIdx != numSubarrays) {
          while (slotIdx != SUBARRAY_SIZE) {
            if (table[subarrayIdx * SUBARRAY_SIZE + slotIdx].occupied) { return; }
            ++slotIdx;
          } slotIdx = 0, ++subarrayIdx;
        }
      }
    public:
      iterator(Nod* t, size_t subIdx, size_t slot, size_t numSubs) noexcept
        : table(t), subarrayIdx(subIdx), slotIdx(slot), numSubarrays(numSubs) {}
      _FORCE_INLINE std::pair<const K, V>* operator->() const noexcept {
        return reinterpret_cast<std::pair<const K, V>*>(&table[subarrayIdx * SUBARRAY_SIZE + slotIdx]);
      }
      _FORCE_INLINE std::pair<const K, V>& operator*() const noexcept {
        return reinterpret_cast<std::pair<const K, V>&>(table[subarrayIdx * SUBARRAY_SIZE + slotIdx]);
      }
      _FORCE_INLINE iterator& operator++() noexcept {
        ++slotIdx;
        moveToNextOccupied();
        return *this;
      }
      _FORCE_INLINE iterator operator++(int) noexcept {
        iterator tmp = *this;
        ++(*this);
        return tmp;
      }
      _FORCE_INLINE bool operator==(const iterator& other) const noexcept {
        return subarrayIdx == other.subarrayIdx && slotIdx == other.slotIdx;
      }
      _FORCE_INLINE bool operator!=(const iterator& other) const noexcept {
        return !(*this == other);
      }
    };
    _FORCE_INLINE iterator begin() noexcept {
      iterator i(table, 0, 0, numSubarrays); i.moveToNextOccupied(); return i;
    }
    _FORCE_INLINE iterator end() noexcept {
      return iterator(table, numSubarrays, 0, numSubarrays);
    }
    inline iterator find(const K& key) noexcept {
      size_t baseIndex = fc_hash(key, totalSize);
      size_t subarrayIdx = baseIndex / SUBARRAY_SIZE;
      for (size_t i = 0; i < numSubarrays; ++i) {
        size_t currentSubarray = (subarrayIdx + i) % numSubarrays;
        for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
          Nod& entry = table[currentSubarray * SUBARRAY_SIZE + j];
          if (entry.occupied && entry.key == key) {
            return iterator(table, currentSubarray, j, numSubarrays);
          }
        }
      }
      return end();
    }
    class const_iterator {
      friend HashMap;
      Nod* table;
      size_t subarrayIdx, slotIdx, numSubarrays;
      inline void moveToNextOccupied() {
        while (subarrayIdx != numSubarrays) {
          while (slotIdx != SUBARRAY_SIZE) {
            if (table[subarrayIdx * SUBARRAY_SIZE + slotIdx].occupied) { return; }
            ++slotIdx;
          } slotIdx = 0, ++subarrayIdx;
        }
      }
    public:
      const_iterator(Nod* t, size_t subIdx, size_t slot, size_t numSubs) noexcept
        : table(t), subarrayIdx(subIdx), slotIdx(slot), numSubarrays(numSubs) {}
      _FORCE_INLINE std::pair<const K, const V>* operator->() const noexcept {
        return reinterpret_cast<std::pair<const K, const V>*>(&table[subarrayIdx * SUBARRAY_SIZE + slotIdx]);
      }
      _FORCE_INLINE std::pair<const K, const V>& operator*() const noexcept {
        return reinterpret_cast<std::pair<const K, const V>&>(table[subarrayIdx * SUBARRAY_SIZE + slotIdx]);
      }
      _FORCE_INLINE const_iterator& operator++() noexcept {
        ++slotIdx;
        moveToNextOccupied();
        return *this;
      }
      _FORCE_INLINE const_iterator operator++(int) noexcept {
        const_iterator tmp = *this;
        ++(*this);
        return tmp;
      }
      _FORCE_INLINE bool operator==(const const_iterator& other) const noexcept {
        return subarrayIdx == other.subarrayIdx && slotIdx == other.slotIdx;
      }
      _FORCE_INLINE bool operator!=(const const_iterator& other) const noexcept {
        return !(*this == other);
      }
    };
    _FORCE_INLINE const const_iterator find(const K& key) const noexcept;
    _FORCE_INLINE const_iterator begin() const noexcept;
    _FORCE_INLINE const_iterator end() const noexcept;
  };
  template<typename K, typename V, typename T, char LOAD_FACTOR_THRESHOLD>
  inline const typename HashMap<K, V, T, LOAD_FACTOR_THRESHOLD>::const_iterator HashMap<K, V, T, LOAD_FACTOR_THRESHOLD>::find(const K& key) const noexcept {
    size_t baseIndex = fc_hash(key, totalSize);
    size_t subarrayIdx = baseIndex / SUBARRAY_SIZE;
    for (size_t i = 0; i < numSubarrays; ++i) {
      size_t currentSubarray = (subarrayIdx + i) % numSubarrays;
      for (size_t j = 0; j < SUBARRAY_SIZE; ++j) {
        const Nod& entry = table[currentSubarray * SUBARRAY_SIZE + j];
        if (entry.occupied && entry.key == key) {
          return const_iterator(table, currentSubarray, j, numSubarrays);
        }
      }
    }
    return HashMap<K, V, T, LOAD_FACTOR_THRESHOLD>::end();
  }
  template<typename K, typename V, typename T, char LOAD_FACTOR_THRESHOLD>
  _FORCE_INLINE typename HashMap<K, V, T, LOAD_FACTOR_THRESHOLD>::const_iterator
    HashMap<K, V, T, LOAD_FACTOR_THRESHOLD>::begin() const noexcept {
    const_iterator i(table, 0, 0, numSubarrays); i.moveToNextOccupied(); return i;
  }
  template<typename K, typename V, typename T, char LOAD_FACTOR_THRESHOLD>
  _FORCE_INLINE typename HashMap<K, V, T, LOAD_FACTOR_THRESHOLD>::const_iterator
    HashMap<K, V, T, LOAD_FACTOR_THRESHOLD>::end() const noexcept {
    return const_iterator(table, numSubarrays, 0, numSubarrays);
  }
  template<typename K, typename V, typename T, char LOAD_FACTOR_THRESHOLD>
  V HashMap<K, V, T, LOAD_FACTOR_THRESHOLD>::dummy;
  template<>
  class HashMap<std::string, std::string, uint32_t, 80>: public HashMap<std::string, std::string> {};
  template<>
  class HashMap<std::string_view, std::string_view, uint8_t, 85>: public HashMap<std::string_view, std::string_view> {};
  using sv_hash_map = fc::HashMap<std::string_view, std::string_view>;
}
#endif