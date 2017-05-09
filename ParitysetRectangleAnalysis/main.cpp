//
//  main.cpp
//  ParitysetRectangleAnalysis
//
//  Created by Frederik Wedel-Heinen on 10/12/16.
//  Copyright © 2016 Frederik Wedel-Heinen. All rights reserved.
//

#include <iostream>
#include "PSfunctions.hpp"

int main(int argc, const char * argv[]) {
    
    vector<uint64_t> parityset;
    parityset.push_back(0xfffffffffffffffd);
    parity(parityset, "Rectangle", 2);
    //parityset.push_back(0xb00b);
    //parityFULL(parityset, "Rectangle", 4);
    return 0;
}
