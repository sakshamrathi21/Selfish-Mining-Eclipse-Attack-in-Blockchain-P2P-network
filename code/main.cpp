// This is the main file
#include <iostream>
#include <vector>
#include "peer.h"
#include "simulator.h"
#include "block.h"
#include "event.h"
#include "transaction.h"
#include "helper.h"
#include "blockchain.h"

using namespace std;

int num_nodes;
double malicious_percentage;
bool whether_ratio = false;
bool whether_longest_chain_height = false;
bool whether_blockchain = false;
bool whether_branches = false;
bool show_network = false;
bool enable_countermeasure = false;
bool whether_dump_all = false;
bool whether_eclipse_attack = true;
bool debug = false;

// Constants
const double minerReward = 50;
double averageBlockArrivalTime = 600;
const int maxTransactionsPerBlock = 999;
const int TransactionSize = 8000;
const int hashSize = 512;
const int broadcastPrivateChainSize = 512;
const int getSize = 560;
const int blockSize = 8000000;
const string genesisHash = "genesis";
const string BlockChainSaveDirectory = "blockchain_data/";
const double initial_balance = 0;
int totalExecutionTime;
int GetRequestTimeout;
int broadcastnumber = 0;
int ringMaster = -1;

vector<Peer*> peers;

int main (int argc, char *argv[]) {
    if (argc < 7) {
        // Checking if the number of arguments is correct
        cout << "Usage: " << argv[0] << " <number-of-nodes> <percentage-of-malicious-nodes> <mean-Time-for-transactions> <average-Block-Arrival-Time> <get-Request-Timeout> <time-of-execution> <flags> " << endl;
        return 1;
    }
    totalExecutionTime = stoi(argv[6]);
    GetRequestTimeout = stoi(argv[5]);
    for (int i = 7 ; i < argc ; i ++ ) {
        if (string(argv[i]) == "--ratio") {
            whether_ratio = true;
        } else if (string(argv[i]) == "--longest-chain-height") {
            whether_longest_chain_height = true;
        } else if (string(argv[i]) == "--blockchain") {
            whether_blockchain = true;
        } else if (string(argv[i]) == "--branches") {
            whether_branches = true;
        } else if (string(argv[i]) == "--show-network") {
            show_network = true;
        } else if (string(argv[i]) == "--countermeasure") {
            enable_countermeasure = true;
        } else if (string(argv[i]) == "--dump-all") {
            whether_dump_all = true;
        } else if (string(argv[i]) == "--no-eclipse") {
            whether_eclipse_attack = false;
        } else if (string(argv[i]) == "--debug") {
            debug = true;
        }
    }

    clearLogFile("logFiles"); // Clear the previous messages from the log file
    num_nodes = stoi(argv[1]);
    malicious_percentage = stod(argv[2]);
    averageBlockArrivalTime = stod(argv[4]);
    Simulator simulator(stod(argv[3]));
    for (int i = 0; i < num_nodes; i++) {
        Peer* peer = new Peer(&simulator, i);
        peers.push_back(peer);
    }
    generateConnectedGraph();
    vector<int> malicious_nodes;
    for (int i = 0 ; i < num_nodes ; i ++){
        if (peers[i]->isMalicious) malicious_nodes.push_back(i);
    }
    if (malicious_nodes.size()) 
    {
        // Selecting the ring master and giving it the hash power of all malicious nodes
        int ringMasterIndex =  uniformRandom(0, malicious_nodes.size() - 1);
        ringMaster = malicious_nodes[ringMasterIndex];
        peers[ringMaster]->hashingPower = ((double)malicious_nodes.size())/num_nodes;
    }
    cout << "Ringmaster is " << ringMaster << "." << endl;
    cout << "There are " << malicious_nodes.size() << " malicious peers." << endl;
    simulator.run();
    handlePostRunFlags();
    return 0;
}