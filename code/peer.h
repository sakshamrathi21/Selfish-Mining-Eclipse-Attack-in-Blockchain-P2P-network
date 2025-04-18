// The header file for the peer class. The peer class is the main class that represents a node in the network.
#ifndef PEER_H
#define PEER_H

#include <vector>
#include "transaction.h"
#include "block.h"
#include "blockchain.h"
#include "simulator.h"
#include "helper.h"
#include <iostream>
#include <set>
#include <map>
#include <cassert>
using namespace std;

// Referring from main.cpp
extern const int maxTransactionsPerBlock;
extern const double initial_balance;
extern double averageBlockArrivalTime;
extern int num_nodes;
extern vector<Peer*> peers;
extern const string genesisHash;
extern int GetRequestTimeout;
extern int ringMaster;
extern bool whether_eclipse_attack;
extern bool debug;

// Forward declaration of the Simulator and Blockchain classes
class Simulator;
class Blockchain;
class Peer {
public:
    Peer(Simulator* simulator, int id); 
    ~Peer();
    void generateTransaction(); 
    void receiveTransaction(Transaction txn, int sender_id);
    void receiveBlock(Block block, int sender_id);     
    void sendTransaction(Transaction txn, int targetPeerID); 
    double interArrivalTime;       
    void setHashingPower();
    void sendBlock(Block block, int targetPeerID);
    int id;                       
    map<int, Peer*> neighbours; 
    map<int, Peer*> malicious_neighbours;
    bool isMalicious;
    double getBlockInterArrivalTime();
    void createGenesisBlock();
    string mining_start();
    void mining_end(string mined_hash);
    Blockchain* blockchain;        
    multiset<Transaction> txPool;
    void sendHash(string hash, int targetPeerID);
    void receiveHash(string hash, int sender_id);
    void sendGetRequest(string hash, int targetPeerID);
    void receiveGetRequest(string hash, int sender_id);
    void handleTimeout(string hash);
    double hashingPower;  
    void broadcastPrivateChain(string receivedHash);  
    void receivePrivateMessage(string message, int sender_id);
    set<int> allBroadcastIDs;
    string selfish_mine_start = genesisHash;

    map<int, double> trustScore;
    map<int, int> banCount;
    map<int, pair<int, int>> pastAttempts;
    void handleFailedRequest(int sender_id);
    void handleSuccesfulRequest(int sender_id);
    void resetScore(int neighbour_id);
    double maxTrustDelay();
    double getTrustDelay(int sender_id);
    void sendDelayedGetRequest(string hash, int targetPeerID, double delayedTime);
    void reportTrust();
    void logToPeerFile(string action, string details);

private:
    Simulator* simulator;          
    double balance;                
    set<int> txIDs; 
    Block current_mined_block;
    string leaf_node = "";         
};

#endif 