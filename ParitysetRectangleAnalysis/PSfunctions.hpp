//
//  PSfunctions.hpp
//  ParitysetRectangleAnalysis
//
//  Created by Frederik Wedel-Heinen on 10/31/16.
//  Copyright © 2016 Frederik Wedel-Heinen. All rights reserved.
//

#ifndef PSfunctions_hpp
#define PSfunctions_hpp

#include <stdio.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <ctime>
#include <stdlib.h>
#include <thread>
#include <mutex>
#include <sys/time.h>
#include  <unordered_set>

using namespace std;
void parity(vector<uint64_t> parityset,string cipher, int boundDifference);
void printtofile(vector<uint64_t> parityset);
vector<uint64_t> readfromfile(string filepath);
void parityFULL(vector<uint64_t> parityset, string cipher, int numberOfRounds);
#endif /* PSfunctions_hpp */
