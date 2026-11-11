#pragma once

#include <new>
#include <stddef.h>
#include <stdint.h>

namespace core {

// Storage held inside the queue object (static RAM when the queue is a global).
template <typename T, size_t Capacity>
struct InlineStorage {
  T items[Capacity];
  bool initialize() { return true; }
  bool ready() const { return true; }
  T *ptr() { return items; }
  const T *ptr() const { return items; }
};

// Storage allocated from the heap on explicit initialization, after runtime startup.
// Use it for large queues that do not fit in the classic ESP32's static DRAM region.
template <typename T, size_t Capacity>
struct HeapStorage {
  T *items;
  HeapStorage() : items(nullptr) {}
  ~HeapStorage() { delete[] items; }
  HeapStorage(const HeapStorage &) = delete;
  HeapStorage &operator=(const HeapStorage &) = delete;
  bool initialize() {
    if (items == nullptr) {
      items = new (std::nothrow) T[Capacity];
    }
    return items != nullptr;
  }
  bool ready() const { return items != nullptr; }
  T *ptr() { return items; }
  const T *ptr() const { return items; }
};

template <typename T, size_t Capacity, template <typename, size_t> class Storage = InlineStorage>
class RingQueue {
 public:
  RingQueue() : head_(0), tail_(0), count_(0) {}

  bool initialize() { return storage_.initialize(); }

  bool push(const T &item) {
    if (!storage_.ready() || count_ >= Capacity) {
      return false;
    }
    storage_.ptr()[head_] = item;
    head_ = (head_ + 1) % Capacity;
    ++count_;
    return true;
  }

  bool pop(T &out) {
    if (!storage_.ready() || count_ == 0) {
      return false;
    }
    out = storage_.ptr()[tail_];
    tail_ = (tail_ + 1) % Capacity;
    --count_;
    return true;
  }

  bool pushDropOldest(const T &item, bool *dropped_oldest = nullptr) {
    bool dropped = false;
    if (!storage_.ready()) {
      if (dropped_oldest != nullptr) {
        *dropped_oldest = false;
      }
      return false;
    }
    if (count_ >= Capacity) {
      tail_ = (tail_ + 1) % Capacity;
      --count_;
      dropped = true;
    }
    (void)push(item);
    if (dropped_oldest != nullptr) {
      *dropped_oldest = dropped;
    }
    return true;
  }

  // Oldest item without removing it, or nullptr when empty.
  const T *front() const {
    return !storage_.ready() || count_ == 0 ? nullptr : &storage_.ptr()[tail_];
  }

  bool empty() const { return count_ == 0; }
  bool full() const { return count_ >= Capacity; }
  size_t size() const { return count_; }
  size_t capacity() const { return Capacity; }
  size_t freeSlots() const { return Capacity - count_; }

  void clear() {
    head_ = 0;
    tail_ = 0;
    count_ = 0;
  }

 private:
  Storage<T, Capacity> storage_;
  size_t head_;
  size_t tail_;
  size_t count_;
};

} // namespace core
