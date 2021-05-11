// LockFree.cpp : 定义控制台应用程序的入口点。
//

#include <boost/thread/thread.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <iostream>

#include <boost/atomic.hpp>

int producer_count = 0;
boost::atomic_int consumer_count (0);

boost::lockfree::spsc_queue<int, boost::lockfree::capacity<1024> > spsc_queue;

const int iterations = 10000000;

void producer(void)
{
    int count = 0;
    for (int i = 0; i != iterations; ++i) {
        int value = ++producer_count;
        while (!spsc_queue.push(value)) //队列满则等待。
            ++count;
    }
    std::cout << "full " << count << " times\n";
}

boost::atomic<bool> done (false);

void consumer(void)
{
    int value;
    while (!done) { //生产者还在生产
        while (spsc_queue.pop(value)) //如果有产品就消费
            ++consumer_count;
    }

    while (spsc_queue.pop(value)) //生产者停工
        ++consumer_count;
}

int main(int argc, char* argv[])
{
    using namespace std;
    cout << "boost::lockfree::queue is ";
    if (!spsc_queue.is_lock_free())
        cout << "not ";
    cout << "lockfree" << endl;

    boost::thread producer_thread(producer);
    boost::thread consumer_thread(consumer);

    producer_thread.join();
    done = true;
    consumer_thread.join();

    cout << "produced " << producer_count << " objects." << endl;
    cout << "consumed " << consumer_count << " objects." << endl;
}