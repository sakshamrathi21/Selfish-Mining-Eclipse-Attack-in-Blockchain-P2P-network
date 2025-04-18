#include "peer.h"

Peer::Peer(Simulator* simulator, int id) : simulator(simulator), id(id) {
    // Default constructor
    isMalicious = false;
    balance = initial_balance; 
    blockchain = new Blockchain(id); // Create a new blockchain for this peer
}

void Peer::generateTransaction() {
    // This function generates a transaction
    double our_balance = blockchain->getPeerBalance(id);
    if (our_balance <= 0) return; // If we don't have any balance, we can't generate a transaction    
    int targetPeerID = id;
    while (targetPeerID == id) {
        targetPeerID = uniformRandom(0, num_nodes - 1); // Choose a random peer to send the transaction to
    }
    Transaction txn = Transaction(id, targetPeerID, uniformRandom(1, our_balance)); // Create a new transaction with a random amount
    txPool.insert(txn); // Insert the transaction into the transaction pool
    
    for (auto& [id, peer] : malicious_neighbours) {
        // Send the transaction to all neighbours
        sendTransaction(txn, peer->id);
    }
    for (auto& [id, peer] : neighbours) {
        // Send the transaction to all neighbours
        sendTransaction(txn, peer->id);
    }
    // if (txPool.size() > 100) simulator->scheduleEvent(simulator->getCurrentTime(), MINING_START, id, -1, {});
}

void Peer::receiveTransaction(Transaction txn, int sender_id) {
    // This function is called when a peer receives a transaction
    if (txIDs.count(txn.getID()) || txn.amount <= 0) return; // If the transaction has already been received, ignore it
    txIDs.insert(txn.getID());
    txPool.insert(txn);  // Insert the transaction into the transaction pool
    for (auto& [id, peer] : malicious_neighbours) {
        // Send the transaction to all neighbours
        sendTransaction(txn, peer->id);
    }
    for (auto& [id, peer] : neighbours) {
        // Send the transaction to all neighbours
        if (peer->id != sender_id) {
            sendTransaction(txn, peer->id);
        }
    }
    // if (txPool.size() > 100) simulator->scheduleEvent(simulator->getCurrentTime(), MINING_START, id, -1, {});
}

void Peer::sendTransaction(Transaction txn, int targetPeerID) {
    // This function is called when a peer sends a transaction
    bool whether_overlay = false;
    if (malicious_neighbours.count(targetPeerID)) whether_overlay = true;
    simulator->scheduleEvent(simulator->getCurrentTime(), TRANSACTION_SEND, id, targetPeerID, txn, whether_overlay);
}

/////////////
void Peer::receiveBlock(Block block, int sender_id) {

    // This function is called when a peer receives a block 
    if(blockchain->blocks.count(block.getBlockHeaderHash())) return; // If the block has already been received, ignore it
    if (blockchain->hash_to_timeout.count(block.getBlockHeaderHash())) {
        blockchain->hash_to_timeout.erase(block.getBlockHeaderHash());
    }
    if (blockchain->hash_to_queue.count(block.getBlockHeaderHash())) {
        blockchain->hash_to_queue.erase(block.getBlockHeaderHash());
    }
    
    if(enable_countermeasure) { handleSuccesfulRequest(sender_id); }

    string old_leaf_node = blockchain->returnLeafNode();

    bool whether_valid = blockchain->insertBlock(block, simulator->getCurrentTime()); // Insert the block into the blockchain
    if (!whether_valid) return; // If the block is invalid, ignore it
    for (auto& [id, peer]: malicious_neighbours)
    {
        if (peer->id != sender_id)
        {
            sendHash(block.getBlockHeaderHash(), peer->id);
        }
    }
    if (!isMalicious || block.minerID != ringMaster) {
        for (auto& [id, peer] : neighbours) {
            if (peer->id != sender_id) {
                // Send the block to all neighbours except the sender
                sendHash(block.getBlockHeaderHash(), peer->id);
                // sendBlock(block, peer->id);
            }
        }
    }
    if (isMalicious && block.minerID != ringMaster) {
        // Honest blocks are already in public
        blockchain->whether_sent_to_honest[block.getBlockHeaderHash()] = true;
    }
    logToPeerFile("RECEIVED BLOCK", "Peer " + to_string(id) + " (" + to_string(isMalicious) + ")" + " received block " + (block.getBlockHeaderHash()) + " (with parent id " + (block.parentHash) + ")" + " from peer " + to_string(sender_id) + " at time " + to_string(simulator->getCurrentTime())); // Log the event to the log file

    string new_leaf_node = blockchain->returnLeafNode();

    if (isMalicious && id != ringMaster) return;
    if (old_leaf_node != new_leaf_node) {
        simulator->scheduleEvent(simulator->getCurrentTime(), MINING_START, id, -1, {});
    }
    if (block.minerID != ringMaster) {
        broadcastPrivateChain(block.getBlockHeaderHash());
    }
}

