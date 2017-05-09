//
//  PSfunctions.cpp
//  ParitysetRectangleAnalysis
//
//  Created by Frederik Wedel-Heinen on 10/31/16.
//  Copyright © 2016 Frederik Wedel-Heinen. All rights reserved.
//

#include "PSfunctions.hpp"
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <ctime>
#include <stdlib.h>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <sys/time.h>
#include  <unordered_set>
#include <fstream>

using namespace std;
mutex mutx;
struct nextnumber{
    uint64_t val=0;
    int nexthw=0;
};
int nexthwRectangle[16]={0,1,1,1,1,1,1,1,1,1,1,2,1,2,1,4};
int nexthwPresent[16]={0,1,1,1,1,1,1,1,1,1,1,1,1,1,2,4};
int nexthwG0[16]={0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,4};
int nexthwG0inverse[16]={0,1,1,1,1,1,1,1,1,1,1,1,1,2,2,4};
static unsigned bitcount (uint64_t n)
{
    //if(sizeof(n)==sizeof(int)) return __builtin_popcount(n);
    //if(sizeof(n)==sizeof(long)) return __builtin_popcountl(n);
    return __builtin_popcountll(n);
}
bool hwcomp (uint64_t i,uint64_t j) { return (bitcount(i)<bitcount(j)); }


void printtofile(vector<uint64_t> parityset){
    string sizle = to_string(parityset.size()), filepath = "VectorSize" + sizle + ".txt";
    ofstream file;
    file.open(filepath);
    file << parityset[0];
    for (int i = 1; i<parityset.size(); i++) {
        file << endl;
        file << parityset[i];
    }
    file.close();
}

vector<uint64_t> readfromfile(string filepath){
    vector<uint64_t> parityset;
    uint64_t val;
    string str;
    ifstream file;
    file.open(filepath);
    while (getline(file, str)) {
        file >> val;
        parityset.push_back(val);
    }
    file.close();
    vector<size_t> del;
    uint64_t numThreads = 4;//thread::hardware_concurrency();
    vector<thread> threads(numThreads);
    uint64_t smallesthw = bitcount(parityset[0]);
    for (uint64_t threadID = 0 ; threadID < numThreads ; ++threadID){
        // Create thread
        threads[threadID] = thread([threadID,numThreads, smallesthw,&parityset,&del](){
            size_t bit=0,start=0;
            for (int j = parityset.size()-1-threadID; j>0; j-=numThreads) {
                if(bit!=bitcount(parityset[j])){
                    bit=bitcount(parityset[j]);
                    if(bit==smallesthw){ break;}
                    start = lower_bound(parityset.begin(), parityset.end(), parityset[j], hwcomp)-parityset.begin();
                }
                for (size_t i = 1; start+1>i; i++) {
                    if (((parityset[j] & parityset[i-1]) == parityset[i-1])&&(i-1!=j)) {
                        mutx.lock();
                        del.push_back(j);
                        mutx.unlock();
                        break;
                    }
                }
                
            }});
    }
    for (uint64_t threadID = 0 ; threadID < numThreads ; ++threadID){
        threads[threadID].join();
    }
    sort(del.begin(), del.end());
    for (int j = del.size()-1; j>=0; j--) {
        parityset.erase(parityset.begin()+del[j]);
    }
    return parityset;
}

