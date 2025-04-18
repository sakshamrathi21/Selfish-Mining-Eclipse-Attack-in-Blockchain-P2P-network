/* This file contains the blockchain class*/
#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H
#include <iostream>
#include "block.h"
#include "transaction.h"
#include "helper.h"
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <queue>
using namespace std;

// Referring variables from main.cpp
extern const double initial_balance;
extern int num_nodes;

extern const double minerReward;
extern const string genesisHash;
extern bool debug;

class Blockchain {
    public:
        Blockchain(int owner_id) { this->owner_id = owner_id; }
        int owner_id;
        map<string, Block> blocks;
        map<string, string> parent_block_id;
        map<string, vector<string>> children_block_ids;
        bool insertBlock(Block block, double timestamp);
        bool validateBlock(Block block);
        set<string> leafBlocks;
        string current_leaf_node = "";
        string returnLeafNode();
        double getPeerBalance(int peerID);
        int getLongestChainHeight ();
        map<string, double> block_to_timestamp;
        multiset<string> children_without_parent;
        void saveBlockChain(string filename);
        vector<Block> currentChain();
        map<string, queue<int>> hash_to_queue;
        map<string, int> hash_to_timeout;
        map<string, bool> whether_sent_to_honest;
};

#endif 
