#include "helper.h"

mt19937 gen(random_device{}()); // Mersenne Twister 19937 generator

int current_transaction_id = 0; // Transaction ID counter
int current_block_id = 0; // Block ID counter

int generateBlockID() {
    // Return block ID and increment counter
    return current_block_id++;
}

double exponentialRandom(double mean) {
    // Return random number from exponential distribution
    exponential_distribution<> dist(1.0 / mean);
    return dist(gen);
}

double uniformRandom(double min, double max) {
    // Return random number from uniform distribution
    uniform_real_distribution<> dist(min, max);
    return dist(gen);
}

double calculateLatency(bool isSlowI, bool isSlowJ, int messageLength, bool whether_overlay) {
    // For latency in seconds
    bool isFastI = !isSlowI;
    bool isFastJ = !isSlowJ;
    double cij = isFastI && isFastJ ? FAST_LINK_SPEED : SLOW_LINK_SPEED;
    double rhoij = uniformRandom(MIN_PROPAGATION_DELAY, MAX_PROPAGATION_DELAY);
    if (whether_overlay) rhoij = uniformRandom(MIN_OVERLAY_PROPAGATION_DELAY, MAX_OVERLAY_PROPAGATION_DELAY);
    double meanQueuingDelay = MEAN_QUEUING_DELAY_FACTOR / cij;
    double dij = exponentialRandom(meanQueuingDelay);
    double latency = rhoij + ((double)messageLength / cij) * 1e3 + dij * 1e3; // converting to milliseconds
    return latency / 1e3; 
}

int generateTransactionID() {
    // Return transaction ID and increment counter
    return current_transaction_id++;
}

vector<int> getSlowNodeSubset(int numNodes, double slowNodePercentage) {
    // Return subset of slow nodes
    if (numNodes <= 0) {
        throw invalid_argument("Number of nodes must be positive.");
    }
    slowNodePercentage /= 100;
    int numSlowNodes = ceil(numNodes * slowNodePercentage);
    vector<int> indices(numNodes);
    iota(indices.begin(), indices.end(), 0);
    shuffle(indices.begin(), indices.end(), gen); // Shuffling and picking the first numSlowNodes
    return vector<int>(indices.begin(), indices.begin() + numSlowNodes);
}

void clearLogFile(string logFolderPath) {
    // Clear all files in the logFiles folder, create folder if it does not exist
    // const string logFolderPath = "logFiles";
    if (!filesystem::exists(logFolderPath)) {
        try {
            filesystem::create_directory(logFolderPath);
        } catch (const filesystem::filesystem_error& e) {
            cerr << "[ERROR] Unable to create log folder: " << logFolderPath << " - " << e.what() << endl;
            return;
        }
    }
    for (const auto& entry : filesystem::directory_iterator(logFolderPath)) {
        try {
            filesystem::remove(entry.path());
        } catch (const filesystem::filesystem_error& e) {
            cerr << "[ERROR] Unable to delete file: " << entry.path() << " - " << e.what() << endl;
        }
    }
}

void logToFile(string level, string message, string filePath) {
    // Log message to file, create file if it does not exist
    ofstream logFile(filePath, ios::app); // Open file in append mode
    if (logFile.is_open()) {
        logFile << "[" << level << "] " << message << endl;
        logFile.close();
    } else {
        // Attempt to create the file if it does not exist
        ofstream newFile(filePath);
        if (newFile.is_open()) {
            newFile << "[" << level << "] " << message << endl;
            newFile.close();
        } else {
            cerr << "[ERROR] Unable to create or open log file!" << endl;
        }
    }
}

void add_graph_to_file(string networkType, vector<pair<int, int>> edges) {
    string filePath = "drawNetwork/" + networkType + ".txt";

    if (filesystem::exists(filePath)) {
        try {
            filesystem::remove(filePath);
        } catch (const filesystem::filesystem_error& e) {
            cerr << "[ERROR] Unable to delete file: " << filePath << " - " << e.what() << endl;
            return;
        }
    }

    ofstream outFile(filePath, ios::out); 
    if (!outFile.is_open()) {
        cerr << "[ERROR] Unable to open file: " << filePath << endl;
        return;
    }

    
    for (const auto& edge : edges) {
        outFile << edge.first << " " << edge.second << endl;
    }
    outFile.close();
}