nextnumber numberCalcRectangle(uint64_t number2){
    nextnumber nn;
    uint64_t nibbleval;
    for (int nibble = 0; nibble<16; nibble++) {
        nibbleval = (number2 >> 4*nibble) & 0xf;
        if(nibbleval==1||nibbleval==2||nibbleval==4||nibbleval==8){
            nn.val ^= (uint64_t)1 << 4*nibble;
        }else if(nibbleval==5||nibbleval==6||nibbleval==12){
            nn.val ^= (uint64_t)5 << 4*nibble;
        }else if(nibbleval==9||nibbleval==14){
            nn.val ^= (uint64_t)9 << 4*nibble;
        }else{
            nn.val ^= (uint64_t)nibbleval << 4*nibble;
        }
        nn.nexthw += nexthwRectangle[nibbleval];
    }
    return nn;
}
nextnumber numberCalcPresent(uint64_t number2){
    nextnumber nn;
    uint64_t nibbleval;
    for (int nibble = 0; nibble<16; nibble++) {
            nibbleval = (number2 >> 4*nibble) & 0xf;
            if(nibbleval==1||nibbleval==2||nibbleval==4||nibbleval==8){
                nn.val ^= (uint64_t)1 << 4*nibble;
            }else if(nibbleval==5||nibbleval==3||nibbleval==9||nibbleval==10||nibbleval==11||nibbleval==12){
                nn.val ^= (uint64_t)3 << 4*nibble;
            }else {
                nn.val ^= (uint64_t)nibbleval << 4*nibble;
            }
            nn.nexthw += nexthwPresent[nibbleval];
    }
    return nn;
}
nextnumber numberCalcG0(uint64_t number2){
    nextnumber nn;
    uint64_t nibbleval;
    for (int nibble = 0; nibble<16; nibble++) {
        nibbleval = (number2 >> 4*nibble) & 0xf;
        if(nibbleval==1||nibbleval==2||nibbleval==4||nibbleval==8||nibbleval==0xc){
            nn.val ^= (uint64_t)1 << 4*nibble;
        }else if(nibbleval==5||nibbleval==3||nibbleval==9||nibbleval==10){
            nn.val ^= (uint64_t)3 << 4*nibble;
        }else {
            nn.val ^= (uint64_t)nibbleval << 4*nibble;
        }
        nn.nexthw += nexthwG0[nibbleval];
    }
    return nn;
}
nextnumber numberCalcG0inverse(uint64_t number2){
    nextnumber nn;
    uint64_t nibbleval;
    for (int nibble = 0; nibble<16; nibble++) {
        nibbleval = (number2 >> 4*nibble) & 0xf;
        if(nibbleval==1||nibbleval==2||nibbleval==4||nibbleval==8||nibbleval==3||nibbleval==10){
            nn.val ^= (uint64_t)1 << 4*nibble;
        }else if(nibbleval==0xb||nibbleval==9){
            nn.val ^= (uint64_t)9 << 4*nibble;
        }else if(nibbleval==7||nibbleval==0xc){
            nn.val ^= (uint64_t)7 << 4*nibble;
        }else {
            nn.val ^= (uint64_t)nibbleval << 4*nibble;
        }
        nn.nexthw += nexthwG0inverse[nibbleval];
    }
    return nn;
}
vector<uint64_t> SPlayerParFULL(const vector<uint64_t> parityset, const vector<vector<uint64_t>> Vs, const int permute[64], string cipher){
    
    vector<uint64_t> parityupdate;
    vector<int> nibblevals;
    int smallesthw = 100,smallestnexthw=100;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    uint64_t numThreads = 8;//thread::hardware_concurrency();
    vector<thread> threads(numThreads);
    
    
    for (uint64_t threadID = 0 ; threadID < numThreads ; ++threadID){
        // Create thread
        threads[threadID] = std::thread([threadID,numThreads,&parityset,&parityupdate,&cipher,Vs,permute](){
            for (uint64_t i = threadID ; i < parityset.size() ; i+=numThreads){
                vector<int> nibblevals;
                for (int nibble = 0; nibble<16; nibble++) {
                    nibblevals.push_back((parityset[i] >> 4*nibble) & 0xf);
                }
                
                vector<int> T(16,0);
                
                while (T[15]<Vs[nibblevals[15]].size()) {
                    // Create the parityvalue after the sbox and permutation
                    
                    uint64_t number = 0, number2 = 0, nibbleval, val;
                    for (int j = 0; j<T.size(); j++) {
                        nibbleval = Vs[nibblevals[j]][T[j]];
                        number ^= nibbleval << 4*j;
                    }
                    
                    for (int ii = 0; ii<64; ii++) {
                        number2 ^= ((number>>ii)&1)<<permute[ii];
                    }
                    
                    mutx.lock();
                    parityupdate.push_back(number2);
                    mutx.unlock();
                    
                    T[0]++;
                    for (int j = 1; j<T.size(); j++) {
                        if (T[j-1] == Vs[nibblevals[j-1]].size()) {
                            T[j-1] = 0;
                            T[j]++;
                        }
                    }
                }//Counter loop end
            }
        });//parityset loop end
    }
    for (uint64_t threadID = 0 ; threadID < numThreads ; ++threadID){
        threads[threadID].join();
    }
    gettimeofday(&end, NULL);
    sort( parityupdate.begin(), parityupdate.end() );
    parityupdate.erase( unique( parityupdate.begin(), parityupdate.end() ), parityupdate.end() );
    cout << "SPLayer done in " << dec << (end.tv_sec  - start.tv_sec) << " seconds" << endl;
    return parityupdate;
}
vector<uint64_t> SPlayerPar(const vector<uint64_t> parityset, const vector<vector<uint64_t>> Vs, const int permute[64], string cipher, int boundDifference){
    
    vector<uint64_t> parityupdate;
    vector<int> nibblevals;
    int smallesthw = 100,smallestnexthw=100;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    uint64_t numThreadsParityupdate = 8;//thread::hardware_concurrency();
    uint64_t numThreadsdelete = 8;//thread::hardware_concurrency();
    vector<thread> threadsParityupdate(numThreadsParityupdate);
    vector<thread> threadsdelete(numThreadsdelete);
    int paritynextblocks = 0;
    int boundblocks = 0;
    for (uint64_t threadID = 0 ; threadID < numThreadsParityupdate ; ++threadID){
        // Create thread
        threadsParityupdate[threadID] = std::thread([threadID,numThreadsParityupdate,&paritynextblocks,&boundblocks,&boundDifference,&parityset,&parityupdate,&smallestnexthw,&cipher,Vs,permute,&smallesthw](){
            unordered_set<uint64_t> paritynext;
            for (uint64_t i = threadID ; i < parityset.size() ; i+=numThreadsParityupdate){
                vector<int> nibblevals;
                for (int nibble = 0; nibble<16; nibble++) {
                    nibblevals.push_back((parityset[i] >> 4*nibble) & 0xf);
                }
                
                vector<int> T(16,0);
                
                while (T[15]<Vs[nibblevals[15]].size()) {
                    // Create the parityvalue after the sbox and permutation
                    
                    uint64_t number = 0, number2 = 0, nibbleval, val;
                    for (int j = 0; j<T.size(); j++) {
                        nibbleval = Vs[nibblevals[j]][T[j]];
                        number ^= nibbleval << 4*j;
                    }
                    
                    for (int ii = 0; ii<64; ii++) {
                        number2 ^= ((number>>ii)&1)<<permute[ii];
                    }
                    // Will it look like something we already have after the next sbox application?
                    bool succ = false;
                    nextnumber nn;
                    if(cipher=="Rectangle")nn=numberCalcRectangle(number2);
                    else if (cipher=="Present")nn=numberCalcPresent(number2);
                    else if(cipher=="G0inverse")nn=numberCalcG0inverse(number2);
                    else if (cipher=="G0")nn=numberCalcG0(number2);
                    
                    int temp = bitcount(number2);
                    if (temp<smallesthw) {
                        mutx.lock();
                        if (temp<smallesthw) smallesthw = temp;
                        mutx.unlock();
                    }
                    if (nn.nexthw<smallestnexthw) {
                        mutx.lock();
                        if (nn.nexthw<smallestnexthw) smallestnexthw=nn.nexthw;
                        mutx.unlock();
                    }
                    number = nn.val;
                    
                    int lowerbound = smallestnexthw + boundDifference;
                    if (lowerbound>smallesthw) lowerbound = smallesthw;
                    
                    if ((nn.nexthw!=smallestnexthw)&&(lowerbound<nn.nexthw)){succ=true;boundblocks++;}
                    else if (paritynext.find(number)!=paritynext.end()){succ=true; paritynextblocks++;}
                    if(!succ){
                        paritynext.insert(number);
                        /*
                        for (int nibble = 0; nibble<16; nibble++) {
                            val = (number>>4*nibble) | (number<<(64-4*nibble));
                            paritynext.insert(val);
                        }*/
                        mutx.lock();
                        parityupdate.push_back(number2);
                        mutx.unlock();
                    }// And then we need to check in the end whether the elements succeeds eachother
                    T[0]++;
                    for (int j = 1; j<T.size(); j++) {
                        if (T[j-1] == Vs[nibblevals[j-1]].size()) {
                            T[j-1] = 0;
                            T[j]++;
                        }
                    }
                    if (smallesthw == 1) {
                        break;
                    }
                }//Counter loop end
                if (smallesthw == 1) {
                    break;
                }
            }
        });//parityset loop end
    }
    
    for (uint64_t threadID = 0 ; threadID < numThreadsParityupdate ; ++threadID){
        threadsParityupdate[threadID].join();
    }
    cout << "Bound blocks: " << boundblocks << endl;
    cout << "Paritynext blocks: " << paritynextblocks << endl;
    gettimeofday(&end, NULL);
    cout << "SPLayer done in " << dec << (end.tv_sec  - start.tv_sec) << " seconds" << endl;
    cout << "Size of parityupdate: " << dec << parityupdate.size() << endl;
    cout << "Smallest hw: " << smallesthw << endl;
    gettimeofday(&start, NULL);
    if (smallesthw == 1) {
        parityupdate.clear();
        parityupdate.push_back(0x1);
        return parityupdate;
    }
    sort(parityupdate.begin(),parityupdate.end(),hwcomp);
    //printtofile(parityupdate);
    vector<size_t> del;
    if (parityupdate.size()<10000000) {
    for (uint64_t threadID = 0 ; threadID < numThreadsdelete ; ++threadID){
        // Create thread
        
        threadsdelete[threadID] = thread([threadID,numThreadsdelete, smallesthw,&parityupdate,&del](){
            size_t stop=upper_bound(parityupdate.begin(), parityupdate.end(), parityupdate[0], hwcomp)-parityupdate.begin();
            for (int j = threadID; j<parityupdate.size(); j+=numThreadsdelete) {
                
                if(j>=stop){ break;}
                for (size_t i = stop; i<parityupdate.size(); i++) {
                    uint64_t val = parityupdate[j];
                    for(int jj = 0; jj<15; jj++){
                        val = (val << 4)^(val >> 60);
                        if (((val & parityupdate[i]) == val)) {
                            mutx.lock();
                            del.push_back(i);
                            mutx.unlock();
                            break;
                        }
                    }
                }
                
            }});
    }
    for (uint64_t threadID = 0 ; threadID < numThreadsdelete ; ++threadID){
        threadsdelete[threadID].join();
    }
    sort(del.begin(), del.end());
    del.erase( unique( del.begin(), del.end() ), del.end() );
    for (int j = del.size()-1; j>=0; j--) {
        parityupdate[del[j]] = parityupdate.back();
        parityupdate.pop_back();
    }
    sort(parityupdate.begin(),parityupdate.end(),hwcomp);
    gettimeofday(&end, NULL);
    cout << "Clear up done in " << dec << (end.tv_sec  - start.tv_sec) << " seconds" << endl;
    cout << "Elements deleted: " << dec << del.size() << endl;

    gettimeofday(&start, NULL);
    del.clear();
    for (uint64_t threadID = 0 ; threadID < numThreadsdelete ; ++threadID){
        // Create thread
        
        threadsdelete[threadID] = thread([threadID,numThreadsdelete, smallesthw,&parityupdate,&del](){
            size_t bit=0,start=0, stop =upper_bound(parityupdate.begin(), parityupdate.end(), parityupdate[0], hwcomp)-parityupdate.begin();
            for (int j = parityupdate.size()-1-threadID; j>0; j-=numThreadsdelete) {
                if(bit!=bitcount(parityupdate[j])){
                    bit=bitcount(parityupdate[j]);
                    if(bit==smallesthw){ break;}
                    start = lower_bound(parityupdate.begin(), parityupdate.end(), parityupdate[j], hwcomp)-parityupdate.begin();
                }
                for (size_t i = stop; start+1>i; i++) {
                    if (((parityupdate[j] & parityupdate[i-1]) == parityupdate[i-1])&&(i-1!=j)) {
                        mutx.lock();
                        del.push_back(j);
                        mutx.unlock();
                        break;
                    }
                    
                }
                
            }});
    }
    for (uint64_t threadID = 0 ; threadID < numThreadsdelete ; ++threadID){
        threadsdelete[threadID].join();
    }
    sort(del.begin(), del.end());
    for (int j = del.size()-1; j>=0; j--) {
        parityupdate[del[j]] = parityupdate.back();
        parityupdate.pop_back();
    }
    sort(parityupdate.begin(),parityupdate.end(),hwcomp);
    gettimeofday(&end, NULL);
    cout << "Clear up done in " << dec << (end.tv_sec  - start.tv_sec) << " seconds" << endl;
     cout << "Elements deleted: " << dec << del.size() << endl;
    }
    return parityupdate;
}

