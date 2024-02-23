#include "client.h"
#include "dpf.h"
#include "keys.h"
#include "server.h"
#include "server_trusted.h"
#include <algorithm>
#include <chrono>

#include<fstream>

int main() {

    std::string ip[6] = {"127.0.0.1", "127.0.0.1", "127.0.0.1", "127.0.0.1", "127.0.0.1", "127.0.0.1"};
    int port[6] = {4000, 4001, 6000, 6001, 8000, 8001};
    Client c = Client(ip, port, 3);

    std::cout<<"C : here\n";
    uint8_t temp = c.recv_uint8(2);


    c.close(0);    
    c.close(1);    
    c.close(2); 
}