void Peer::sendBlock(Block block, int targetPeerID) {
    // This function is called when a peer sends a block
    if(!enable_countermeasure || isMalicious) {
        bool whether_overlay = false;
        if (!peers[targetPeerID]->isMalicious && isMalicious) blockchain->whether_sent_to_honest[block.getBlockHeaderHash()] = true;
        if (malicious_neighbours.count(targetPeerID)) whether_overlay = true;
        simulator->scheduleEvent(simulator->getCurrentTime(), BLOCK_SEND, id, targetPeerID, block, whether_overlay);

    } else {
        if(trustScore[targetPeerID] > banThreshold) {
            double delayedSendTime = simulator->getCurrentTime() + getTrustDelay(targetPeerID);
            simulator->scheduleEvent(delayedSendTime, BLOCK_SEND, id, targetPeerID, block, false);

        } else {
            banCount[targetPeerID] += 1;
            if(banCount[targetPeerID] > maxBan) {
                resetScore(targetPeerID);
                banCount[targetPeerID] = 0;
            }
        }

    }
}

void Peer::setHashingPower() {
    // This function sets the hashing power of the peer, based on the number of peers and whether the peer is low CPU
    hashingPower = 1;
    hashingPower /= peers.size();
}


Peer::~Peer() {
    // Default destructor
    // cout << " checking " << endl;
    delete blockchain;
}

void Peer::createGenesisBlock() {
    // This function creates the genesis block and inserts it into the blockchain
    Block genesisBlock = Block(genesisHash);
    genesisBlock.parentHash = genesisHash;
    genesisBlock.hashBlockHeader = genesisHash;
    genesisBlock.minerID = -1;
    genesisBlock.height = 0;
    blockchain->current_leaf_node = genesisHash;
    blockchain->whether_sent_to_honest[genesisBlock.hashBlockHeader] = true;
    blockchain->insertBlock(genesisBlock, simulator->getCurrentTime());
}


string Peer::mining_start() {
    // This function is called when a peer starts mining
    current_mined_block = Block("");
    int count = 0;
    
    vector<double> peerBalancesRightNow(num_nodes); // Get the current balances of all peers
    for (int i = 0 ; i < num_nodes ; i ++ ) peerBalancesRightNow[i] = blockchain->getPeerBalance(i);

    for (Transaction txn : txPool) {
        if (peerBalancesRightNow[txn.sender] < txn.amount) {
            // If the sender doesn't have enough balance, ignore the transaction
            continue;
        }
        peerBalancesRightNow[txn.sender] -= txn.amount;
        current_mined_block.addTransaction(txn);
        count++; 
        if(count >= maxTransactionsPerBlock) {break;}  // If the block is full, stop adding transactions
    }
    current_mined_block.parentHash = blockchain->current_leaf_node; // Set the parent ID of the block
    current_mined_block.minerID = id; // Set the miner ID of the block
    current_mined_block.height = blockchain->getLongestChainHeight(); // Set the height of the block
    current_mined_block.timestamp_of_creation = simulator->getCurrentTime();
    current_mined_block.getBlockHeaderHash();
    leaf_node = blockchain->current_leaf_node; // Set the leaf node of the block
    return current_mined_block.getBlockHeaderHash();
}

