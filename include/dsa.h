#pragma once
#include "process.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <utility>

class CustomMaxHeap {
private:
    process* heap;
    int capacity;
    int current_size;

    int parent(int i) const { return (i - 1) / 2; }
    int left_child(int i) const { return (2 * i) + 1; }
    int right_child(int i) const { return (2 * i) + 2; }

    bool is_higher_priority(const process& a, const process& b) const {
        if (a.score != b.score) {
            return a.score > b.score;
        }
        return a.ram > b.ram;
    }

    void swap_nodes(int i, int j) {
        process temp = heap[i];
        heap[i] = heap[j];
        heap[j] = temp;
    }

    void resize(int new_capacity) {
        process* new_heap = new process[new_capacity];
        for (int i = 0; i < current_size; i++) {
            new_heap[i] = heap[i];
        }
        delete[] heap;
        heap = new_heap;
        capacity = new_capacity;
    }

    void heapify_up(int i) {
        while (i > 0 && is_higher_priority(heap[i], heap[parent(i)])) {
            swap_nodes(i, parent(i));
            i = parent(i);
        }
    }

    void heapify_down(int i) {
        int max_index = i;
        int l = left_child(i);
        int r = right_child(i);

        if (l < current_size && is_higher_priority(heap[l], heap[max_index])) {
            max_index = l;
        }
        if (r < current_size && is_higher_priority(heap[r], heap[max_index])) {
            max_index = r;
        }

        if (i != max_index) {
            swap_nodes(i, max_index);
            heapify_down(max_index);
        }
    }

public:
    explicit CustomMaxHeap(int initial_capacity = 256) {
        capacity = (initial_capacity < 16) ? 16 : initial_capacity;
        current_size = 0;
        heap = new process[capacity];
    }

    ~CustomMaxHeap() {
        delete[] heap;
    }

    CustomMaxHeap(const CustomMaxHeap& other) {
        capacity = other.capacity;
        current_size = other.current_size;
        heap = new process[capacity];
        for (int i = 0; i < current_size; i++) {
            heap[i] = other.heap[i];
        }
    }

    CustomMaxHeap& operator=(const CustomMaxHeap& other) {
        if (this != &other) {
            delete[] heap;
            capacity = other.capacity;
            current_size = other.current_size;
            heap = new process[capacity];
            for (int i = 0; i < current_size; i++) {
                heap[i] = other.heap[i];
            }
        }
        return *this;
    }

    CustomMaxHeap(CustomMaxHeap&& other) noexcept 
        : heap(other.heap), capacity(other.capacity), current_size(other.current_size) {
        other.heap = nullptr;
        other.capacity = 0;
        other.current_size = 0;
    }

    CustomMaxHeap& operator=(CustomMaxHeap&& other) noexcept {
        if (this != &other) {
            delete[] heap;
            heap = other.heap;
            capacity = other.capacity;
            current_size = other.current_size;
            other.heap = nullptr;
            other.capacity = 0;
            other.current_size = 0;
        }
        return *this;
    }

    bool is_empty() const {
        return current_size == 0;
    }

    int size() const {
        return current_size;
    }

    void insert(const process& p) {
        if (current_size >= capacity) {
            resize(capacity * 2);
        }
        heap[current_size] = p;
        heapify_up(current_size);
        current_size++;
    }

    bool extract_max(process& out) {
        if (current_size <= 0) return false;
        out = heap[0];
        heap[0] = heap[current_size - 1];
        current_size--;
        if (current_size > 0) {
            heapify_down(0);
        }
        return true;
    }

    bool peek(process& out) const {
        if (current_size <= 0) return false;
        out = heap[0];
        return true;
    }

    void clear() {
        current_size = 0;
    }
};

class CustomCircularQueue {
private:
    static const int CAPACITY = 10;
    double buffer[CAPACITY];
    int front_idx;
    int rear_idx;
    int count;

public:
    CustomCircularQueue() {
        front_idx = 0;
        rear_idx = -1;
        count = 0;
        for (int i = 0; i < CAPACITY; i++) buffer[i] = 0.0;
    }

