#include "blockchain.h"

bool Blockchain::validateBlock(Block block) {
    // This function checks if the block is valid or not
    if (blocks.find(block.parentHash) == blocks.end()) {
        // If the parent block is not present in the blockchain
        return false;
    }
    map<int, double> all_peer_balances; // calculating the peer balances from the blocks of the blockchain
    for (int i = 0 ; i < num_nodes ; i ++ ) {
        all_peer_balances[i] = initial_balance;
    }
    Block current_block = block;
    while (true) {
        if (children_without_parent.count(current_block.hashBlockHeader)) {
            // If the block is not present in the blockchain
            // then we can't validate the block
            return false;
        }
        for (Transaction txn : current_block.transactions) {
            all_peer_balances[txn.sender] -= txn.amount;
            all_peer_balances[txn.receiver] += txn.amount;
        }
        all_peer_balances[current_block.minerID] += minerReward; // mining fee
        if (current_block.parentHash == genesisHash or current_block.parentHash == "") break; // genesis block reached, we can break
        current_block = blocks[current_block.parentHash];
    }
    for (int i = 0 ; i < num_nodes ; i ++ ) {
        if (all_peer_balances[i] < 0) {
            // If the balance of any peer is negative, then the block is invalid
            return false;
        }
    }
    return true;
}

double Blockchain::getPeerBalance(int peerID) {
    // This function returns the balance of the peer
    double balance = initial_balance;
    // cout << " leaf node" << returnLeafNode() << endl;
    Block current_block = blocks[returnLeafNode()]; // get the leaf node
    while (true) {
        if (children_without_parent.count(current_block.getBlockHeaderHash())) {
            break;
        }
        for (Transaction txn : current_block.transactions) {
            if (txn.sender == peerID) balance -= txn.amount;
            if (txn.receiver == peerID) balance += txn.amount;

        }
        if (current_block.minerID == peerID) balance += minerReward; // mining fee
        // cout << " hello " << current_block.parentHash << " " << genesisHash << endl;
        if (current_block.parentHash == genesisHash or current_block.parentHash == "") break; // reached the genesis block
        current_block = blocks[current_block.parentHash];
    }
    return balance;
}