void Peer::mining_end(string mined_hash) {
    // This function is called when a peer finishes mining and we need to insert the block into the blockchain
    if (current_mined_block.getBlockHeaderHash() != mined_hash) return;
    if (blockchain->current_leaf_node != leaf_node) return;  // If the leaf node has changed, ignore the block, and we need to start mining again
    
    blockchain->insertBlock(current_mined_block, simulator->getCurrentTime());
    for (auto& [id, peer]: malicious_neighbours)
    {
        sendHash(current_mined_block.getBlockHeaderHash(), peer->id);
    }
    if (id != ringMaster) {
        for (auto& [id, peer] : neighbours) 
        {
            sendHash(current_mined_block.getBlockHeaderHash(), peer->id);
        }
    }  
    if (id == ringMaster) {
        if (blockchain->whether_sent_to_honest[current_mined_block.parentHash]) {
            selfish_mine_start = current_mined_block.parentHash;
        }
    }
    if (id == ringMaster) {
        simulator->scheduleEvent(simulator->getCurrentTime(), MINING_START, id, -1, {});
    }
}

double Peer::getBlockInterArrivalTime() {
    // This function returns the block inter-arrival time
    return exponentialRandom( averageBlockArrivalTime / hashingPower );
}

void Peer::sendHash(string hash, int targetPeerID)
{
    if (!peers[targetPeerID]->isMalicious && isMalicious) blockchain->whether_sent_to_honest[hash] = true;
    bool whether_overlay = false;
    if (malicious_neighbours.count(targetPeerID)) whether_overlay = true;
    simulator->scheduleEvent(simulator->getCurrentTime(), HASH_SEND, id, targetPeerID, hash, whether_overlay);
}

void Peer::receiveHash(string hash, int sender_id)
{
    if (blockchain->blocks.count(hash)) {
        return;
    }
    blockchain->hash_to_queue[hash].push(sender_id);

    if (!blockchain->hash_to_timeout.count(hash) || blockchain->hash_to_timeout[hash] <= simulator->getCurrentTime()) {
        if(!enable_countermeasure || isMalicious) {
            blockchain->hash_to_timeout[hash] = simulator->getCurrentTime() + GetRequestTimeout;
            sendGetRequest(hash, sender_id);

        } else if(enable_countermeasure && !isMalicious) {
            if(trustScore[sender_id] > banThreshold) {
                double delayedRequestTime = simulator->getCurrentTime() + getTrustDelay(sender_id);
                blockchain->hash_to_timeout[hash] = GetRequestTimeout + delayedRequestTime;
                sendDelayedGetRequest(hash, sender_id, delayedRequestTime); 

            } else {
                banCount[sender_id] += 1;
                if(banCount[sender_id] > maxBan) {
                    resetScore(sender_id);
                    banCount[sender_id] = 0;
                }
            }
        } 
    }
}

/////////////////
void Peer::sendGetRequest(string hash, int targetPeerID)
{
    bool whether_overlay = false;
    if (malicious_neighbours.count(targetPeerID)) whether_overlay = true;
    simulator->scheduleEvent(simulator->getCurrentTime(), GET_SEND, id, targetPeerID, hash, whether_overlay);
}

void Peer::sendDelayedGetRequest(string hash, int targetPeerID, double delayedRequestTime)
{
    simulator->scheduleEvent(delayedRequestTime, GET_SEND, id, targetPeerID, hash, false);
}

void Peer::receiveGetRequest(string hash, int sender_id)
{
    if (whether_eclipse_attack && isMalicious && !peers[sender_id]->isMalicious && blockchain->blocks[hash].minerID != ringMaster) {
        return;
    }
    sendBlock(blockchain->blocks[hash], sender_id);
}

/////////////
void Peer::handleFailedRequest(int sender_id) {
    trustScore[sender_id] -= (10.0);
    trustScore[sender_id] = max(0.0, trustScore[sender_id]);

    pastAttempts[sender_id].second += 1;
    pastAttempts[sender_id].first -= 1;
    pastAttempts[sender_id].first = max(0, pastAttempts[sender_id].first);
}

/////////////
void Peer::handleSuccesfulRequest(int sender_id) {
    trustScore[sender_id] += 1;
    trustScore[sender_id] = min(maxTrustScore, trustScore[sender_id]);

    pastAttempts[sender_id].second += 1;
    pastAttempts[sender_id].first += 1;
}

