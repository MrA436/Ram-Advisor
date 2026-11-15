#pragma once
#include "process.h"
#include <iostream>
#include <string>
class CustomMaxHeap {
private:
    process heap[256];
    int current_size;
    int parent(int i) { return (i - 1) / 2; }
    int left_child(int i) { return (2 * i) + 1; }
    int right_child(int i) { return (2 * i) + 2; }
    void swap_nodes(int i, int j) {
        process temp = heap[i];
        heap[i] = heap[j];
        heap[j] = temp;
    }
    void heapify_up(int i) {
        while (i > 0 && heap[parent(i)].score < heap[i].score) {
            swap_nodes(i, parent(i));
            i = parent(i);
        }
    }
    void heapify_down(int i) {
        int max_index = i;
        int l = left_child(i);
        int r = right_child(i);
        if (l < current_size && heap[l].score > heap[max_index].score)
            max_index = l;
        if (r < current_size && heap[r].score > heap[max_index].score)
            max_index = r;
        if (i != max_index) {
            swap_nodes(i, max_index);
            heapify_down(max_index);
        }
    }
public:
    CustomMaxHeap() {
        current_size = 0;
    }
    bool is_empty() const {
        return current_size == 0;
    }
    int size() const {
        return current_size;
    }
    void insert(const process& p) {
        if (current_size >= 256) return;
        heap[current_size] = p;
        heapify_up(current_size);
        current_size++;
    }
    bool extract_max(process& out) {
        if (current_size <= 0) return false;
        out = heap[0];
        heap[0] = heap[current_size - 1];
        current_size--;
        heapify_down(0);
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
};
class CustomHashTable {
private:
    struct HashNode {
        int pid;
        HashNode* next;
        HashNode(int p) : pid(p), next(nullptr) {}
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
        if (L[i].score >= R[j].score) {
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
    if (left < right) {
        int mid = left + (right - left) / 2;
        custom_merge_sort(arr, left, mid);
        custom_merge_sort(arr, mid + 1, right);
        merge_subarrays(arr, left, mid, right);
    }
}
