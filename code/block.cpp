#include "block.h"

void Block::addTransaction(Transaction& txn) {
    // Adds a transaction to the block
    transactions.push_back(txn);
}

int Block::getBlocksize() {
    // Returns the size of the block
    return blockSize;
}

string Block::getBlockHeaderHash() 
{
    if (hashBlockHeader != "") return hashBlockHeader;
    string merkle_root = "";
    for (auto txn : transactions) merkle_root += txn.getString();
    merkle_root = sha256(merkle_root);
    string res = merkle_root;
    res += to_string(minerID);
    res += to_string(height);
    res += parentHash;
    res += to_string(timestamp_of_creation);
    res = sha256(res);
    hashBlockHeader = res;
    return res;
}