vector<vector<uint64_t>> initCompactVsRectangle(){
    vector<vector<uint64_t>> Vs(16);
    vector<uint64_t> container;
    container.push_back(0);
    Vs[0] = container;
    container.clear();
    container.push_back(1);
    container.push_back(2);
    container.push_back(4);
    container.push_back(8);
    Vs[1] = container;
    Vs[2] = container;
    Vs[4] = container;
    Vs[8] = container;
    container.clear();
    container.push_back(1);
    container.push_back(4);
    container.push_back(10);
    Vs[3] = container;
    container.clear();
    container.push_back(4);
    container.push_back(8);
    container.push_back(3);
    Vs[5] = container;
    container.clear();
    container.push_back(4);
    container.push_back(8);
    container.push_back(3);
    Vs[6] = container;
    container.clear();
    container.push_back(8);
    container.push_back(3);
    container.push_back(5);
    container.push_back(6);
    Vs[9] = container;
    container.clear();
    container.push_back(8);
    container.push_back(2);
    container.push_back(5);
    Vs[10] = container;
    container.clear();
    container.push_back(8);
    container.push_back(4);
    container.push_back(3);
    Vs[12] = container;
    container.clear();
    container.push_back(4);
    container.push_back(3);
    container.push_back(9);
    Vs[7] = container;
    container.clear();
    container.push_back(6);
    container.push_back(11);
    container.push_back(13);
    Vs[11] = container;
    container.clear();
    container.push_back(6);
    container.push_back(10);
    container.push_back(13);
    Vs[13] = container;
    container.clear();
    container.push_back(8);
    container.push_back(3);
    container.push_back(5);
    Vs[14] = container;
    Vs[15].push_back(15);
    return Vs;
}
vector<vector<uint64_t>> initCompactVsPresent(){
    vector<vector<uint64_t>> Vs(16);
    vector<uint64_t> container;
    
    container.push_back(0);
    Vs[0] = container;
    container.clear();
    container.push_back(1);
    container.push_back(2);
    container.push_back(4);
    container.push_back(8);
    Vs[1] = container;
    Vs[2] = container;
    Vs[4] = container;
    Vs[8] = container;
    container.clear();
    container.push_back(2);
    container.push_back(4);
    container.push_back(8);
    Vs[3] = container;
    container.push_back(3);
    Vs[5] = container;
    Vs[9] = container;
    Vs[10] = container;
    Vs[12] = container;
    container.clear();
    container.push_back(2);
    container.push_back(1);
    container.push_back(8);
    container.push_back(5);
    Vs[6] = container;
    container.clear();
    container.push_back(8);
    container.push_back(2);
    container.push_back(0xd);
    Vs[7] = container;
    container.clear();
    container.push_back(4);
    container.push_back(2);
    container.push_back(8);
    container.push_back(3);
    Vs[11] = container;
    container.clear();
    container.push_back(4);
    container.push_back(2);
    container.push_back(8);
    container.push_back(9);
    Vs[13] = container;
    container.clear();
    container.push_back(11);
    container.push_back(5);
    Vs[14] = container;
    Vs[15].push_back(15);
    return Vs;
}
vector<vector<uint64_t>> initCompactVsG0(){
    vector<vector<uint64_t>> Vs(16);
    vector<uint64_t> container;
    
    container.push_back(0);
    Vs[0] = container;
    container.clear();
    container.push_back(1);
    container.push_back(2);
    container.push_back(4);
    container.push_back(8);
    Vs[1] = container;
    Vs[2] = container;
    Vs[4] = container;
    Vs[8] = container;
    container.clear();
    container.push_back(2);
    container.push_back(4);
    container.push_back(8);
    Vs[3] = container;
    Vs[5] = container;
    Vs[9] = container;
    Vs[10] = container;
    container.clear();
    container.push_back(4);
    container.push_back(1);
    container.push_back(8);
    Vs[6] = container;
    container.clear();
    container.push_back(1);
    container.push_back(2);
    container.push_back(4);
    container.push_back(8);
    Vs[0xc] = container;
    container.clear();
    container.push_back(4);
    container.push_back(10);
    container.push_back(0xe);
    Vs[7] = container;
    container.clear();
    container.push_back(8);
    container.push_back(3);
    container.push_back(5);
    Vs[11] = container;
    container.clear();
    container.push_back(4);
    container.push_back(8);
    container.push_back(3);
    Vs[13] = container;
    container.clear();
    container.push_back(4);
    container.push_back(9);
    Vs[14] = container;
    Vs[15].push_back(15);
    return Vs;
}
vector<vector<uint64_t>> initCompactVsG0inverse(){
    vector<vector<uint64_t>> Vs(16);
    vector<uint64_t> container;
    
    container.push_back(0);
    Vs[0] = container;
    container.clear();
    container.push_back(1);
    container.push_back(2);
    container.push_back(4);
    container.push_back(8);
    Vs[1] = container;
    Vs[2] = container;
    Vs[4] = container;
    Vs[8] = container;
    Vs[10] = container;
    Vs[3] = container;
    container.clear();
    container.push_back(2);
    container.push_back(4);
    container.push_back(8);
    Vs[5] = container;
    container.clear();
    container.push_back(2);
    container.push_back(4);
    container.push_back(1);
    Vs[6] = container;
    container.clear();
    container.push_back(2);
    container.push_back(1);
    container.push_back(8);
    Vs[9] = container;
    container.clear();
    container.push_back(2);
    container.push_back(4);
    container.push_back(9);
    Vs[12] = container;
    container.clear();
    container.push_back(4);
    container.push_back(2);
    container.push_back(9);
    Vs[7] = container;
    container.clear();
    container.push_back(1);
    container.push_back(2);
    container.push_back(8);
    Vs[11] = container;
    container.clear();
    container.push_back(3);
    container.push_back(5);
    container.push_back(6);
    container.push_back(10);
    container.push_back(12);
    Vs[13] = container;
    container.clear();
    container.push_back(3);
    container.push_back(9);
    container.push_back(15);
    Vs[14] = container;
    Vs[15].push_back(15);
    return Vs;
}
void parityFULL(vector<uint64_t> parityset, string cipher, int numberOfRounds){
    vector<vector<uint64_t>> Vs;
    int permute[64];
    // Initializing Vs and permutation:
    if (cipher=="Rectangle") {
        Vs = initCompactVsRectangle();
        for ( int i = 0; i<64;i++){
            if ((i%4)==3) permute[i] = (i+13*4)%64;
            if ((i%4)==2) permute[i] = (i+12*4)%64;
            if ((i%4)==1) permute[i] = (i+1*4)%64;
            if ((i%4)==0) permute[i] = i;
        }
    }else if (cipher=="Present"){
        Vs = initCompactVsPresent();
        int iter=0;
        for ( int i = 0; i<64;i++){
            if ((i%4)==0) iter = i/4;
            permute[i] = iter + (i%4)*16;
        }
    }else if (cipher=="G0"){
        Vs = initCompactVsG0();
        int iter=0;
        for ( int i = 0; i<64;i++){
            if ((i%4)==0) iter = i/4;
            permute[i] = iter + (i%4)*16;
        }
    }else if (cipher=="G0inverse"){
        Vs = initCompactVsG0inverse();
        int iter=0;
        for ( int i = 0; i<16;i++){
            permute[i] = iter;
            iter += 4;
        }
        iter=1;
        for ( int i = 0; i<16;i++){
            permute[i+16] = iter;
            iter += 4;
        }
        iter=2;
        for ( int i = 0; i<16;i++){
            permute[i+32] = iter;
            iter += 4;
        }
        iter=3;
        for ( int i = 0; i<16;i++){
            permute[i+48] = iter;
            iter += 4;
        }
    }
    
    // Initializing parityset and running through one round
    
    int round = 0, hamming=2;
    while (round<numberOfRounds) {
        round++;
        cout << "For round " << dec <<round << endl;
        parityset = SPlayerParFULL(parityset, Vs, permute, cipher);
        cout << "Size after S-layer: " << dec << parityset.size() << endl;
        sort(parityset.begin(), parityset.end(), hwcomp);
        hamming = bitcount(parityset[0]);
        int i = 0;
        while ((bitcount(parityset[i])<2)&&(i<parityset.size())) {
            i++;
        }cout << "There are a total of " << dec << i << " weight 1 u's in parityset" << endl;
    }
    cout << "Weight 1 u's in parityset: "<< endl;
    int i = 0;
    while ((bitcount(parityset[i])<2)&&(i<parityset.size())) {
        cout << hex << parityset[i] << endl;
        i++;
    }
    cout << "There are a total of " << dec << i << " weight 1 u's in parityset" << endl;
    
}