void add_peers_to_file(vector<int> malicious_peers) {
    string filePath = "drawNetwork/malicious_peers.txt";

    sort(malicious_peers.begin(), malicious_peers.end());
    if (filesystem::exists(filePath)) {
        try {
            filesystem::remove(filePath);
        } catch (const filesystem::filesystem_error& e) {
            cerr << "[ERROR] Unable to delete file: " << filePath << " - " << e.what() << endl;
            return;
        }
    }

    ofstream outFile(filePath, ios::out); 
    if (!outFile.is_open()) {
        cerr << "[ERROR] Unable to open file: " << filePath << endl;
        return;
    }

    
    for (const auto& peer : malicious_peers) {
        outFile << peer << endl;
    }
    outFile.close();
}

void generateConnectedGraph() {
    // Generate connected graph
    vector<int> peer_ids;
    for (int i = 0 ; i < num_nodes ; i ++ ) peer_ids.push_back(peers[i]->id);
    vector<pair<int, int>> edges;
    int num_peers = peer_ids.size();

    int minPossibleDegree = min(3, num_peers - 1);

    if(num_peers < 2) return; // No edges to add
    vector<set<int>> adj_list(num_peers);
    vector<int> shuffled_ids = peer_ids;
    shuffle(shuffled_ids.begin(), shuffled_ids.end(), gen);
    for(int i = 1; i < num_peers; i++) {
        // Adding edges between shuffled peers
        int peer1 = shuffled_ids[i - 1],
            peer2 = shuffled_ids[i];
        adj_list[peer1].insert(peer2);
        adj_list[peer2].insert(peer1);
        edges.emplace_back(peer1, peer2);
    }
    for(int i = 0; i < num_peers; i++) {
        // Adding more edges to make the graph connected and ensuring that each peer has at least 3 neighbours and not more than 6
        int curr_peer = peer_ids[i];
        while(adj_list[curr_peer].size() < minPossibleDegree) {
            int rand_peer = peer_ids[rand() % num_peers];
            if(rand_peer != curr_peer && adj_list[curr_peer].find(rand_peer) == adj_list[curr_peer].end() && adj_list[rand_peer].size() < 6) {
                adj_list[curr_peer].insert(rand_peer);
                adj_list[rand_peer].insert(curr_peer);
                edges.emplace_back(curr_peer, rand_peer);
            }
        }
    }
    for (int i = 0 ; i < edges.size() ; i ++ ) {
        // Updating the neighbours of the peers
        int peer1 = edges[i].first;
        int peer2 = edges[i].second;
        peers[peer1]->neighbours[peer2] = peers[peer2];
        peers[peer2]->neighbours[peer1] = peers[peer1];

        if(enable_countermeasure) {
            peers[peer1]->trustScore[peer2] = maxTrustScore;
            peers[peer2]->trustScore[peer1] = maxTrustScore;

            peers[peer1]->pastAttempts[peer2] = pair<int, int>(1, 1);
            peers[peer2]->pastAttempts[peer1] = pair<int, int>(1, 1);

            peers[peer1]->banCount[peer2] = 0;
            peers[peer2]->banCount[peer1] = 0;
        }   
    }
    for (int i = 0 ; i < peers.size() ; i ++ ) peers[i]->setHashingPower(); // Setting the hashing power of the peers

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    shuffled_ids.clear();

    int num_malicious = (malicious_percentage / 100) * num_peers;

    minPossibleDegree = min(num_malicious - 1, 3);
    
    vector<int> malicious_peers = peer_ids;
    shuffle(malicious_peers.begin(), malicious_peers.end(), gen);
    malicious_peers.resize(num_malicious);

    // Output the selected random peers
    for (auto id : malicious_peers) {
        peers[id] -> isMalicious = true;
    }
    map<int, set<int>> malicious_adj_list;
    vector<pair<int, int>> malicious_edges;

    shuffled_ids = malicious_peers;
    shuffle(shuffled_ids.begin(), shuffled_ids.end(), gen);
    
    for(int i = 1; i < num_malicious; i++) {
        // Adding edges between shuffled peers
        int peer1 = shuffled_ids[i - 1], peer2 = shuffled_ids[i];
        malicious_adj_list[peer1].insert(peer2);
        malicious_adj_list[peer2].insert(peer1);
        malicious_edges.emplace_back(peer1, peer2);
    }
    
    for(int i = 0; i < num_malicious; i++) {
        // Adding more edges to make the graph connected and ensuring that each peer has at least 3 neighbours and not more than 6
        int curr_peer = malicious_peers[i];
        while(malicious_adj_list[curr_peer].size() < minPossibleDegree) {
            int rand_peer = malicious_peers[rand() % num_malicious];
            if(rand_peer != curr_peer && malicious_adj_list[curr_peer].find(rand_peer) == malicious_adj_list[curr_peer].end() && malicious_adj_list[rand_peer].size() < 6) {
                malicious_adj_list[curr_peer].insert(rand_peer);
                malicious_adj_list[rand_peer].insert(curr_peer);
                malicious_edges.emplace_back(curr_peer, rand_peer);
            }
        }
    }

    for (int i = 0 ; i < malicious_edges.size() ; i ++ ) {
        // Updating the neighbours of the peers
        int peer1 = malicious_edges[i].first;
        int peer2 = malicious_edges[i].second;
        peers[peer1]->malicious_neighbours[peer2] = peers[peer2];
        peers[peer2]->malicious_neighbours[peer1] = peers[peer1];
    }

    if(show_network) {
        add_graph_to_file("normal_network", edges);
        add_peers_to_file(malicious_peers);
        add_graph_to_file("overlay_network", malicious_edges);
        string command = "python3 drawNetwork/draw.py";
        int ret_code = system(command.c_str());
    }
}

