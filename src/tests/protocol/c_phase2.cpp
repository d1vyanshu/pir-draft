#include "client.h"
#include "dpf.h"
#include "keys.h"
#include "server.h"
#include "server_trusted.h"
#include <algorithm>
#include <chrono>
#include <bits/stdc++.h>
#include<fstream>

int main() {

    std::string ip[6] = {"127.0.0.1", "127.0.0.1", "127.0.0.1", "127.0.0.1", "127.0.0.1", "127.0.0.1"};
    int port[6] = {4000, 4001, 6000, 6001, 8000, 8001};
    Client c = Client(ip, port, 3);

    int input_size = 18;
    int entry_size = 40;
    int block;
    if(entry_size%bitlength!=0) block = entry_size/bitlength + 1;
    else block = entry_size/bitlength;

    uint8_t x = 0;
    c.send_uint8(x, 2);
    auto start = std::chrono::high_resolution_clock::now();

    int t=125;
    GroupElement index = GroupElement(100, input_size);
    GroupElement indices[t];
    uint8_t open[t];
    GroupElement r[t];
    for(int i=0; i<t; i++)
        r[i] = c.recv_ge(input_size, 2);

    for(int i=0; i<t; i++) 
        if(i<(3*t)/5) open[i] = 1;
        else open[i] = 0;
    
    
    std::shuffle(open, open + t, std::default_random_engine(time(NULL)));
    prng.SetSeed(toBlock(0, time(NULL)), sizeof(block));

    for(int i=0; i<t; i++) {
        if (open[i]) indices[i] = r[i];
        else indices[i] = index - r[i];

        // GroupElement index0 = random_ge(input_size);
        c.send_ge(indices[i], input_size, 0);
        // indices[i] = indices[i] - index0;
        c.send_ge(indices[i], input_size, 1);
        // std::cout<<"i "<<i<<" index0 "<<index0.value<<" index1 "<<indices[i].value<<"\n";
    }

    //C sends I_1 to P2 which has 3t/5 indices costing 4 bytes each.
    c.bytes_sent += 12*t/5;

    for(int i=0; i<t; i++) 
        c.send_uint8(open[i], 2);

    uint64_t out;
    uint64_t *outb0, *outb1;
    if(entry_size > bitlength) {
        outb0 = (uint64_t*)malloc(block*sizeof(uint64_t));
        outb1 = (uint64_t*)malloc(block*sizeof(uint64_t));
    }


    uint8_t flag = 0;
    for(int i=0; i<t; i++) {
        if(entry_size <= bitlength) {
            if(open[i]) {
                GroupElement temp = c.recv_ge(bitlength, 2);
                GroupElement temp0 = c.recv_ge(bitlength, 0);
                GroupElement temp1 = c.recv_ge(bitlength, 1);
            }
            else {
                GroupElement temp1 = c.recv_ge(bitlength, 2);
                GroupElement temp2 = c.recv_ge(bitlength, 2);
                GroupElement temp3 = c.recv_ge(bitlength, 0);
                GroupElement temp4 = c.recv_ge(bitlength, 1);
                // std::cout<<"i "<<i<<" temp1 "<<temp1.value<<" temp2 "<<temp2.value<<"\n";
                if(flag) {
                    continue;
                }
                else {
                    if(temp1.value != temp3.value)
                        std::cout<<"Server S0 is cheating\n";
                    else if(temp2.value != temp4.value)
                        std::cout<<"Server S1 is cheating\n";
                out = temp3.value ^ temp4.value; 
                //    std::cout<<out<<"\n";
                flag = 1;
                }
            }
        }
        else {
            if(open[i]) {
                for(int j=0; j<block; j++) 
                    GroupElement temp = c.recv_ge(bitlength, 2); 
            }
            else {
                if(flag) continue;
                
                for(int j=0; j<block; j++) {
                    outb0[j] = (c.recv_ge(bitlength, 2)).value;
                    outb1[j] = (c.recv_ge(bitlength, 2)).value;
                }
                flag = 1;
            }
        }
    }

    std::cout<<"Final Output"<<out<<"\n";
    // for(int j=0; j<block; j++)
        // std::cout<<(outb0[j] ^ outb1[j])<<" ";

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
    std::cout << "Runtime for phase 2: " << duration.count()*1e-6 <<"\n";

        std::cout<<"Client sends bytes: "<<c.bytes_sent<<"\n";

    c.close(0);    
    c.close(1);    
    c.close(2); 
}