#ifndef HASH_H
#define HASH_H
#include<iostream>
#include<string>
#include<openssl/sha.h>
#include<sstream>
#include<iomanip>

using namespace std;

#include <openssl/sha.h>
string sha256(const string& data);

#endif