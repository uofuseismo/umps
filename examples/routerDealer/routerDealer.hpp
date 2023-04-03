#ifndef EXAMPLES_ROUTER_DEALER_HPP
#define EXAMPLES_ROUTER_DEALER_HPP
#define N_SERVERS 2
#define N_CLIENTS 3
#define N_REQUESTS 4
void proxy();
void server(int serverID = 1); 
void client(int clientID = 1); 
#endif
