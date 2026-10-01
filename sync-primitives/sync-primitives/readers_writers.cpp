

#include <condition_variable>
#include <mutex>
#include <random>
#include <iostream>
#include <thread>


int random_temperature(){
    std::random_device dev;
    std::mt19937 mm(dev());
    std::uniform_int_distribution<int> d(1, 35);
    return d(mm);
}


/*
    I need a way to handle multiple reads synchronous whereas readers can read synchronously
    and when the writer comes, it stops readers

*/

std::mutex console;

class SharedData {
    private:
        int readers = 0;
        int writers = 0;
        int data=0;
        std::mutex mwrite;
        std::condition_variable wait_write;
        std::condition_variable wait_read;
    
    public:
        SharedData() {};
        int read(){

            {
                std::unique_lock<std::mutex> lock(mwrite);
                wait_read.wait(lock, [this]{return this->writers == 0 && this->data != 0;});
                readers++;
    
            }

            {
                std::lock_guard<std::mutex> llock(mwrite);
                readers--;
            }
            wait_write.notify_one();
            return data;

        }

        void write(int in){

            std::unique_lock<std::mutex> lock(mwrite);
            writers++;
            wait_write.wait(lock, [this]{return this->readers == 0;});
            data = in;
            writers--;
            wait_read.notify_all();

        }
};


int producer(SharedData &ref){
    while(true){
        std::cout << "In the producer..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        ref.write(random_temperature());
    }
}

int consumer(SharedData &ref, int id){
    while(true){
        int v = ref.read();
        std::lock_guard<std::mutex> lock(console);
        std::cout << "Consumer id: " << id << "produced: " << v << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
}

int main(){
    SharedData sd;
    std::thread t1(producer, std::ref(sd)), t2(consumer, std::ref(sd), 1), t3(consumer,std::ref(sd), 2), t4(consumer,std::ref(sd),1);
    t1.join();
    t2.join();
    t3.join();
    t4.join();

    return 0;
}
