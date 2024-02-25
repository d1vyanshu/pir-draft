#include "client.h"
#include "dpf.h"
#include "keys.h"
#include "server.h"
#include "server_trusted.h"
#include <chrono>
#include<fstream>


int main() {

    int entry_size = 40;
    int Bout = bitlength;
    int Bin = 16;
    int t = 125;
    int database_size = (1<<Bin);
    int block;
    if(entry_size%bitlength!=0) block = entry_size/bitlength + 1;
    else block = entry_size/bitlength;

    prng.SetSeed(toBlock(0, 0), sizeof(block));
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
    std::cout<<"Database "<<database[100].value<<"\n";
    // std::cout<<"Database 1 "<<databaseB[0][1].value<<" Database 1000 "<<databaseB[1][1].value<<" database 2000 "<<databaseB[2][1].value<<"\n";
    std::string ip[4] = {"127.0.0.1", "127.0.0.1", "127.0.0.1", "127.0.0.1"};
    int port[4] = {2000, 2001, 3000, 3001};
    std::string ipr[2] = {"127.0.0.1", "127.0.0.1"};
    int portr[2] = {8000, 8001};
    ServerTrusted p2 = ServerTrusted(ip, port, 2);
    p2.connect_to_client(ipr, portr);
    std::cout<<"----------------Running Key Gen for cut-and-choose-----------------\n";



    GroupElement *out0, *out1, *outhash0, *outhash1, r[t];
    GroupElement **o0, **o1;
    if(entry_size <= bitlength) {
        out0 = (GroupElement*)malloc(t*sizeof(GroupElement));
        out1 = (GroupElement*)malloc(t*sizeof(GroupElement));
        outhash0 = (GroupElement*)malloc(t*sizeof(GroupElement));
        outhash1 = (GroupElement*)malloc(t*sizeof(GroupElement));
    }
    else {
        o0 = (GroupElement**)malloc(t*sizeof(GroupElement*));
        o1 = (GroupElement**)malloc(t*sizeof(GroupElement*));
        for(int j=0; j<t; j++) {
            o0[j] = (GroupElement*)malloc(block*sizeof(GroupElement));
            o1[j] = (GroupElement*)malloc(block*sizeof(GroupElement));
        }
    }
    
    for(int j=0; j<t; j++) {
        r[j] = random_ge(Bin);
        p2.send_ge(r[j], Bin, 2);
    }

    for(int j=0; j<t; j++) {
        if(j%10 == 0) std::cout<<"Iteration "<<j<<"\n";
        dpf_input_pack* dpfip = new dpf_input_pack;

        dpfip->index = r[j];
        dpfxor_key k0, k1;

        auto start = std::chrono::high_resolution_clock::now();
        std::tie(k0, k1) = dpfxor_keygen_local(Bin, dpfip);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
        int kgtime = static_cast<int>(duration.count());
        free(dpfip);
        //Sending key and index to P0
        auto start_send = std::chrono::high_resolution_clock::now();
        // std::cout<<"P2 key0: s "<<k0.s<<"\n";

        // for(int i=0; i<k0.height-7; i++) {
        //     std::cout<<"P2 sigma i "<<i<<" "<<k0.sigma[i]<<"\n";
        // }
        
        // GroupElement index0 = random_ge(Bin);
        // std::cout<<"P2 index "<<index0.value<<"\n";
        // p2.send_ge(index0, Bin, 0);
        // std::cout<<"P2 here\n";
        p2.send_dpfxor_key(k0, Bin, 0);
        
        
        //Sending key, index and payload to P1
        // dpfip->index = dpfip->index - index0;
        // p2.send_ge(dpfip->index, Bin, 1);
        p2.send_dpfxor_key(k1, Bin, 1);

        auto end_send = std::chrono::high_resolution_clock::now();
        auto duration_send = std::chrono::duration_cast<std::chrono::microseconds>(end_send-start_send);
        auto preprocess_comm = p2.bytes_sent;
        // std::cout<<"P2 key0: s "<<k0.s<<"\n";

        // for(int i=0; i<k0.height-7; i++) {
        //     std::cout<<"P2 sigma i "<<i<<" "<<k0.sigma[i]<<"\n";
        //     std::cout<<"P2 tau0"<<(int)k0.tau0[i]<<"\n";
        //     std::cout<<"P2 tau1"<<(int)k0.tau1[i]<<"\n";
        // }

        // std::cout<<"P2 gamma "<<k0.gamma<<"\n\n\n\n";

        // std::cout<<"P2 key1: s "<<k1.s<<"\n";

        // for(int i=0; i<k1.height-7; i++) {
        //     std::cout<<"P2 sigma i "<<i<<" "<<k1.sigma[i]<<"\n";
        //     std::cout<<"P2 tau0"<<(int)k1.tau0[i]<<"\n";
        //     std::cout<<"P2 tau1"<<(int)k1.tau1[i]<<"\n";
        // }

        // std::cout<<"P2 gamma "<<k1.gamma<<"\n";
        
        // std::cout<<"P2: here\n";
        //Receiving output from P0
        if(entry_size <= bitlength) {
                out0[j] = p2.recv_ge(bitlength, 0);
                out1[j] = p2.recv_ge(bitlength, 1);
                // std::cout<<"P2 o0 "<<o0.value<<" o1 "<<o1.value<<"\n";

                // uint64_t dbout = o0.value ^ o1.value;

                // if(j == 100) std::cout<<"\nP2: Output: "<<dbout<<" .\n\n";

        }
        else {

                
                // o0 = (GroupElement*)malloc(block*sizeof(GroupElement));
                // o1 = (GroupElement*)malloc(block*sizeof(GroupElement));

                for(int i=0; i<block; i++) {
                    o0[j][i] = p2.recv_ge(bitlength, 0);
                }

                for(int i=0; i<block; i++) {
                    o1[j][i] = p2.recv_ge(bitlength, 1);
                }
                // if(j==10) {
                //     for(int x=0; x<block; x++) {
                //         std::cout<<"db0 "<<(o0[j][x].value ^ o1[j][x].value)<<"\n";
                //     }
                // }
                // uint64_t *dbout = (uint64_t*) malloc(block*sizeof(uint64_t));
                // for(int i=0; i<block; i++) {
                //     dbout[i] = (o0[i].value ^ o1[i].value);
                    // std::cout<<"Block i: "<<i<<" value: "<<dbout[i].value<< " ";
                    
        }
                // std::cout<<o0[i]
                // std::cout<<dbout[0]<<"\n";
                // if(j==100) {
                //     std::ofstream myfile("output.txt");
                //         for(int i=0; i<block; i++)
                //             myfile<<dbout[i]<<std::endl;
                //     myfile.close();

                //     std::cout<<"\n";
                // }  

        
    }
    // std::cout<<"P2: Here also\n";
    // std::cout<<(out0[1].value ^ out1[1].value)<<"\n";
    


    uint8_t open[t];
    for(int i=0; i<t; i++) {
        open[i] = p2.recv_uint8(2);
    }


    // for(int i=0; i<10; i++)
    //     std::cout<<(int)open[i]<<"\n";
    for(int i=0; i<t; i++) {
        p2.send_uint8(open[i], 0);
        p2.send_uint8(open[i], 1);
        // std::cout<<(int)open[i]<<"\n";
    }


    if(entry_size <= bitlength) {
        for(int i=0; i<t; i++) {
            
                outhash0[i] = p2.recv_ge(bitlength, 0);
                outhash1[i] = p2.recv_ge(bitlength, 1);

                if(open[i]) {
                    // std::cout<<"i "<<i<<"\n";
                        if(outhash0[i].value != out0[i].value)
                            std::cout<<"Server S0 is cheating\n";
                        else if(outhash1[i].value != out1[i].value)
                            std::cout<<"Server S1 is cheating\n";

                        if((outhash0[i].value ^ outhash1[i].value) != database[(GroupElement(2, Bin)*r[i]).value].value) {
                            std::cout<<"One of the evaluator is cheating.\n";
                            // std::cout<<"i "<<i<<" "<<(out0[i].value ^ out1[i].value)<<" "<<database[(GroupElement(2, Bin)*r[i]).value].value<<"\n";
                        }
                }
            }
    }

    
    // uint8_t temp = 4;
    // p2.send_uint8(temp, 2);
    for(int i=0; i<t; i++) {
        if(entry_size <= bitlength) {
            if(open[i]) {
                GroupElement temp = GroupElement(0, bitlength);
                p2.send_ge(temp, bitlength, 2);
            }
            else {
                // std::cout<<"i "<<i<<" out0 "<<out0[i].value<<" out1 "<<out1[i].value<<"\n";
                p2.send_ge(out0[i], bitlength, 2);
                p2.send_ge(out1[i], bitlength, 2);
            }
        }
        else {
            if(open[i]) {
                for(int j=0; j<block; j++) {
                   GroupElement temp = GroupElement(0, bitlength);
                    p2.send_ge(temp, bitlength, 2); 
                }
            }
            else {
                for(int j=0; j<block; j++) {
                    p2.send_ge(o0[i][j], bitlength, 2);
                    p2.send_ge(o1[i][j], bitlength, 2);
                }
            }
        }

    }



    p2.close(0);
    p2.close(1);
    p2.close(2);
    // std::cout << "P2: Time taken for keygen + sending to P0, P1: " << duration.count()*1e-6 + duration_send.count() * 1e-6 <<"\n";
    // std::cout<< "P2: Bytes sent in offline phase: " << preprocess_comm << "\n";



}