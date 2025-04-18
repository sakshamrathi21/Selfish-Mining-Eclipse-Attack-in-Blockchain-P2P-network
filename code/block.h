/* This file contains the block class */
#ifndef BLOCK_H
#define BLOCK_H

#include <vector>
#include "transaction.h"
#include "hash.h"
#include <cassert>
using namespace std;

extern const int blockSize;

class Block {
public:
    Block() = default;
    Block(string hashBlockHeader) : hashBlockHeader(hashBlockHeader) {}
    void addTransaction(Transaction& txn);
    // int id;
    // int parentID;
    vector<Transaction> transactions;
    int minerID; 
    int height;           
    int getBlocksize();
    string hashBlockHeader = "";
    string parentHash = "";
    string getBlockHeaderHash();
    int timestamp_of_creation = -1;
};

#endif