    void enqueue(double val) {
        if (count == CAPACITY) {
            front_idx = (front_idx + 1) % CAPACITY;
            rear_idx = (rear_idx + 1) % CAPACITY;
            buffer[rear_idx] = val;
        } else {
            rear_idx = (rear_idx + 1) % CAPACITY;
            buffer[rear_idx] = val;
            count++;
        }
    }

    int size() const {
        return count;
    }

    bool is_empty() const {
        return count == 0;
    }

    double get_at(int logical_index) const {
        if (logical_index < 0 || logical_index >= count) return 0.0;
        int actual_idx = (front_idx + logical_index) % CAPACITY;
        return buffer[actual_idx];
    }

    void clear() {
        front_idx = 0;
        rear_idx = -1;
        count = 0;
    }
};

class CustomHashTable {
private:
    struct HashNode {
        int pid;
        HashNode* next;
        explicit HashNode(int p) : pid(p), next(nullptr) {}
    };

    static const int TABLE_SIZE = 37;
    HashNode* buckets[TABLE_SIZE];

    int hash_function(int pid) const {
        int val = pid % TABLE_SIZE;
        return val < 0 ? val + TABLE_SIZE : val;
    }

public:
    CustomHashTable() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            buckets[i] = nullptr;
        }
    }

    ~CustomHashTable() {
        clear();
    }

    CustomHashTable(const CustomHashTable& other) {
        for (int i = 0; i < TABLE_SIZE; i++) buckets[i] = nullptr;
        for (int i = 0; i < TABLE_SIZE; i++) {
            HashNode* curr = other.buckets[i];
            HashNode** dest = &buckets[i];
            while (curr != nullptr) {
                *dest = new HashNode(curr->pid);
                dest = &((*dest)->next);
                curr = curr->next;
            }
        }
    }

    CustomHashTable& operator=(const CustomHashTable& other) {
        if (this != &other) {
            clear();
            for (int i = 0; i < TABLE_SIZE; i++) {
                HashNode* curr = other.buckets[i];
                HashNode** dest = &buckets[i];
                while (curr != nullptr) {
                    *dest = new HashNode(curr->pid);
                    dest = &((*dest)->next);
                    curr = curr->next;
                }
            }
        }
        return *this;
    }

    CustomHashTable(CustomHashTable&& other) noexcept {
        for (int i = 0; i < TABLE_SIZE; i++) {
            buckets[i] = other.buckets[i];
            other.buckets[i] = nullptr;
        }
    }

    CustomHashTable& operator=(CustomHashTable&& other) noexcept {
        if (this != &other) {
            clear();
            for (int i = 0; i < TABLE_SIZE; i++) {
                buckets[i] = other.buckets[i];
                other.buckets[i] = nullptr;
            }
        }
        return *this;
    }

    void insert(int pid) {
        if (contains(pid)) return;
        int idx = hash_function(pid);
        HashNode* new_node = new HashNode(pid);
        new_node->next = buckets[idx];
        buckets[idx] = new_node;
    }

    bool contains(int pid) const {
        int idx = hash_function(pid);
        HashNode* curr = buckets[idx];
        while (curr != nullptr) {
            if (curr->pid == pid) return true;
            curr = curr->next;
        }
        return false;
    }

    void clear() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            HashNode* curr = buckets[i];
            while (curr != nullptr) {
                HashNode* temp = curr;
                curr = curr->next;
                delete temp;
            }
            buckets[i] = nullptr;
        }
    }
};

inline void merge_subarrays(process arr[], int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    process* L = new process[n1];
    process* R = new process[n2];

    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;

    while (i < n1 && j < n2) {
        bool take_left = false;
        if (L[i].score > R[j].score) {
            take_left = true;
        } else if (L[i].score == R[j].score) {
            take_left = (L[i].ram >= R[j].ram);
        }

        if (take_left) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }

    delete[] L;
    delete[] R;
}

inline void custom_merge_sort(process arr[], int left, int right) {
    if (!arr) return;
    if (left < right) {
        int mid = left + (right - left) / 2;
        custom_merge_sort(arr, left, mid);
        custom_merge_sort(arr, mid + 1, right);
        merge_subarrays(arr, left, mid, right);
    }
}