int getBroadCastNumber() 
{
    return broadcastnumber++;
}

int ratio_cal () {
    auto ringmasterChain = peers[ringMaster]->blockchain;
    int numMaliciousBlocks_chain = 0;
    int numHonestBlocks_chain = 0;
    int totalMaliciousBlocks = 0;
    vector<Block> longestChain = ringmasterChain->currentChain();

    for (Block b : longestChain) {
        if(peers[b.minerID]->isMalicious) 
            numMaliciousBlocks_chain += 1;
            
        else 
            numHonestBlocks_chain += 1;
        
    }

    double ratio_malicious_total = (double)numMaliciousBlocks_chain / ((double)numMaliciousBlocks_chain + numHonestBlocks_chain);

    string genesisId = genesisHash;
    queue<string> blocks;
    blocks.push(genesisId);

    while(!blocks.empty()) {
        string parent_id = blocks.front();
        blocks.pop();

        auto child_hashes = ringmasterChain->children_block_ids[parent_id];
        for(string child_id: child_hashes) {
            Block childBlock = ringmasterChain->blocks[child_id];
            if(peers[childBlock.minerID]->isMalicious) {
                totalMaliciousBlocks += 1;
            }

            blocks.push(child_id);
        }
    }

    double ratio_malicious_totalMalicious = (double)numMaliciousBlocks_chain / (double)totalMaliciousBlocks;

    cout<<"Malicious Chain: "<<numMaliciousBlocks_chain<<endl;
    cout<<"Total Blocks Chain: "<<(numMaliciousBlocks_chain + numHonestBlocks_chain)<<endl;
    cout<<"Total Malicious: "<<totalMaliciousBlocks<<endl;
    cout<<"Malicious Blocks in Chain / Total Blocks in Longest Chain: "<<ratio_malicious_total<<endl;
    cout<<"Malicious Blocks in Chain / Total Malicious Blocks: "<<ratio_malicious_totalMalicious<<endl;
    return 1;
}

void handlePostRunFlags() {
    // Handle post run flags

    clearLogFile("countermeasure");
    if(enable_countermeasure) {
        for(auto peer: peers) {
            peer->reportTrust();
        }
    }

    if(whether_ratio) {
        ratio_cal();
    }

    // if (whether_branches) {
    //     vector<int> branch_heights;
    //     for (const auto& b : peers[0]->blockchain->leafBlocks) {
    //         branch_heights.push_back(peers[0]->blockchain->blocks[b].height);
    //     }
    //     sort(branch_heights.begin(), branch_heights.end());
    //     cout << "Branch Heights:- ";
    //     for (int i = 0; i < branch_heights.size(); i++) {
    //         cout << branch_heights[i] << " ";
    //     }
    //     cout << endl;
    // }

    if (whether_blockchain) {
        blockchain_print();
        string command = "python3 plot_blockchain.py";
        int ret_code = system(command.c_str());
    }

    for(int i = 0; i < num_nodes; i++) {
        delete peers[i]; // Delete the peer objects
    }
}

void blockchain_print() {
    system(("rm -rf " + BlockChainSaveDirectory).c_str()); // Remove all previous data
    system(("mkdir " + BlockChainSaveDirectory).c_str()); // Making a new directory
    if (whether_dump_all) {
        for (int i = 0 ; i < num_nodes ; i ++ ) {
            peers[i]->blockchain->saveBlockChain(BlockChainSaveDirectory + to_string(i) + ".txt");
        }
    } else if (ringMaster != -1) {
        peers[ringMaster]->blockchain->saveBlockChain(BlockChainSaveDirectory + "ringmaster.txt");
    }
}

