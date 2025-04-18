// Contains the transaction class
#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
using namespace std;

extern const int TransactionSize; // referring from main.cpp

class Transaction {
public:
    Transaction() = default;
    Transaction(int sender, int receiver, double amount);
    int getID() const;
    int getSize();
    string getString();
    int sender;
    int receiver;
    double amount;
    bool operator<(const Transaction& other) const {
        // For storing the transactions in a set, we have defined the order of transactions
        return id > other.id;
    }

private:
    int id;
};

#endif