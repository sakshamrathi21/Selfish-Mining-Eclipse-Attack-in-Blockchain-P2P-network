/* This file contains the event class */
#ifndef EVENT_H
#define EVENT_H

#include "transaction.h"
#include "block.h"
#include <variant>
using namespace std;

typedef variant<Transaction, Block, string> EventData; // type safe union

// Enum to represent the type of the event. It creates a mapping between the event type and a string.
enum EventType {
    CREATE_TRANSACTION,
    TRANSACTION_SEND,
    TRANSACTION_RECEIVE,
    MINING_START,
    MINING_END,
    BLOCK_SEND,
    BLOCK_RECEIVE,
    GET_SEND,
    GET_RECEIVE,
    HASH_SEND,
    HASH_RECEIVE,
    HANDLE_TIMEOUT,
    PRIVATE_MESSAGE_SEND,
    PRIVATE_MESSAGE_RECEIVE,
};

struct Event {
    double time;           
    EventType type;        
    int sourcePeer;        
    int targetPeer;       
    EventData data;
    bool whether_overlay = false;       

    Event(double time, EventType type, int sourcePeer, int targetPeer, EventData data)
        : time(time), type(type), sourcePeer(sourcePeer), targetPeer(targetPeer), data(data) {}
    
    Event(double time, EventType type, int sourcePeer, int targetPeer, EventData data, bool whether_overlay)
        : time(time), type(type), sourcePeer(sourcePeer), targetPeer(targetPeer), data(data), whether_overlay(whether_overlay) {}

    bool operator<(const Event& other) const {
        // For sorting the events in the priority queue
        return time > other.time; 
    }
    string eventTypeToString() {
        /* This function creates a mapping between the type of the event and a string, which can be used for printing */
        switch (type) {
            case CREATE_TRANSACTION: return "CREATE_TRANSACTION";
            case TRANSACTION_SEND: return "TRANSACTION_SEND";
            case TRANSACTION_RECEIVE: return "TRANSACTION_RECEIVE";
            case MINING_START: return "MINING_START";
            case MINING_END: return "MINING_END";
            case BLOCK_SEND: return "BLOCK_SEND";
            case BLOCK_RECEIVE: return "BLOCK_RECEIVE";
            case GET_SEND: return "GET_SEND";
            case GET_RECEIVE: return "GET_RECEIVE";
            case HASH_RECEIVE: return "HASH_RECEIVE";
            case HASH_SEND: return "HASH_SEND";
            case HANDLE_TIMEOUT: return "HANDLE_TIMEOUT";
            case PRIVATE_MESSAGE_SEND: return "PRIVATE_MESSAGE_SEND";
            case PRIVATE_MESSAGE_RECEIVE: return "PRIVATE_MESSAGE_RECEIVE";
            default: return "UNKNOWN_EVENT";
        }
    }

    friend std::ostream& operator<<(std::ostream& os, Event& obj) {
        if (obj.type == CREATE_TRANSACTION) {
            os << "Peer " << obj.sourcePeer << " created a transaction";
        }
        else if (obj.type == TRANSACTION_SEND) {
            os << "Peer " << obj.sourcePeer << " sent a transaction to Peer " << obj.targetPeer;
        }
        else if (obj.type == TRANSACTION_RECEIVE) {
            os << "Peer " << obj.sourcePeer << " received a transaction from Peer " << obj.targetPeer;
        }
        else if (obj.type == MINING_START) {
            os << "Peer " << obj.sourcePeer << " started mining";
        }
        else if (obj.type == MINING_END) {
            os << "Peer " << obj.sourcePeer << " finished mining block with hash " << get<string>(obj.data);
        }
        else if (obj.type == BLOCK_SEND) {
            os << "Peer " << obj.sourcePeer << " sent a block to Peer " << obj.targetPeer << " with hash " << get<Block>(obj.data).getBlockHeaderHash();
        }
        else if (obj.type == BLOCK_RECEIVE) {
            os << "Peer " << obj.targetPeer << " received a block from Peer " << obj.sourcePeer << " with hash " << get<Block>(obj.data).getBlockHeaderHash();
        }
        else if (obj.type == GET_SEND) {
            os << "Peer " << obj.sourcePeer << " sent a GET request to Peer " << obj.targetPeer << " for hash " << get<string>(obj.data);
        }
        else if (obj.type == GET_RECEIVE) {
            os << "Peer " << obj.targetPeer << " received a GET request from Peer " << obj.sourcePeer << " for hash " << get<string>(obj.data);
        }
        else if (obj.type == HASH_SEND) {
            os << "Peer " << obj.sourcePeer << " sent a hash " << get<string>(obj.data) << " to Peer " << obj.targetPeer;
        }
        else if (obj.type == HASH_RECEIVE) {
            os << "Peer " << obj.targetPeer << " received a hash " << get<string>(obj.data) <<  " from Peer " << obj.sourcePeer;
        }
        else if (obj.type == HANDLE_TIMEOUT) {
            os << "Peer " << obj.sourcePeer << " handled a timeout for hash " << get<string>(obj.data);
        }
        else if (obj.type == PRIVATE_MESSAGE_SEND) {
            os << "Peer " << obj.sourcePeer << " sent a private message to Peer " << obj.targetPeer;
        }
        else if (obj.type == PRIVATE_MESSAGE_RECEIVE) {
            os << "Peer " << obj.targetPeer << " received a private message from Peer " << obj.sourcePeer;
        }
        else {
            os << "Unknown event type.";
        }
        os << " at time " << obj.time << endl;
        return os;
    }

};

#endif 