#include <iostream>
#include <thread>
#include <list>
#include <map>
#include <condition_variable>
#include <mutex>
#include <algorithm>
#include <vector>


using namespace std;


struct WaitingThread {
    bool ready;
    condition_variable cv;
};

class PrioritySemaphore {

    private:
        int size_s;
        map<int,list<WaitingThread*>> waiting_queue;
        mutex mtx;

    
    public:
        
        PrioritySemaphore(int size) : size_s(size) {}
        
        void acquire(int priority){

            unique_lock<mutex> lock(mtx);

            if(size_s > 0){
                size_s--;
                return;
            }


            WaitingThread wt;
            list<WaitingThread> waiting_threads;

            wt.ready = false;

            waiting_queue[priority].push_back(&wt);


            

            wt.cv.wait(lock, [&wt]{return wt.ready;});

        }

        void release(void){
            unique_lock<mutex> lock(mtx);
            
            size_s++;
            if(waiting_queue.empty()){return;}
            
            auto it = find_if(waiting_queue.begin(), waiting_queue.end(),
                [](const auto &pair){
                    return !pair.second.empty();
                }

            );

            WaitingThread *chosen = it->second.front();
            chosen -> ready = true;
            chosen->cv.notify_one();
            it->second.pop_front();




        }



};


void worker(PrioritySemaphore& sem, int id, int priority) {
    cout << "Thread " << id << " (Priorita' " << priority << ") in attesa...\n";
    
    sem.acquire(priority); // Si blocca qui se i permessi sono a 0
    
    cout << "Thread " << id << " (Priorita' " << priority << ") HA OTTENUTO il semaforo!\n";
    this_thread::sleep_for(chrono::milliseconds(300)); // Simula un lavoro
    cout << "Thread " << id << " (Priorita' " << priority << ") rilascia il semaforo.\n";
    
    sem.release();
}

int main() {
    // Iniziamo con 0 permessi per forzare tutti i thread ad accodarsi
    PrioritySemaphore sem(0); 
    vector<thread> threads;

    // Lanciamo i thread con priorità sfasate rispetto all'ordine di partenza
    threads.push_back(thread(worker, ref(sem), 1, 10)); // Priorità Bassa
    this_thread::sleep_for(chrono::milliseconds(50));   // Piccola pausa per forzare l'ordine di entrata in coda
    
    threads.push_back(thread(worker, ref(sem), 2, 50)); // Priorità Altissima
    this_thread::sleep_for(chrono::milliseconds(50));
    
    threads.push_back(thread(worker, ref(sem), 3, 30)); // Priorità Media
    this_thread::sleep_for(chrono::milliseconds(50));

    cout << "\n=== Tutti i thread sono bloccati. Inizio a rilasciare 1 permesso ===\n\n";
    
    // Il main sblocca la situazione dando il primo "gettone"
    sem.release();

    cout <<"\nFirst released\n";

    // Attendiamo che tutti abbiano finito
    for (auto& t : threads) {
        t.join();
    }
    
    cout << "\nTest concluso con successo.\n";
    return 0;
}