/////////////
double Peer::maxTrustDelay() {
    double maxDelay = 20 * GetRequestTimeout;
    return maxDelay;
}

/////////////
double Peer::getTrustDelay(int sender_id) {
    double successRatio = (double)pastAttempts[sender_id].first / (double)pastAttempts[sender_id].second;
    double max_delay = maxTrustDelay();
    double delay = max_delay * (1 - successRatio * trustScore[sender_id] / maxTrustScore);
    return delay;
}

/////////////
void Peer::resetScore(int neighbour_id) {
    trustScore[neighbour_id] = maxTrustScore;
    pastAttempts[neighbour_id] = pair<int, int>(1, 1);
}

void Peer::reportTrust() { 
    string peerLogFile = "countermeasure/peer" + to_string(id);

    for(auto neighbour: neighbours) {
        int nid = neighbour.second->id;
        logToFile("- ", to_string(nid) + ": Score = " + to_string(trustScore[nid]) + " | H = " + to_string(pastAttempts[nid].first) + " " + to_string(pastAttempts[nid].second) , peerLogFile); 
    }
}

void Peer::handleTimeout(string hash) {
    // Pop queue and send next get request
    if (blockchain->blocks.count(hash) || !blockchain->hash_to_queue.count(hash)) {
        return;
    }
    if(!blockchain->hash_to_queue[hash].empty()) {
        int sender_id = blockchain->hash_to_queue[hash].front();
        handleFailedRequest(sender_id);

        if (debug) {
            cout << "Peer " << id << " didn't get response from peer " << sender_id << " for hash " << hash << endl;
        }

        blockchain->hash_to_queue[hash].pop();
    }
    if (blockchain->hash_to_queue[hash].empty()) {
        return;
    }

    sendGetRequest(hash, blockchain->hash_to_queue[hash].front());

    if (debug) {
        cout << "Peer " << id << " handled a timeout for hash " << hash << " at time " << simulator->getCurrentTime() << endl;
    }
}

void Peer::broadcastPrivateChain(string receivedHash)
{
    if (id != ringMaster) return;
    if (blockchain->blocks.count(receivedHash) == 0) return;
    if (blockchain->blocks[receivedHash].height <= blockchain->blocks[selfish_mine_start].height) return;
    int longestHonestChainHeight = 0, longestPrivateChainHeight = 0;
    for (auto block_checking : blockchain->blocks)
    {
        if (blockchain->whether_sent_to_honest[block_checking.first]) longestHonestChainHeight = max(longestHonestChainHeight, block_checking.second.height);
        else longestPrivateChainHeight = max(longestPrivateChainHeight, block_checking.second.height);
    }
    if ((longestHonestChainHeight == longestPrivateChainHeight) || (longestPrivateChainHeight == 1 + longestHonestChainHeight))
    {
        receivePrivateMessage("PRIVATE " + to_string(getBroadCastNumber()), id);
    }
}

void Peer::receivePrivateMessage(string message, int sender_id)
{
    int bid = stoi(message.substr(8));
    if (allBroadcastIDs.count(bid)) return;
    allBroadcastIDs.insert(bid);
    for (auto neighbour : malicious_neighbours)
    {
        if (neighbour.first != sender_id)
        simulator->scheduleEvent(simulator->getCurrentTime(), PRIVATE_MESSAGE_SEND, id, neighbour.first, message, true);
    }
    string current_node = blockchain->current_leaf_node;
    vector<string> hashes_to_be_sent;
    while (true)
    {
        if (blockchain->whether_sent_to_honest[current_node] || current_node == "") break;
        hashes_to_be_sent.push_back(current_node);
        current_node = blockchain->blocks[current_node].parentHash;
    }
    reverse(hashes_to_be_sent.begin(), hashes_to_be_sent.end());
    for (int i = 0 ; i < hashes_to_be_sent.size() ; i ++) {
        for (auto neighbour : neighbours) {
            if (!peers[neighbour.first]->isMalicious && neighbour.first != sender_id) sendHash(hashes_to_be_sent[i], neighbour.first);
        }
    }
}

void Peer::logToPeerFile(string action, string details) {
    string peerLogFile = "logFiles/logs" + to_string(id);
    logToFile(action, details, peerLogFile);
}