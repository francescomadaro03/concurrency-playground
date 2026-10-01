/*
Lo Scenario:
Invece di una linea dritta, hai una fabbrica che smista i pacchi in base al contenuto.

1 thread Generatore: Genera numeri interi casuali (da 1 a 100) e li inserisce in una coda coda_iniziale (Max 10 elementi).

1 thread Smistatore: Legge da coda_iniziale. Se il numero è Pari, lo sposta in coda_pari (Max 5 elementi). Se il numero è Dispari, lo sposta in coda_dispari (Max 5 elementi).

2 thread Lavoratori: Il Lavoratore A legge solo da coda_pari, moltiplica il numero per 2 e lo stampa. Il Lavoratore B legge solo da coda_dispari, moltiplica il numero per 3 e lo stampa.

I Vincoli:

Tutte e tre le code (iniziale, pari, dispari) devono essere classi sicure (Thread-Safe) con gestione interna o esterna di blocchi per evitare buffer overflow.

Lo Smistatore è il thread più complesso: deve aspettare che ci siano dati in coda_iniziale, prelevarlo, capire cos'è, e poi aspettare che ci sia spazio nella coda di destinazione corretta prima di inserirlo.

Bisogna prevenire qualsiasi forma di Deadlock tra le code.


*/


#include <deque>
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <random>
#include <thread>

#define RAW 10
#define COMPUTED 5

class SafeThreadQueue {
    private:
        int size;
        std::deque<int> d;
        std::mutex handle;
        std::condition_variable cv_full, cv_empty;
    public:
        
        SafeThreadQueue(int s) : size(s) {};

        bool is_empty(){
            return d.size() == 0;
        }
        bool is_full() {
            return d.size() >= size;
        }

        
        int get_data(){
            std::unique_lock<std::mutex> lock(handle);
            cv_empty.wait(lock, [this](){return !this->is_empty();});
            int val = d.front();
            d.pop_front();
            cv_full.notify_one();
            return val;
        }

        void insert(int data){
           
           std::unique_lock<std::mutex> lock(handle);
           cv_full.wait(lock, [this]{return !this->is_full();});
           d.push_back(data);
           cv_empty.notify_one();


        }

};


SafeThreadQueue raw_buffer(RAW);
SafeThreadQueue even_computed(COMPUTED);
SafeThreadQueue noteven_computed(COMPUTED);

std::random_device dev;
std::mt19937 mm(dev());
std::uniform_int_distribution<int> d(0,500);

std::mutex console;

void producer(){
    while(true){
        int random_value = d(mm);
        raw_buffer.insert(random_value);
    }

}

void router(){
    while(true){
        int value = raw_buffer.get_data();
        (value % 2) ? noteven_computed.insert(value) : even_computed.insert(value);
    }
}

void noteven_computer(){
    while(true){
        int value = noteven_computed.get_data();
        std::lock_guard<std::mutex> lock(console);
        std::cout << value << std::endl;
    }
}


void even_computer(){
    while(true){
        int value = even_computed.get_data();
        std::lock_guard<std::mutex> lock(console);
        std::cout << value << std::endl;
    }
}

int main(){
    std::thread t1(producer), t2(router), t3(noteven_computer), t4(even_computer);
    t1.join();
    t2.join();
    t3.join();
    t4.join();

    return 0;
}