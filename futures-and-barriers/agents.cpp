#include <iostream>
#include <thread>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <mutex>
#include <random>
#include <set>
#include <barrier>

std::mutex matrix;
std::set<long long> ar;



std::vector<std::vector<long long>> read_and_populate_matrix(const std::string &fname){
    
    std::ifstream file(fname);
    std::string line;
    if(!file.is_open()){
        throw std::runtime_error("No file was opened!");
    }

    int row, col;
    getline(file, line);
    std::stringstream iss(line);
    iss >> row;
    iss >> col;

    std::vector<std::vector<long long>> m(row, std::vector<long long>(col, 0));

    int i = 0, j= 0;

    while(i<row && getline(file, line)){
        std::stringstream rline(line);
        long long value;
        while(j<col && rline >> value){
            m[i][j] = value;
            j++;
        }
        j = 0;
        i++;
    }

    return m;

}


int random_value(){
    thread_local std::random_device dev;
    thread_local std::mt19937 mt(dev());
    thread_local std::uniform_int_distribution<int> d(-1, 1); 

    int v = d(mt);
    return v;
}

long long atomic_compare_and_swap(std::vector<std::vector<long long>> &m, int row, int col, long long value){
    std::unique_lock<std::mutex> lock(matrix);
    long long temp = m[row][col];
    if(temp != 0){return -1;}
    m[row][col] = value;
    ar.insert(value);
    return temp;

}


template <typename BarrierType>
void random_walk(std::vector<std::vector<long long>> &m, BarrierType &s, int row_start, int row_end){

    /*
        Here I simply scan the array in mutual exclusion (someone could write while I am reading)
        then i compute the next address. I must place the agent in the 8 nearest cells, meaning
        I am required to compute the 3*3 square around the item, closely looking for boundaries
    */

    while(true){
        long long read;
        



        for(int i = row_start; i<row_end; i++){ //outer loop to start scanning the array. 
            for(int j = 0; j<m[i].size(); j++){
                std::unique_lock<std::mutex> lock(matrix);
                read = m[i][j];
                
                if(read == 0 || ar.contains(read)){continue;} //nothing to do, I start from another value
                lock.unlock(); // I unlock to let other read/write on the matrix without owning it too much time

                bool ok = false;
                
                int offset_row = random_value(), offset_col = random_value();

                while(!ok){
                    if(i+offset_row < 0 || i+offset_row >= m.size() || j+offset_col < 0 || j+offset_col >= m[i].size()){
                        offset_row = random_value();
                        offset_col = random_value();
                    }
                    else {ok = true;}
                }

                
                long long old = atomic_compare_and_swap(m, (i+offset_row), (j+offset_col), read);
                if(old == -1){
                    continue;
                }
                lock.lock();
                m[i][j] = 0;
                lock.unlock(); 
                /*ambiguity in the text: i suppose that if i do not have place, i won't move neither agent*/
            
            }

        }
        s.arrive_and_wait();
    }
   



    return;
}



int main(int argc, char **argv){
    if(argc < 2){
        return 1;
    }

    auto m = read_and_populate_matrix(argv[1]);

    int nthreads = std::thread::hardware_concurrency();
    if(nthreads == 0){
        return 2;
    }
    auto r = []() noexcept { ar.clear();};
    std::barrier sync(nthreads, r);
    std::vector<std::thread> vt;
    int vec_size = (int) m.size();
    for(int i = 0; i<nthreads; i++){
        int start = (vec_size)*i / nthreads;
        int end = ((i+1)*vec_size) / nthreads;
        vt.emplace_back(random_walk<decltype(sync)>, std::ref(m), std::ref(sync), start, end);

    }

    for(auto &t : vt){
        t.join();
    }
    return 0;
}