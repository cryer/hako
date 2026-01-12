#include <iostream>
#include <thread>
#include <vector>
#include <cassert>
#include <chrono>

#include "arena.hpp"

void test_single_thread() {
    std::cout << "=== Single-thread test ===\n";
    Arena arena(1024);

    int* p1 = static_cast<int*>(arena.allocate(sizeof(int), alignof(int)));
    double* p2 = static_cast<double*>(arena.allocate(sizeof(double), alignof(double)));
    char* p3 = static_cast<char*>(arena.allocate(100, 1));

    *p1 = 42;
    *p2 = 3.14;
    for (int i = 0; i < 100; ++i) p3[i] = static_cast<char>(i);

    assert(*p1 == 42);
    assert(*p2 == 3.14);
    assert(p3[50] == 50);

    std::cout << "Used: " << arena.used() << " bytes\n";
    std::cout << "Capacity: " << arena.capacity() << " bytes\n";

    arena.reset();
    assert(arena.used() == 0);
    std::cout << "Single-thread test passed.\n\n";
}

void test_thread_safe_arena() {
    std::cout << "=== Multi-thread test ===\n";
    ThreadSafeArena arena(4096);

    const int num_threads = 4;
    const int allocs_per_thread = 1000;
    std::vector<std::thread> threads;

    auto worker = [&](int id) {
        for (int i = 0; i < allocs_per_thread; ++i) {
            size_t size = (i % 100) + 1;
            void* ptr = arena.allocate(size, 8);
            static_cast<char*>(ptr)[0] = static_cast<char>(id);
            static_cast<char*>(ptr)[size - 1] = static_cast<char>(i);
        }
    };

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker, i);
    }

    for (auto& t : threads) {
        t.join();
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Allocated " << num_threads * allocs_per_thread << " blocks in "
              << duration.count() << " ms\n";
    std::cout << "Total used: " << arena.used() << " bytes\n";
    std::cout << "Capacity: " << arena.capacity() << " bytes\n";
    std::cout << "Multi-thread test passed.\n\n";
}

int main() {
    test_single_thread();
    test_thread_safe_arena();
    std::cout << "All tests passed!\n";
    return 0;
}