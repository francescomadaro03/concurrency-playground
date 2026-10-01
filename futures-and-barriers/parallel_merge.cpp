#include <iostream>
#include <future>
#include <thread>
#include <mutex>
#include <vector>
#include <random>
#include <algorithm>

void build_array(std::promise<std::vector<int>> &p, int size){
    std::random_device dev;
    std::mt19937 mt(dev());
    std::uniform_int_distribution<int> g(0,10000);

    std::vector<int> helper;

    for(int i = 0; i< size; i++){
        helper.emplace_back(g(mt));
    }

    std::sort(helper.begin(), helper.end());

    p.set_value(helper);
    return;

}

std::vector<int> merge_sort(std::future<std::vector<int>> &f1, std::future<std::vector<int>> &f2){
    std::vector<int> h1 = f1.get();
    std::vector<int> h2 = f2.get();

    std::vector<int> sorted;

    int i = 0, j = 0;
    while(i<h1.size() && j<h2.size()){
        if(h1[i] > h2[j]){
            sorted.push_back(h2[j]);
            j++;
        }
        else {
            sorted.push_back(h1[i]);
            i++;
        }
    }

    while(i<h1.size()){
        sorted.push_back(h1[i]);
        i++;
    }

    while(j<h2.size()){
        sorted.push_back(h2[j]);
        j++;
    }

    for(auto &el : sorted){ std::cout << el << " ";}
    return sorted;
}

int main(int argc, char **argv){
    if(argc < 2){return 1;}
    const int N = std::stoi(argv[1]);

    std::promise<std::vector<int>> p1, p2;
    std::future<std::vector<int>> f1 = p1.get_future(), f2 = p2.get_future();

    std::thread t1(build_array, std::ref(p1), N),
    t2(build_array, std::ref(p2), N),
    t3(merge_sort, std::ref(f1), std::ref(f2));

    t1.join();
    t2.join();
    t3.join();



    return 0;
}