#include "Queues.hpp"
#include <fstream>
#include <thread>
#include <list>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

void producer(int maxArticles, int index);
void dispatcher();
void screenManager();
void coEditor(UnboundedQueue *category);

vector<BoundedQueue *> producersQueues;
vector<int> maxArticles;

/**
 * categories:
 * index 0 - NEWS queue
 * index 1 - SPORTS queue
 * index 2 - WEATHER queue
 */
UnboundedQueue *categories[3] = {
    new UnboundedQueue(),
    new UnboundedQueue(),
    new UnboundedQueue(),
};

BoundedQueue *broadcastReady;

list<thread> threadsPool;

int main(int argc, char const *argv[])
{
    // reading the config file
    fstream config(argv[1]);
    if (!config.is_open())
    {
        cout << "failed to open file" << endl;
        return 1;
    }
    int coEditorSize;
    int producerMaxArticle;
    int queueCapacity;
    string line1, line2, line3;
    while (getline(config, line1))
    {
        if (line1.empty() || line1 == " ")
        {
            continue;
        }
        if (!getline(config, line2) || !getline(config, line3))
        {
            coEditorSize = stoi(line1);
            broadcastReady = new BoundedQueue(coEditorSize);
            break;
        }
        producerMaxArticle = stoi(line2);
        maxArticles.push_back(producerMaxArticle);
        queueCapacity = stoi(line3);
        producersQueues.push_back(new BoundedQueue(queueCapacity));
    }
    // creating producer threads
    for (int index = 0; index < maxArticles.size(); index++)
    {
        std::thread producerThread(producer, maxArticles.at(index), index);
        threadsPool.push_back(move(producerThread));
    }
    // createing dispatcher thread
    std::thread dispatcherThread(dispatcher);
    threadsPool.push_back(move(dispatcherThread));
    // creating co editors threads
    for (auto category : categories)
    {
        thread coEditorThread(coEditor, category);
        threadsPool.push_back(move(coEditorThread));
    }
    // screen manager code
    thread screenManagerThread(screenManager);
    threadsPool.push_back(move(screenManagerThread));
    // join threads from the last thread to the first
    list<thread>::reverse_iterator currentThread = threadsPool.rbegin();
    while (currentThread != threadsPool.rend())
    {
        currentThread->join();
        currentThread++;
    }
    cout << "DONE" << endl;
    return 0;
}

/**
 * @brief producer behaviour:
 * Creates articles from 3 categories: NEWS, SPORTS, WEATHER
 * @param numOfArticles - number of articles it can produces
 * @param index - its index
 */
void producer(int maxArticles, int index)
{
    int articlesProduced = 0, newsArticles = 0, sportsArticles = 0, weatherArticles = 0;
    srand(time(NULL));
    int randomChoice;
    string category;
    while (articlesProduced < maxArticles)
    {
        // detrmine the category
        randomChoice = rand() % 3 + 1;
        if (randomChoice == 1)
        {
            article a(index, "NEWS", newsArticles);
            producersQueues.at(index)->insert(a);
            newsArticles++;
            articlesProduced++;
        }
        else if (randomChoice == 2)
        {
            article a(index, "SPRORTS", sportsArticles);
            producersQueues.at(index)->insert(a);
            sportsArticles++;
            articlesProduced++;
        }
        else // randomChoice == 3
        {
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
/**
 * @brief The dispatcher is responsible of sorting the articles produced by each producer to the
 * appropriate queue, according to the article's category.
 * The dispatcher iterates the producers queues in a "round robin" manner:
 * As long as the queue contains articles, the dispatcher will remove it.
 * When the queue is empty the dispatcher will proceed to the next valid queue.
 */
void dispatcher()
{
    int numOfProducers = producersQueues.size();
    vector<bool> isDone;
    for (int i = 0; i < numOfProducers; i++)
    {
        isDone.push_back(false);
    }
    int numOfDoneProducers = 0;
    while (numOfDoneProducers < numOfProducers)
    {
        for (int i = 0; i < numOfProducers; i++)
        {
            if (!isDone.at(i))
            {
                    // pop the article
                    article a = producersQueues.at(i)->remove();
                    // check if done
                    string category = a.category;
                    if (category == "DONE")
                    {
                        isDone.at(i) = true;
                        numOfDoneProducers++;
                    }
                    else if (category == "NEWS")
                    {
                        categories[0]->push(a);
                    }
                    else if (category == "SPORTS")
                    {
                        categories[1]->push(a);
                    }
                    else // category == "WEATHER"
                    { 
                        categories[2]->push(a);
                    }
            }
        }
    }
    // now we are done with all the queues so we can push done to all dispatcher queues
    article done(-1, "DONE", -1);
    for (auto q : categories)
    {
        q->push(done);
    }
}

void coEditor(UnboundedQueue *category)
{
    article a = category->remove();
    while (a.category != "DONE")
    {
        broadcastReady->insert(a);
        sleep(0.1);
        a = category->remove();
    }
    broadcastReady->insert(a);
}

void screenManager()
{
    int numOfDones = 0;
    while (numOfDones != 3)
    {
        article a = broadcastReady->remove();
        if (a.category == "DONE")
        {
            numOfDones++;
            continue;
        }
        a.broadcastArticle();
    }
}