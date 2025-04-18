#include "simulator.h"

void Simulator::run() {
    // This function is the main function that runs the simulation
    for (int i = 0 ; i < num_nodes ; i++ ) {
        // Create genesis block for each peer
        peers[i]->createGenesisBlock();
        peers[i]->blockchain->whether_sent_to_honest[genesisHash] = true;
    }
    for (int i = 0 ; i < num_nodes ; i ++ ) {
        double interArrivalTime = getInterArrivalTime();
        scheduleEvent(interArrivalTime, CREATE_TRANSACTION, i, -1, {}); // Schedule the first transaction for each peer
    }
    for (int i = 0 ; i < num_nodes ; i ++) 
    {
        if (!peers[i]->isMalicious || i == ringMaster)
        {
            scheduleEvent(currentTime, MINING_START, i, -1, {}); // Schedule the first mining event
        }
    }
    
    while ( !eventQueue.empty() && currentTime <= totalExecutionTime ){
        // Run the simulation until the event queue is empty
        Event current = eventQueue.top();
        if (debug && current.type != CREATE_TRANSACTION && current.type != TRANSACTION_SEND && current.type != TRANSACTION_RECEIVE && current.type != HANDLE_TIMEOUT) {
            cout << current;
        }
        eventQueue.pop();
        currentTime = current.time;
        handleEvent(current);
    }

    if (debug) {
        cout << "Starting post simulation broadcast" << endl;
    }
    while (!eventQueue.empty()) eventQueue.pop();
    currentTime = totalExecutionTime;
    peers[ringMaster]->receivePrivateMessage("PRIVATE " + to_string(getBroadCastNumber()), ringMaster);

    while ( !eventQueue.empty() ) {
        Event current = eventQueue.top();
        eventQueue.pop();
        currentTime = current.time;

        if (current.type == CREATE_TRANSACTION
            || current.type == TRANSACTION_SEND
            || current.type == TRANSACTION_RECEIVE
            || current.type == MINING_START
            || current.type == MINING_END) {
            continue; // Skip these events in the final output
        }

        if (debug && current.type != CREATE_TRANSACTION && current.type != TRANSACTION_SEND && current.type != TRANSACTION_RECEIVE && current.type != HANDLE_TIMEOUT) {
            cout << current;
        }

        handleEvent(current);
    }
}

void Simulator::handleEvent( Event& event ) {
    // This function handles the event based on the event type
    double newTime;
    string current_mined_hash;
    switch (event.type) {
        case MINING_START:
            current_mined_hash = peers[event.sourcePeer]->mining_start();
            newTime = currentTime + peers[event.sourcePeer]->getBlockInterArrivalTime();
            scheduleEvent(newTime, MINING_END, event.sourcePeer, -1, current_mined_hash); // Schedule the mining end event 
            break;
        case MINING_END:
            peers[event.sourcePeer]->mining_end(get<string>(event.data));
            break;
        case CREATE_TRANSACTION:
            peers[event.sourcePeer]->generateTransaction();
            newTime = currentTime + getInterArrivalTime();
            scheduleEvent(newTime, CREATE_TRANSACTION, event.sourcePeer, -1, event.data); // Schedule the next transaction
            break;
        case TRANSACTION_SEND:
            newTime = currentTime + calculateLatency(!peers[event.sourcePeer]->isMalicious, !peers[event.targetPeer]->isMalicious, get<Transaction>(event.data).getSize(), event.whether_overlay);
            scheduleEvent(newTime, TRANSACTION_RECEIVE, event.sourcePeer, event.targetPeer, event.data); // Schedule the transaction receive event
            break;
        case TRANSACTION_RECEIVE:
            peers[event.targetPeer]->receiveTransaction(get<Transaction>(event.data), event.sourcePeer); // Receive the transaction
            break;
        case BLOCK_SEND:
            newTime = currentTime + calculateLatency(!peers[event.sourcePeer]->isMalicious, !peers[event.targetPeer]->isMalicious, get<Block>(event.data).getBlocksize(), event.whether_overlay);
            scheduleEvent(newTime, BLOCK_RECEIVE, event.sourcePeer, event.targetPeer, event.data);
            break;
        case BLOCK_RECEIVE:
            peers[event.targetPeer]->receiveBlock(get<Block>(event.data), event.sourcePeer);
            break;
        case GET_SEND:
            newTime = currentTime + calculateLatency(!peers[event.sourcePeer]->isMalicious, !peers[event.targetPeer]->isMalicious, getSize, event.whether_overlay);
            scheduleEvent(newTime, GET_RECEIVE, event.sourcePeer, event.targetPeer, event.data);
            newTime = currentTime + GetRequestTimeout;
            scheduleEvent(newTime, HANDLE_TIMEOUT, event.sourcePeer, event.targetPeer, event.data);
            break;
        case GET_RECEIVE:
            peers[event.targetPeer]->receiveGetRequest(get<string>(event.data), event.sourcePeer);
            break;
        case HASH_SEND:
            newTime = currentTime + calculateLatency(!peers[event.sourcePeer]->isMalicious, !peers[event.targetPeer]->isMalicious, hashSize, event.whether_overlay);
            scheduleEvent(newTime, HASH_RECEIVE, event.sourcePeer, event.targetPeer, event.data);
            break;
        case HASH_RECEIVE:
            peers[event.targetPeer]->receiveHash(get<string>(event.data), event.sourcePeer);
            break;
        case HANDLE_TIMEOUT:
            peers[event.sourcePeer]->handleTimeout(get<string>(event.data));
            break;
        case PRIVATE_MESSAGE_RECEIVE:
            peers[event.targetPeer]->receivePrivateMessage(get<string>(event.data), event.sourcePeer);
            break;
        case PRIVATE_MESSAGE_SEND:
            newTime = currentTime + calculateLatency(!peers[event.sourcePeer]->isMalicious, !peers[event.targetPeer]->isMalicious, broadcastPrivateChainSize, event.whether_overlay);
            scheduleEvent(newTime, PRIVATE_MESSAGE_RECEIVE, event.sourcePeer, event.targetPeer, event.data);
            break;
    }
}

void Simulator::scheduleEvent(double time, EventType type, int sourcePeer, int targetPeer, EventData data, bool whether_overlay) {
    Event newEvent = Event(time, type, sourcePeer, targetPeer, data, whether_overlay);
    // Push the event to the event queue
    eventQueue.push(newEvent);
}

double Simulator::getInterArrivalTime() { 
    return exponentialRandom(meanTime); 
}