bool Blockchain::insertBlock(Block block, double timestamp) {
    // This function inserts the block in the blockchain
    if (blocks.find(block.getBlockHeaderHash()) == blocks.end()) block_to_timestamp[block.getBlockHeaderHash()] = timestamp; // Storing the timestamps for printing purposes (if the block comes back again, then no need to update the timestamp)
    blocks[block.getBlockHeaderHash()] = block; // Map from id to block object
    if (blocks.find(block.parentHash) == blocks.end()) {
        // Child came before the parent, so for now we just return false, and add it when the parent reaches
        children_without_parent.insert(block.getBlockHeaderHash());
        return true;
    }
    string curr_hash = block.getBlockHeaderHash();
    while(curr_hash != genesisHash && curr_hash != "") {
        if (children_without_parent.count(curr_hash)) {
            children_without_parent.insert(block.getBlockHeaderHash());
            return true;
        }
        curr_hash = blocks[curr_hash].parentHash;
    }
    if (block.getBlockHeaderHash() != genesisHash && !validateBlock(block)) {
        return false;
    } // If it is not the genesis block and the block is invalid, then return false
    if (block.getBlockHeaderHash() != genesisHash) {
        // map between parent and children
        parent_block_id[block.getBlockHeaderHash()] = block.parentHash;
        children_block_ids[block.parentHash].push_back(block.getBlockHeaderHash());
    }
    
    if (current_leaf_node == block.parentHash) current_leaf_node = block.getBlockHeaderHash(); // if the parent was previous leaf node, then update the leaf node
    if (leafBlocks.find(block.parentHash) != leafBlocks.end()) leafBlocks.erase(block.parentHash); // parent is no longer a leaf node
    leafBlocks.insert(block.getBlockHeaderHash()); // child is the new leaf node
    int max_height = blocks[current_leaf_node].height; // the previous maximum height
    string prev_leaf_node = current_leaf_node;
    for (auto& b : leafBlocks) {
        // Updating the leaf node
        // This is essentially the logic for picking the chain with the maximum height
        if (blocks[b].height > max_height) {
            max_height = blocks[b].height;
            current_leaf_node = b;
        }
    }
    string new_leaf_node = current_leaf_node;
    if (prev_leaf_node != new_leaf_node) {
        // We need to update the transaction pool of the peer, if we have a new leaf node
        // We need to find the common ancestor of the two leaf nodes
        // For this we keep going up the chain until we reach the same height
        while(blocks[prev_leaf_node].height > blocks[new_leaf_node].height) {
            for (Transaction &t : blocks[prev_leaf_node].transactions) {
                if (peers[owner_id]->txPool.find(t) == peers[owner_id]->txPool.end()) {
                    peers[owner_id]->txPool.insert(t);
                }
            }
            prev_leaf_node = parent_block_id[prev_leaf_node];
        }
        while(blocks[new_leaf_node].height > blocks[prev_leaf_node].height) {
            for (Transaction &t : blocks[new_leaf_node].transactions) {
                if (peers[owner_id]->txPool.find(t) != peers[owner_id]->txPool.end()) {
                    peers[owner_id]->txPool.erase(t);
                }
            }
            new_leaf_node = parent_block_id[new_leaf_node];
        }
        while (prev_leaf_node != new_leaf_node) {
            for (Transaction &t : blocks[prev_leaf_node].transactions) {
                if (peers[owner_id]->txPool.find(t) == peers[owner_id]->txPool.end()) {
                    peers[owner_id]->txPool.insert(t);
                }
            }
            for (Transaction &t : blocks[new_leaf_node].transactions) {
                if (peers[owner_id]->txPool.find(t) != peers[owner_id]->txPool.end()) {
                    peers[owner_id]->txPool.erase(t);
                }
            }
            prev_leaf_node = parent_block_id[prev_leaf_node];
            new_leaf_node = parent_block_id[new_leaf_node];
        }
    }
    if (current_leaf_node == block.getBlockHeaderHash()) {
        for (Transaction& t : block.transactions) {
            if (peers[owner_id]->txPool.find(t) != peers[owner_id]->txPool.end()) {
                peers[owner_id]->txPool.erase(t);
            }
        }
    }
    for (string child : children_without_parent) {
        // We need to check if the children of the block can now be added
        if (blocks[child].parentHash == block.getBlockHeaderHash()) {
            children_without_parent.erase(child); // Parent found
            for (auto p : peers[owner_id]->neighbours)
                peers[owner_id]->sendHash(blocks[child].getBlockHeaderHash(), p.second->id);
            insertBlock(blocks[child], timestamp); // Insert the block
            return true;
        }
    }
    return true;
}

string Blockchain::returnLeafNode () {
    return current_leaf_node; // Returns the current leaf node
}

int Blockchain::getLongestChainHeight () {
    string leaf_node = current_leaf_node;
    return blocks[leaf_node].height + 1; // Returns the height of the longest chain
}

void Blockchain::saveBlockChain(string filename) {
    // This function saves the blockchain to a file
    ofstream file(filename, ios::trunc);
    file << fixed;
    for (auto& b : blocks) {
        if (b.first == "") continue;
        if(b.second.getBlockHeaderHash() == genesisHash) continue;
        file << b.second.getBlockHeaderHash() << "\t" << b.second.parentHash << "\t";
        file << (b.second.minerID == ringMaster ? "Malicious" : "Honest") << "\t";
        file << block_to_timestamp[b.second.getBlockHeaderHash()] << "\t";
        file << b.second.height << "\n";
    }
    file.close();
}

vector<Block> Blockchain::currentChain() {
    // This function returns the current chain
    vector<Block> chain;
    string leaf_node = current_leaf_node;
    while (leaf_node != genesisHash) {
        chain.push_back(blocks[leaf_node]);
        leaf_node = parent_block_id[leaf_node];
    }
    reverse(chain.begin(), chain.end());
    return chain;
}