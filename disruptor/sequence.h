// Copyright 2011 <François Saint-Jacques>

#include <atomic>

#include "disruptor/utils.h"

#ifndef CACHE_LINE_SIZE_IN_BYTES // NOLINT
#define CACHE_LINE_SIZE_IN_BYTES 64 // NOLINT
#endif // NOLINT
#define SEQUENCE_PADDING_LENGTH \
    (CACHE_LINE_SIZE_IN_BYTES - sizeof(std::atomic<int64_t>))/8

#ifndef DISRUPTOR_SEQUENCE_H_ // NOLINT
#define DISRUPTOR_SEQUENCE_H_ // NOLINT

namespace disruptor {

const int64_t kInitialCursorValue = -1L;

// Sequence counter.
class Sequence {
 public:
    // Construct a sequence counter that can be tracked across threads.
    //
    // @param initial_value for the counter.
    Sequence(int64_t initial_value = kInitialCursorValue) :
            value_(initial_value) {}

    // Get the current value of the {@link Sequence}.
    //
    // @return the current value.
    int64_t sequence() const { return value_.load(std::memory_order::memory_order_acquire); }

    // Set the current value of the {@link Sequence}.
    //
    // @param the value to which the {@link Sequence} will be set.
    void set_sequence(int64_t value) { value_.store(value, std::memory_order::memory_order_release); }

    // Increment and return the value of the {@link Sequence}.
    //
    // @param increment the {@link Sequence}.
    // @return the new value incremented.
    int64_t IncrementAndGet(const int64_t& increment) {
        return value_.fetch_add(increment, std::memory_order::memory_order_release) + increment;
    }

 private:
    // members
    std::atomic<int64_t> value_;

    DISALLOW_COPY_AND_ASSIGN(Sequence);
};

// Cache line padded sequence counter.
//
// Can be used across threads without worrying about false sharing if a
// located adjacent to another counter in memory.
class PaddedSequence : public Sequence {
 public:
    PaddedSequence(int64_t initial_value = kInitialCursorValue) :
            Sequence(initial_value) {}

 private:
    // padding
    int64_t padding_[SEQUENCE_PADDING_LENGTH];

    DISALLOW_COPY_AND_ASSIGN(PaddedSequence);
};

// Cache line padded non-atomic sequence counter.
//
// This counter is not thread safe.
class PaddedLong {
 public:
     PaddedLong(int64_t initial_value = kInitialCursorValue) :
         sequence_(initial_value) {}

     int64_t sequence() const { return sequence_; }

     void set_sequence(const int64_t& sequence) { sequence_ = sequence; };

     int64_t IncrementAndGet(const int64_t& delta) { sequence_ += delta; return sequence_; }

 private:
     int64_t sequence_;
};

};  // namespace throughput

#endif // DISRUPTOR_SEQUENCE_H_ NOLINT
