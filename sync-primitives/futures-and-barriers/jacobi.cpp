/*

xi all'iterazione k+1 => 1/Aii(bi- sum(Aij * xk)) dove j!= i e compreso tra 0 e len A


*/

#include <vector>
#include <thread>
#include <mutex>
#include <future>
#include <condition_variable>

std::mutex val;
std::condition_variable cv;

int count = 0;



void jacobi (
    std::vector<std::vector<double>>& A,
    std::vector<double>& b,
    std::vector<double>& x,
    std::vector<double>& x_new,
    int n
){
    /*
        In my implementation, the int n value is the index of the xi variable
    */

    double Aii = A[n][n];
    double bi = b[n];
    double sum = 0;

    for(int j = 0; j<A[n].size(); j++){
        if(j == n){
            continue;
        }
        sum += A[n][j]*x[j];
    }

    sum = bi-sum;
    sum = sum/Aii;
    x_new[n] = sum;

    {
        std::unique_lock<std::mutex> lock(val);
        count++;
        if(count == A.size()){
            cv.notify_all();
        }
    }

    return;
}

int main(int argc, char **argv){
    if(argc < 2){
        exit(1);
    }
    int it = std::stoi(argv[1]);

    std::vector<std::vector<double>> A = {
        {10.0,  2.0,  1.0},
        { 1.0,  5.0,  1.0},
        { 2.0,  3.0, 10.0}
    };
    std::vector<double> b = {7.0, -8.0, 6.0};
    std::vector<double> x = {0.0, 0.0, 0.0};
    std::vector<double> x_new(3, 0);

    while(it){

        std::vector<std::thread> P;

        for(int i = 0; i<A.size(); i++){
            P.emplace_back(jacobi, std::ref(A), std::ref(b), std::ref(x), std::ref(x_new), i);
        }

        std::unique_lock<std::mutex> lock(val);
        cv.wait(lock, [&A]{return count == A.size();});
        count = 0;
        x = x_new;

        for(int i = 0; i< P.size(); i++){
            P[i].join();
        }

        it--;
    }
}