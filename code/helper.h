/* This file contains some of the helpful functions */
#ifndef HELPER_H
#define HELPER_H

#include <iostream>
#include <openssl/sha.h>
#include <random>
#include <cmath>
#include <algorithm>
#include <vector>
#include <chrono>
#include <fstream>
#include <string>
#include "peer.h"
#include <filesystem>
using namespace std;

extern int num_nodes; // referring from main.cpp

extern int current_transaction_id;  
extern mt19937 gen;      
extern int current_block_id; 
extern double malicious_percentage; 
extern int broadcastnumber;
extern const string BlockChainSaveDirectory;

class Peer;  // Forward declaration of the class Peer
extern vector<Peer*> peers;
extern bool whether_ratio;
extern bool whether_longest_chain_height;
extern bool whether_blockchain;
extern bool whether_branches;
extern bool show_network;
extern bool enable_countermeasure;
extern bool whether_dump_all;

// Some of the important constants to calculate the latency
constexpr double FAST_LINK_SPEED = 100e6; // in bits per second
constexpr double SLOW_LINK_SPEED = 5e6; // in bits per second
constexpr double MIN_PROPAGATION_DELAY = 10; // in milliseconds
constexpr double MIN_OVERLAY_PROPAGATION_DELAY = 1; // in milliseconds
constexpr double MAX_OVERLAY_PROPAGATION_DELAY = 10; // in milliseconds
constexpr double MAX_PROPAGATION_DELAY = 500; // in milliseconds
constexpr double MEAN_QUEUING_DELAY_FACTOR = 96e3; // in bits
constexpr double maxTrustScore = 100;
constexpr double banThreshold = 20;
constexpr double maxBan = 5;

int generateTransactionID();
int generateBlockID();
vector<int> getSlowNodeSubset(int numNodes, double slowNodePercentage);
double uniformRandom(double min, double max);
double exponentialRandom(double mean);
double calculateLatency(bool isFastI, bool isFastJ, int messageLength, bool whether_overlay = false);
void logToFile(string level, string message, string filePath);
void clearLogFile(string filePath);
void generateConnectedGraph();
string sha256(const string& data);
int getBroadCastNumber();
void blockchain_print();
void handlePostRunFlags();

#endif