void parity(vector<uint64_t> parityset, string cipher, int boundDifference){
    vector<vector<uint64_t>> Vs;
    int permute[64];
    // Initializing Vs and permutation:
    if (cipher=="Rectangle") {
        Vs = initCompactVsRectangle();
        for ( int i = 0; i<64;i++){
            if ((i%4)==3) permute[i] = (i+13*4)%64;
            if ((i%4)==2) permute[i] = (i+12*4)%64;
            if ((i%4)==1) permute[i] = (i+1*4)%64;
            if ((i%4)==0) permute[i] = i;
        }
    }else if (cipher=="Present"){
        Vs = initCompactVsPresent();
        int iter=0;
        for ( int i = 0; i<64;i++){
            if ((i%4)==0) iter = i/4;
            permute[i] = iter + (i%4)*16;
        }
    }else if (cipher=="G0"){
        Vs = initCompactVsG0();
        int iter=0;
        for ( int i = 0; i<64;i++){
            if ((i%4)==0) iter = i/4;
                permute[i] = iter + (i%4)*16;
        }
    }else if (cipher=="G0inverse"){
        Vs = initCompactVsG0inverse();
        int iter=0;
        for ( int i = 0; i<16;i++){
            permute[i] = iter;
            iter += 4;
        }
        iter=1;
        for ( int i = 0; i<16;i++){
            permute[i+16] = iter;
            iter += 4;
        }
        iter=2;
        for ( int i = 0; i<16;i++){
            permute[i+32] = iter;
            iter += 4;
        }
        iter=3;
        for ( int i = 0; i<16;i++){
            permute[i+48] = iter;
            iter += 4;
        }
    }
    
    // Initializing parityset and running through one round
    
    int round = 0, hamming=2;
    while (hamming>1) {
        round++;
        cout << "For round " << dec <<round << endl;
        parityset = SPlayerPar(parityset, Vs, permute, cipher, boundDifference);
        cout << "Size after S-layer: " << dec << parityset.size() << endl;
        hamming = bitcount(parityset[0]);
    }
    cout << "Distinguisher on " << dec << (round - 1) << " rounds" << endl;
}
