#include <thread>
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <random>
#include <algorithm>

std::mutex console;

class Barrier {
    private:
        int n;
        int waiting = 0;
        int generation = 0;
        std::condition_variable cv;
        std::mutex m;
    public:
        Barrier(int nthreads) : n(nthreads) {};
        void wait(){
            std::unique_lock<std::mutex> lock(m);
            int current_generation = generation;
            waiting++;

            if(n == waiting){
                waiting = 0;
                generation++;
                lock.unlock();
                cv.notify_all();
            }
            else {
                cv.wait(lock, [this, current_generation](){return generation != current_generation;});
            }

        }
};


std::vector<int> create_random_array(int size){
    std::vector<int> v;
    std::random_device dev;
    std::mt19937 mt(dev());
    std::uniform_int_distribution<int> d(0,100);

    for(int i = 0; i< size; i++){
        v.emplace_back(d(mt));
    }

    return v;
}

void sort_with_condition(std::vector<int> &v, bool cond){

    (cond == true) ? std::sort(v.begin(), v.end()) : std::sort(v.begin(), v.end(), [](int a, int b){return b < a;});
}

int check_order_sorted(std::vector<int> &v, bool cond){

    if(cond){
        for(int i = 0; i<v.size()-1; i++){
            if(v[i+1]< v[i]){
                return 0;
            }
        }

        return 1;
    }
    else {
        for(int i = 0; i<v.size()-1; i++){
            if(v[i+1] > v[i]){
                return 0;
            }
        }
        return 1;
    }

}

void t_exe(Barrier &b, int size){
    auto v = create_random_array(size);
    
    {   
        std::lock_guard<std::mutex> lock(console);
        std::cout << "Thread id: " << std::this_thread::get_id() << " " << "Waiting on barrier stage 1.... "<< std::endl;
    }

    b.wait(); // barrier to wait first stage

    std::random_device dev;
    std::mt19937 t(dev());
    std::uniform_int_distribution<int> d(0,1);

    int cond = d(t);

    sort_with_condition(std::ref(v), cond);

    {   
        std::lock_guard<std::mutex> lock(console);
        std::cout << "Thread id: " << std::this_thread::get_id()<< " "  << "Waiting on barrier stage 2.... "<< std::endl;
    }
    b.wait(); //

    bool is_sorted = check_order_sorted(std::ref(v), cond);

    std::lock_guard<std::mutex> lock(console);
    std::cout << "Thread id: " << std::this_thread::get_id()<< " "  << "Correctly sorted: " << is_sorted << std::endl;

    return;


}

int main(int argc, char **argv){
    if(argc < 3){return 1;}
    int n_threads = std::stoi(argv[1]);
    int size_v = std::stoi(argv[2]);

    Barrier b(n_threads);
    std::vector<std::thread> v_threads;
    for(int i = 0; i<n_threads; i++){
        v_threads.emplace_back(t_exe, std::ref(b), size_v);
    }

    for(auto &t : v_threads){
        t.join();
    }

    return 0;
}