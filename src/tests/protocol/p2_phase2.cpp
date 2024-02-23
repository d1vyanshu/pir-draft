#include "client.h"
#include "dpf.h"
#include "keys.h"
#include "server.h"
#include "server_trusted.h"
#include <chrono>

int main() {

    prng.SetSeed(toBlock(0, time(NULL)), sizeof(block));

    std::string ip[4] = {"127.0.0.1", "127.0.0.1", "127.0.0.1", "127.0.0.1"};
    int port[4] = {2000, 2001, 3000, 3001};
    std::string ipr[2] = {"127.0.0.1", "127.0.0.1"};
    int portr[2] = {8000, 8001};
    ServerTrusted p2 = ServerTrusted(ip, port, 2);
    p2.connect_to_client(ipr, portr);
    std::cout<<"----------------Running Key Gen for cut-and-choose-----------------\n";

    int entry_size = 40;
    int Bout = bitlength;
    int Bin = 18;
    int t = 125;

    int block;
    if(entry_size%bitlength!=0) block = entry_size/bitlength + 1;
    else block = entry_size/bitlength;

    dpf_input_pack* dpfip = new dpf_input_pack;
    dpfip->index = GroupElement(5, Bin);
    dpfxor_key k0, k1;

    auto start = std::chrono::high_resolution_clock::now();
    std::tie(k0, k1) = dpfxor_keygen_local(Bin, dpfip);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
    int kgtime = static_cast<int>(duration.count());

    //Sending key and index to P0
    auto start_send = std::chrono::high_resolution_clock::now();
    // std::cout<<"P2 key0: s "<<k0.s<<"\n";

    // for(int i=0; i<k0.height-7; i++) {
    //     std::cout<<"P2 sigma i "<<i<<" "<<k0.sigma[i]<<"\n";
    // }
    
    GroupElement index0 = random_ge(Bin);
    // std::cout<<"P2 index "<<index0.value<<"\n";
    p2.send_ge(index0, Bin, 0);
    // std::cout<<"P2 here\n";
    p2.send_dpfxor_key(k0, Bin, 0);
    
    
    //Sending key, index and payload to P1
    dpfip->index = dpfip->index - index0;
    p2.send_ge(dpfip->index, Bin, 1);
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
            GroupElement o0 = p2.recv_ge(bitlength, 0);
            GroupElement o1 = p2.recv_ge(bitlength, 1);
            // std::cout<<"P2 o0 "<<o0.value<<" o1 "<<o1.value<<"\n";

            uint64_t dbout = o0.value ^ o1.value;

            std::cout<<"\nP2: Output: "<<dbout<<" .\n\n";

    }
    else {

            GroupElement *o0, *o1;
            o0 = (GroupElement*)malloc(block*sizeof(GroupElement));
            o1 = (GroupElement*)malloc(block*sizeof(GroupElement));

            for(int i=0; i<block; i++) {
                o0[i] = p2.recv_ge(bitlength, 0);
            }

            for(int i=0; i<block; i++) {
                o1[i] = p2.recv_ge(bitlength, 0);
            }

            GroupElement* dbout = (GroupElement*)malloc(block*sizeof(GroupElement));
            for(int i=0; i<block; i++) {
                dbout[i] = o0[i] + o1[i];
                std::cout<<"Block i: "<<i<<" value: "<<dbout[i].value<< " ";
            }

            std::cout<<"\n";       
            
    }
    // std::cout<<"P2: Here also\n";
    uint8_t temp = 4;
    p2.send_uint8(temp, 2);


    p2.close(0);
    p2.close(1);
    p2.close(2);
    std::cout << "P2: Time taken for keygen + sending to P0, P1: " << duration.count()*1e-6 + duration_send.count() * 1e-6 <<"\n";
    std::cout<< "P2: Bytes sent in offline phase: " << preprocess_comm << "\n";



}