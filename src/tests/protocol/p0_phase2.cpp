#include "client.h"
#include "dpf.h"
#include "keys.h"
#include "server.h"
#include "server_trusted.h"
#include <chrono>
#include<fstream>



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
    // for(int i=)
    // std::cout<<"Here\n";
    // std::ofstream myfile("db.txt");
    //         for(int i=0; i<block; i++)
    //                 myfile<<databaseB[i][1046].value<<std::endl;
    // myfile.close();
    // std::cout<<"Here\n";
    // std::cout<<"database D[i] "<<databaseB[0][1100].value<<" "<<databaseB[1][1100].value<<" "<<databaseB[2][1100].value<<"\n";
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2-start2);
    // std::cout<<duration2.count()*1e-6<<'\n';

    std::string ip[2] = {"127.0.0.1", "127.0.0.1"};
    int port[2] = {2000, 2001};
    std::string ipp[2] = {"127.0.0.1", "127.0.0.1"};
    int portp[2] = {4000, 4001};

    Server p0 = Server(ip, port, 0);
    p0.connect_to_client(ipp, portp);

    
    //Receive DPF key from P2
    int t = 125;
    GroupElement indices[t];
    for(int i=0; i<t; i++) {
        indices[i] = p0.recv_ge(input_size, 3);
        // std::cout<<"index i"<<i<<" "<<indices[i].value<<"\n";
    }
        
    GroupElement o[t];
    for(int j=0; j<t; j++) {
        // GroupElement index = p0.recv_ge(input_size, 2);
        // std::cout<<"P0 index "<<index.value<<"\n";
        dpfxor_key k0 = p0.recv_dpfxor_key(bitlength, 2);
        // std::cout<<"P0 key s "<<k0.s<<"\n";
        // for(int i=0; i<k0.height-7; i++) {
        //     std::cout<<"P0 sigma "<<i<<" "<<k0.sigma[i]<<"\n";
        //     std::cout<<"P0 tau0"<<(int)k0.tau0[i]<<"\n";
        //     std::cout<<"P0 tau1"<<(int)k0.tau1[i]<<"\n";
        // }

        // std::cout<<"P0 gamma "<<k0.gamma<<"\n\n";

        //Eval All
        // uint8_t* t = (uint8_t*)malloc(database_size*sizeof(uint8_t));
        // auto start = std::chrono::high_resolution_clock::now();
        uint8_t *t = dpfxor_eval_all(0, k0);
        // auto end = std::chrono::high_resolution_clock::now();
        // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);

        if(entry_size <= bitlength) {
                // auto start2 = std::chrono::high_resolution_clock::now();
                o[j] = inner_xor(database_size, indices[j], database, t);
                // auto end2 = std::chrono::high_resolution_clock::now();
                // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end2-start2);
                // std::cout<<"P0 o "<<o.value<<"\n";
                p0.send_ge(o[j], bitlength, 2);
                p0.bytes_sent += 24;
        }
        else {
                // auto start2 = std::chrono::high_resolution_clock::now();
                
                GroupElement* o = inner_xor_Zp(database_size, indices[j], databaseB, t, block);
                // std::cout<<"P0: "<<o[0].value<<"\n";
                for(int i=0; i<block; i++) {
                    p0.send_ge(o[i], bitlength, 2);
                }
                p0.bytes_sent += (32 - block*8);

                // auto end2 = std::chrono::high_resolution_clock::now();
                // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end2-start2);
                
                
        }


        free_dpfxor_key(k0);
    }

    // uint8_t open[t];
    // for(int i=0; i<t; i++)
    //     open[i] = p0.recv_uint8(2);
    
    // uint8_t zero = 0;
    // for(int i=0; i<t; i++) {
    //     if(open[i] == 0) p0.send_uint8(zero, 2);
    //     else p0.send_uint8(out)
    // }
    uint8_t open[t];
    for(int i=0; i<t; i++) 
        open[i] = p0.recv_uint8(2);

    if(entry_size <= bitlength) {
        for(int i=0; i<t; i++) {
            if(open[i])
                p0.send_ge(o[i], bitlength, 2);
            else {
                GroupElement temp = GroupElement(0, bitlength);
                p0.send_ge(temp, bitlength, 2);
                p0.bytes_sent -= 8;
            }
        }
                
            for(int i=0; i<t; i++) {
            if(open[i]==0)
                p0.send_ge(o[i], bitlength, 3);
            else {
                GroupElement temp = GroupElement(0, bitlength);
                p0.send_ge(temp, bitlength, 3);
                p0.bytes_sent -= 8;
            }
        }
    }

    if(database != NULL) free(database);
    if(databaseB != NULL) {
        for(int i=0; i<block; i++)
            free(databaseB[i]);
        free(databaseB);
    }
    p0.close(0);
    p0.close(1);
    std::cout<<"Communication bytes sent by P0: "<<p0.bytes_sent<<"\n";
    // std::cout << "P0: Time taken for EvalAll: " << duration.count()*1e-6 <<"\n";
    // std::cout << "P0: Time taken for Offline Phase: " << (duration.count() + kgtime)*1e-6 << "\n";
   
    
}