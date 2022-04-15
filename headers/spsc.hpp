// A simple single-producer single-consumer thread-safe message queue

#ifndef SPSC_INCLUDED
#define SPSC_INCLUDED

#include <vector>
#include <mutex>
#include <condition_variable>
#include <cassert>

using namespace std::literals::chrono_literals;

using std::unique_lock;
using std::condition_variable;
using std::vector;

template <typename T>
class Spsc_queue {
private:
    std::mutex mux;
    unique_lock<std::mutex> producer_lock;
    unique_lock<std::mutex> consumer_lock;
    condition_variable cond;
    vector<T> storage;
    size_t first_valid;
public:
    Spsc_queue(size_t capacity);

    void produce(T elem);
    T consume(void);
};

// class impl

template <typename T>
Spsc_queue<T>::Spsc_queue(size_t capacity) {
    this->producer_lock = unique_lock(this->mux, std::defer_lock);
    this->consumer_lock = unique_lock(this->mux, std::defer_lock);
    this->storage = vector<T>();
    this->storage.reserve(capacity);
    this->first_valid = 0;
}

template <typename T>
bool space_avail(vector<T>& v, size_t first) {
    return v.size() < v.capacity();
}

template <typename T>
void Spsc_queue<T>::produce(T elem) {
    //std::this_thread::sleep_for(100ms);

    this->producer_lock.lock();

    // wait until there's storage available
    while(!space_avail(this->storage, this->first_valid)) {
        this->cond.wait(this->producer_lock);

    }
    this->storage.push_back(elem);


    this->producer_lock.unlock();
    this->cond.notify_all();
}

template <typename T>
bool elems_avail(size_t first, vector<T>& v) {
    assert(first >= 0);
    return first < v.size();
}

template <typename T>
T Spsc_queue<T>::consume(void) {
    this->consumer_lock.lock();

    // wait until there are elements in the storage
    while(!elems_avail(this->first_valid, this->storage)) {
        this->cond.wait(this->consumer_lock);

    }
    // The first valid element is extracted and the next is marked as invalid
    T el = this->storage[this->first_valid++];

    // when all elements in the storage are invalid, clear it
    if (this->first_valid == this->storage.size()) {
        this->storage.clear();
        this->first_valid = 0;
    }

    this->consumer_lock.unlock();
    this->cond.notify_all();

    return el;
}

#endif