#include "client.h"
#include "dpf.h"
#include "keys.h"
#include "server.h"
#include "server_trusted.h"
#include <chrono>


int main() {
    auto start2 = std::chrono::high_resolution_clock::now();
    int input_size = 18;
    int database_size = (1<<input_size);
    int entry_size = 40;
    int block;
    if(entry_size%bitlength!=0) block = entry_size/bitlength + 1;
    else block = entry_size/bitlength;
    prng.SetSeed(toBlock(0, 0), sizeof(block));
    //Creating database for both cases when entry size < 40 bits and 1KB.
    GroupElement *database = NULL;
    GroupElement **databaseB = NULL;
    if(entry_size<= bitlength) {
        database = new GroupElement[database_size];
        for(int i=0; i<database_size; i++)
            database[i] = random_ge(bitlength);
            // database[i] = i;
    }
    else {
        databaseB = (GroupElement**)malloc(block*sizeof(GroupElement*));
        for(int i=0; i<block; i++) {
            databaseB[i] = (GroupElement*)malloc(database_size*sizeof(GroupElement));
            for(int j=0; j<database_size; j++) {
                databaseB[i][j] = random_ge(bitlength);
            }

        }
    }
    // std::cout<<"Database 1 "<<database[1].value<<" Database 1000 "<<database[1000].value<<" database 2000 "<<database[2000].value<<"\n";

    // std::cout<<"Here\n";
    // std::cout<<"database D[i] "<<databaseB[0][1046].value<<" "<<databaseB[1][1046].value<<" "<<databaseB[2][1046].value<<"\n";
    // std::cout<<"here2\n";
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2-start2);
    // std::cout<<duration2.count()*1e-6<<'\n';

    std::string ip[2] = {"127.0.0.1", "127.0.0.1"};
    int port[2] = {3000, 3001};
    std::string ipq[2] = {"127.0.0.1", "127.0.0.1"};
    int portq[2] = {6000, 6001};

    Server p1 = Server(ip, port, 1);
    p1.connect_to_client(ipq, portq);

    //Receive DPF key from P2
    int t = 125;
    GroupElement indices[t];
    for(int i=0; i<t; i++) {
        indices[i] = p1.recv_ge(input_size, 3);
        // std::cout<<"index i"<<i<<" "<<indices[i].value<<"\n";
    }
    GroupElement o[t];
    for(int j=0; j<t; j++) {
        // GroupElement index = p1.recv_ge(input_size, 2);
        dpfxor_key k1 = p1.recv_dpfxor_key(bitlength, 2);

        //     std::cout<<"P1 key s "<<k1.s<<"\n";
        // for(int i=0; i<k1.height-7; i++) {
        //     std::cout<<"P1 sigma "<<i<<" "<<k1.sigma[i]<<"\n";
        //     std::cout<<"P1 tau0"<<(int)k1.tau0[i]<<"\n";
        //     std::cout<<"P1 tau1"<<(int)k1.tau1[i]<<"\n";
        // }

        // std::cout<<"P1 gamma "<<k1.gamma<<"\n\n";
        //Eval All
        // uint8_t* t = (uint8_t*)malloc(database_size*sizeof(uint8_t));
        // auto start = std::chrono::high_resolution_clock::now();
        uint8_t* t = dpfxor_eval_all(1, k1);
        // auto end = std::chrono::high_resolution_clock::now();
        // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);

        // std::cout<<"P1 here\n";
        if(entry_size <= bitlength) {
                // auto start2 = std::chrono::high_resolution_clock::now();
                o[j] = inner_xor(database_size, indices[j], database, t);
                // auto end2 = std::chrono::high_resolution_clock::now();
                // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end2-start2);
                // std::cout<<"P1 o "<<o.value<<"\n";

                p1.send_ge(o[j], bitlength, 2);
                p1.bytes_sent += 24;
        }
        else {
                // auto start2 = std::chrono::high_resolution_clock::now();
                
                GroupElement* o = inner_xor_Zp(database_size, indices[j], databaseB, t, block);
                // std::cout<<"P1: "<<o[0].value<<"\n";
                for(int i=0; i<block; i++) {
                    p1.send_ge(o[i], bitlength, 2);
                }

                p1.bytes_sent += (32 - block*8);

                // auto end2 = std::chrono::high_resolution_clock::now();
                // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end2-start2);
                
                
        }

        free_dpfxor_key(k1);
    }

    uint8_t open[t];
    for(int i=0; i<t; i++) {
        open[i] = p1.recv_uint8(2);
        // std::cout<<(int)open[i]<<"\n";
    }

    if(entry_size <= bitlength) {
        for(int i=0; i<t; i++) {
            if(open[i])
                p1.send_ge(o[i], bitlength, 2);
            else {
                GroupElement temp = GroupElement(0, bitlength);
                p1.send_ge(temp, bitlength, 2);
                p1.bytes_sent -= 8;
            }
        }
    

        
        for(int i=0; i<t; i++) {
            if(open[i]==0)
                p1.send_ge(o[i], bitlength, 3);
            else {
                GroupElement temp = GroupElement(0, bitlength);
                p1.send_ge(temp, bitlength, 3);
                p1.bytes_sent -= 8;
            }
        }
    }


    if(database != NULL) free(database);
    if(databaseB != NULL) {
        for(int i=0; i<block; i++)
            free(databaseB[i]);
        free(databaseB);
    }
    p1.close(0);
    p1.close(1);
        std::cout<<"Communication bytes sent by P1: "<<p1.bytes_sent<<"\n";

    // std::cout << "p1: Time taken for EvalAll: " << duration.count()*1e-6 <<"\n";
    // std::cout << "p1: Time taken for Offline Phase: " << (duration.count() + kgtime)*1e-6 << "\n";
   
    
}