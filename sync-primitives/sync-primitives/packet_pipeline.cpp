/*

n sistema di ispezione di rete costruisce e analizza pacchetti dati in memoria utilizzando un buffer circolare condiviso composto da N slot. Ogni slot contiene una struttura dati Pacchetto che deve essere assemblata prima di poter essere analizzata.

L'assemblaggio è diviso in tre fasi sequenziali, ciascuna gestita da un pool di thread "Producer" indipendenti e concorrenti:

Producer di Tipo A (Header): Inizializzano uno slot vuoto scrivendo l'intestazione del pacchetto.

Producer di Tipo B (Payload): Leggono il carico utile dalla rete e lo inseriscono nello slot, ma solo dopo che il Producer A ha scritto l'intestazione in quello specifico slot.

Producer di Tipo C (Firma): Calcolano l'hash di sicurezza (checksum) e lo scrivono nello slot, ma solo dopo che il Producer B ha inserito il payload.

Infine, un pool di thread Consumer (Analizzatori) preleva i pacchetti completi (che contengono Header, Payload e Firma), li processa e libera lo slot nel buffer per ricominciare il ciclo.


*/

#include <thread>
#include <iostream>
#include <semaphore>
#include <condition_variable>
#include <deque>
#include <string>
#include <sstream>
#include <vector>

using namespace std;
/*

-> thread condividono lo stack
-> ho N creator, N payload, N checksum e dei consumatori
-> uso delle code di pending-> pending l1, pending l2, pending consumer
*/

struct Packet {
    string header;
    string payload;
    string checksum;
};


counting_semaphore pending_payload{0};
counting_semaphore pending_checksum{0};
counting_semaphore pending_consumer{0};

deque<Packet> pending_payload_q;
deque<Packet> pending_checksum_q;
deque<Packet> pending_consumer_q;

mutex m_pending_payload;
mutex m_pending_checksum;
mutex m_pending_consumer;
mutex console;

vector<thread> thread_create;
vector<thread> thread_payload;
vector<thread> thread_checksum;
vector<thread> thread_consumer;

void create_packet(void){

    Packet p = *new Packet;
    stringstream ss;
    ss << this_thread::get_id();
    p.header = ss.str();

    unique_lock<mutex> lock(m_pending_payload);
    pending_payload_q.push_back(p);
    pending_payload.release();

}

void add_payload(void){
    pending_payload.acquire();


    unique_lock<mutex> lock(m_pending_payload);
    Packet p = pending_payload_q.front();
    pending_payload_q.pop_front();
    lock.unlock();
    stringstream ss;
    ss << this_thread::get_id();
    p.payload = ss.str();
    unique_lock<mutex> lock_1(m_pending_checksum);
    pending_checksum_q.push_back(p);
    pending_checksum.release();

}

void add_checksum(void){
    pending_checksum.acquire();

    unique_lock<mutex> lock(m_pending_checksum);

    Packet p = pending_checksum_q.front();
    pending_checksum_q.pop_front();

    lock.unlock();

    stringstream ss;
    ss << this_thread::get_id();

    p.checksum = ss.str();
    
    unique_lock<mutex> lock_1(m_pending_consumer);

    pending_consumer_q.push_back(p);
    pending_consumer.release();


}

void consume(void){
    pending_consumer.acquire();


    
    unique_lock<mutex> lock(m_pending_consumer);

    Packet p = pending_consumer_q.front();
    pending_consumer_q.pop_back();
    lock.unlock();

    lock_guard<mutex> lockIO(console);
    cout << "Header: " << p.header << " " << "Payload: " << p.payload << " " << "Checksum: " << p.checksum << endl;
}


int main(int argc, char *argv[]){
    int n = stoi(argv[1]);
    for(int i = 0; i<n; i++){

        thread_create.emplace_back(create_packet);
        thread_payload.emplace_back(add_payload);
        thread_checksum.emplace_back(add_checksum);
        thread_consumer.emplace_back(consume);
    }

    for(int j = 0; j<n; j++){
        thread_create[j].join();
        thread_payload[j].join();
        thread_checksum[j].join();
        thread_consumer[j].join();
    }

}