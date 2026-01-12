#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <vector>
#include <mutex>
#include <cassert>
#include <algorithm>

static constexpr bool is_power_of_two(size_t x) {
    return x != 0 && (x & (x - 1)) == 0;
}

static inline bool check_add_overflow(uintptr_t a, size_t b, uintptr_t* result) {
    if (b > SIZE_MAX - a) {
        return false; 
    }
    *result = a + static_cast<uintptr_t>(b);
    return true;
}

// nolock policy
struct NoLock {
    void lock() {}
    void unlock() {}
};

//lock policy
struct MutexLock {
    void lock() { mtx.lock(); }
    void unlock() { mtx.unlock(); }
private:
    std::mutex mtx;
};

// Arena 
template<typename LockPolicy = NoLock>
class BasicArena {
public:
    explicit BasicArena(size_t block_size = 4096)
        : block_size_(std::max(block_size, static_cast<size_t>(1))),
          ptr_(nullptr),
          end_(nullptr),
          used_bytes_(0) {
        allocateBlock(); 
    }

    ~BasicArena() {
        for (auto* block : blocks_) {
            delete[] block;
        }
        ptr_ = nullptr;
        end_ = nullptr;
        blocks_.clear();
    }

    BasicArena(const BasicArena&) = delete;
    BasicArena& operator=(const BasicArena&) = delete;

    BasicArena(BasicArena&& other) noexcept
        : block_size_(other.block_size_),
          ptr_(other.ptr_),
          end_(other.end_),
          used_bytes_(other.used_bytes_),
          blocks_(std::move(other.blocks_)),
          lock_() {
      
        other.ptr_ = nullptr;
        other.end_ = nullptr;
        other.used_bytes_ = 0;
    }

    BasicArena& operator=(BasicArena&& other) noexcept {
        if (this != &other) {
            for (auto* block : blocks_) {
                delete[] block;
            }
            block_size_ = other.block_size_;
            ptr_ = other.ptr_;
            end_ = other.end_;
            used_bytes_ = other.used_bytes_;
            blocks_ = std::move(other.blocks_);

            other.ptr_ = nullptr;
            other.end_ = nullptr;
            other.used_bytes_ = 0;
        }
        return *this;
    }

    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        assert(alignment >= 1 && "alignment must be at least 1");
        assert(is_power_of_two(alignment) && "alignment must be a power of two");

        lock_.lock();

        char* current_ptr = ptr_;
        // to left-value
        size_t space = static_cast<size_t>(end_ - ptr_); 


        void* aligned_ptr = std::align(alignment, size,
                                       reinterpret_cast<void*&>(current_ptr),
                                       space);
        if (!aligned_ptr) {
            allocateBlock();
            current_ptr = ptr_;
            space = block_size_; 
            aligned_ptr = std::align(alignment, size,
                                     reinterpret_cast<void*&>(current_ptr),
                                     space);

            assert(aligned_ptr && "Allocation failed even after new block");
        }

        ptr_ = current_ptr + size;
        used_bytes_ += size;

        lock_.unlock();
        return aligned_ptr;
    }

    void reset() {
        lock_.lock();
        if (!blocks_.empty()) {
            ptr_ = blocks_.front();          
            end_ = ptr_ + block_size_;       
            used_bytes_ = 0;               
        }
        lock_.unlock();
    }

    size_t used() const {
        return used_bytes_;
    }

    size_t capacity() const {
        return blocks_.size() * block_size_;
    }

private:
    void allocateBlock() {
        char* new_block = new char[block_size_];
        blocks_.push_back(new_block);
        ptr_ = new_block;
        end_ = new_block + block_size_;
    }

    size_t block_size_;             
    char* ptr_;                     
    char* end_;                     
    size_t used_bytes_;             
    std::vector<char*> blocks_;      
    mutable LockPolicy lock_;      
};


using Arena = BasicArena<NoLock>;
using ThreadSafeArena = BasicArena<MutexLock>;
