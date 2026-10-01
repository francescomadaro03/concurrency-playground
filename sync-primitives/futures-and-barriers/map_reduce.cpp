#include <iostream>
#include <thread>
#include <barrier>
#include <random>
#include <mutex>
#include <vector>

#define LEN 10000

int global_sum;



std::random_device dev;
std::mt19937 mm(dev());
std::uniform_int_distribution<int> d(0,1000);


int generate_random(){
    return d(mm);
}

template <typename Operation>
void worker(std::barrier<Operation> &b, std::vector<int>&vec, int &sum, int start, int end){
    for(int i = start; i <end; i++){
        (vec[i] % 2) ? sum++ : 0;
    }
    b.arrive_and_wait();
    return;
};

int main(int argc, char **argv){
    if(argc < 2){ return 1;}

    const int N = std::stoi(argv[1]);
    std::vector<int> v(LEN);
    std::vector<int> partial_sums(N);
    std::vector<std::thread> vec_t;

    for(auto &el : v){ el = generate_random(); }

    auto completion = [&partial_sums]() noexcept {

        for(auto &sum : partial_sums){
            global_sum+=sum;
        }
    };

    std::barrier b(N, completion);
    for(int i = 0; i < N; i++){
        int start = (i*LEN) / N;
        int end = ((i+1)*LEN) / N;
        vec_t.emplace_back(worker<decltype(completion)>, std::ref(b), std::ref(v), std::ref(partial_sums[i]), start, end);

    }

    for(auto &t : vec_t){
        t.join();
    }

    return;

    
    


}