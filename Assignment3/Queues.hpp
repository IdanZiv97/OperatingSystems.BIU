#ifndef QUEUES_HPP
#define QUEUES_HPP

#include <vector>
#include <queue>
#include <string>
#include <mutex>
#include <semaphore.h>
using namespace std;

struct article {
    int producer;
    string category;
    int index;
    article(int p, string c, int i) {
        producer = p;
        category = c;
        index = i + 1;
    }
    void print() {
        cout << "Producer " << producer << category << index << endl;
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
            sem_init(&_full, 1, 0);
            sem_init(&_empty, 1, _size);
        }
        void insert(article a) {
            // _full++; - if too full wait
            // lock mutex
            _container.push(a);
            // decrement empty - indiate there are less open spaces
            // unlock mutex
        }

        article remove() {
            // check if empty - wait until not
            // lock mutex
            article data = _container.front();
            _container.pop();
            // increase number of empty slots in queue
            // unlock mutex
            return data;
        }
        bool isEmpty() {
            return _container.empty();
        }

};

class UnboundedQueue {
    public:
        void push(article a);
        article remove();
};
#endif