#ifndef QUEUES_HPP
#define QUEUES_HPP

#include <vector>
#include <iostream>
#include <queue>
#include <string>
#include <mutex>
#include "semaphore.h"
using namespace std;

struct article {
    int producer;
    string category;
    int index;
    article(int p, string c, int i) {
        producer = p + 1;
        category = c;
        index = i;
    }
    void print() {
        cout << "Producer " << producer << " " << category << " " << index << endl;
    }
};

class BoundedQueue {
    private:
        queue<article> _container;
        int _size;
        std::mutex _occupied;
        sem_t _full, _empty;
    public:
        BoundedQueue(int s): _size(s) {
            sem_init((&this->_full), 0, 0);
            sem_init((&this->_empty), 0, 4);
        }
        void insert(article a) {
            // check if queue is empty
            sem_wait(&_empty);
            _occupied.lock();
            _container.push(a);
            _occupied.unlock();
            sem_post(&_full);
        }

        article remove() {
            sem_wait(&_full);
            _occupied.lock();
            article data = _container.front();
            _container.pop();
            _occupied.unlock();
            sem_post(&_empty);
            return data;
        }

        bool isEmpty() {
            return _container.empty();
        }
};

class UnboundedQueue {
    private:
        queue<article> _container;
        mutex _occupied;
        sem_t _full;
    public:
        UnboundedQueue() {
            sem_init((&this->_full), 0, 0);
        }
        void push(article a) {
            _occupied.lock();
            _container.push(a);
            _occupied.unlock();
            sem_post(&_full);
        }
        article remove() {
            sem_wait(&_full);
            _occupied.lock();
            article data = _container.front();
            _container.pop();
            _occupied.unlock();
            return data;
        }
        bool isEmpty() {
            return _container.empty();
        }
};
#endif