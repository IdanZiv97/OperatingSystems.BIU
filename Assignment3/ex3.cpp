#include <Queues.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <thread>
#include <unistd.h>
using namespace std;
void producer(int maxArticles, int index);
void coEditor(UnboundedQueue* q);

vector<BoundedQueue*> producersQueues;
vector<int> maxArticles;
/**
 * dispatcherQueues:
 * index 0 - NEWS
 * index 1 - SPORTS
 * index 2 - WEATHER
 */
UnboundedQueue* dispatcherQueues[3] = {
    new UnboundedQueue(),
    new UnboundedQueue(),
    new UnboundedQueue(),
};

BoundedQueue* screenManagerQueue;

/**
 * @brief 
 * 
 * @param numOfArticles - number of articles it can produces
 * @param index - its index
 */
void producer(int maxArticles, int index) {
    int articlesProduced = 0, newsArticles = 0, sportsArticles = 0, weatherArticles = 0;
    int modulo;
    string category;
    while (articlesProduced < maxArticles) {
        //detrmine the category
        modulo = articlesProduced % 3;
        if (modulo == 0) {
            article a(index, "NEWS", newsArticles);
            producersQueues.at(index)->insert(a);
            newsArticles++;
            articlesProduced++;
        } else if (modulo == 1) {
            article a(index, "SPRORTS", sportsArticles);
            producersQueues.at(index)->insert(a);
            sportsArticles++;
            articlesProduced++;
        } else {
            article a(index, "WEATHER", weatherArticles);
            producersQueues.at(index)->insert(a);
            weatherArticles++;
            articlesProduced++;
        }
    }
    // push DONE
    article a(-1, "DONE", -1);
    producersQueues.at(index)->insert(a);
}

void dispatcher() {
    int numOfProducers = producersQueues.size();
    vector<bool> isDone;
    for (int i = 0; i < numOfProducers; i++) {
        isDone.push_back(false);
    }
    int numOfDoneProducers = 0;
    while (numOfDoneProducers < numOfProducers) {
        for (int i = 0; i < numOfProducers; i++) {
            if (!isDone.at(i)) {
                // pop the article
                article a = producersQueues.at(i)->remove();
                // check if done
                string category = a.category;
                if (category == "DONE") {
                    isDone.at(i) = true;
                    numOfDoneProducers++;
                } else if (category == "NEWS") {
                    dispatcherQueues[0]->push(a);
                } else if (category == "SPORTS") {
                    dispatcherQueues[1]->push(a);
                } else { // category == "WEATHER"
                    dispatcherQueues[2]->push(a);
                }
            }
        }
    }
    // now we are done with all the queues so we can push done to all dispatcher queues
    article done(-1, "DONE", -1);
    for (auto q : dispatcherQueues) {
        q->push(done);
    }
}

void coEditor(UnboundedQueue* q) {
    article a = q->remove();
    while (a.category != "DONE") {
        screenManagerQueue->insert(a);
        sleep(0.1);
    }
    screenManagerQueue->insert(a);
}

int main(int argc, char const *argv[]) {
    /**
     * code to read from input file
     * this code creates all the shared queues
    */
   fstream file(argv[1]);
   if (!file.is_open()) {
       cout << "failed to open file" << endl;
       return 1;
   }
   int coEditorSize;
   int producerMaxArticle;
   int sizeOfQueue;
   string line1, line2, line3;
   while (getline(file, line1)) {
       if (line1.empty()) {
           continue;
       }
       if (getline(file, line2) || getline(file, line3)) {
           coEditorSize = stoi(line1);
           break;
       }
       producerMaxArticle = stoi(line2);
       maxArticles.push_back(producerMaxArticle);
       sizeOfQueue = stoi(line3);
       producersQueues.push_back(new BoundedQueue(sizeOfQueue));
   }
   // creating producer threads
   for (int index = 0; index < maxArticles.size(); index++) {
       thread producerThread(producer, maxArticles.at(index), index);
   }
   // createing dispatcher thread
   thread dispatcherThread(dispatcher);
   // creating co editors threads
   for (auto q : dispatcherQueues) {
       thread coEditorThread(coEditor, q);
   }
    // screen manager code
    int numOfDones = 0;
    while(numOfDones != 3) {
        article a = screenManagerQueue->remove();
        if (a.category == "DONE") {
            numOfDones++;
            continue;
        }
        a.print();
    }
    cout << "DONE" << endl;
}


