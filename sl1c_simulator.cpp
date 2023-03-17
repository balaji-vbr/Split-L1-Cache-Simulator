#include <bits/stdc++.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <tuple>
#include <vector>
#include<string.h>
#include <cctype>
#include<cstring>
#define NOHIT 0
#define HIT 1
#define HITM 2


#define READ 1 /* Bus Read */
#define WRITE 2 /* Bus Write */
#define INVALIDATE 3 /* Bus Invalidate */
#define RWIM 4 /* Bus Read With Intent to Modify */
using namespace std;

unsigned int address;
int instr;
bool NormalMode;
bool SilentMode;
bool File;

unsigned int data_cache_hit 	= 0; 				// Data Cache Hits count
unsigned int data_cache_miss = 0; 				// Data Cache Misses count
unsigned int data_cache_read = 0; 				// Data Cache read count
unsigned int data_cache_write = 0; 				// Data Cache write count

unsigned int instruction_cache_hit	= 0; 		// Instruction Cache Hits count
unsigned int instruction_cache_miss 	= 0; 		// Instruction Cache Misses count
unsigned int instruction_cache_read 	= 0; 		// Instruction Cache read count
char MESI_name;
float hitratio;
float DATAhitratio;
float INSTRhitratio;
	
                                                                                                                              

//---------PARSE TRACE LINE FUNCTION----------------
void parse_line(std::string access,unsigned int arr[]) {
	// What we want to parse
	int instr;
	unsigned int address;
	// Parse from the string we read from the file
	sscanf(access.c_str(), "%d %lx", &instr, &address);
	arr[0]=instr;
	arr[1]=address;
}

//---------CALCULATE SET------------------
int get_set(unsigned int address) {
	auto shifted_address = address >> 6;
	shifted_address = shifted_address & 0x3FFF;
	if(NormalMode) {
		cout<<"Set:"<<shifted_address<<"\t";
	}
	return shifted_address;
}

//--------CALCULATE TAG------------------
int get_tag(unsigned int address) {
	auto shifted_address = address >> 20;
	if(NormalMode) {
		cout<<"Tag:"<<std::hex<<shifted_address<<endl;
	}
	return shifted_address;
}

enum mesi_st {I,E,S,M}; // I-0; E-1; S-2; M-3;

//-------------SINGLE WAY--------------------
struct lines{							
			int MESI;
			int	LRU;
			int	TAG;			 
			};

//---------DATA CACHE--------------------				
struct datac {																	
	struct lines *LINE = (lines*)malloc(sizeof(struct lines) * 8);			// Lines for 8-way
} ;	

//--------INSTRUCTION CACHE--------------------
struct instruction {																	
	struct lines *LINE = (lines*)malloc(sizeof(struct lines) * 4);			// Lines for n-way
} ;	

//-------------CHECK WAYS FILLED FOR DATACACHE------------------
int waysFilledD(datac cache) {
	int ways_filled = 0;
	for (int i=0;i<8;i++) {
		if (cache.LINE[i].MESI != I) {
			ways_filled++;
		}
	}
	if(NormalMode) {
		cout<< "Ways filled - " <<ways_filled<<endl;	
	}
	return ways_filled;
}

//-------------CHECK EMPTY WAY FOR DATACACHE------------------
int checkWayD(datac cache) {
	int empty_way;
	for(int i=0;i<8;i++) {
		if (cache.LINE[i].MESI == I) {
			empty_way = i;
			break;
		}
	}
	if (empty_way>7) {
		empty_way = 8;
		cout<<"All ways are full!"<<endl;
	}
	if(NormalMode) {
		cout<<"Empty way - "<<empty_way<<endl;
	}
	return empty_way;
}

//-------------CHECK WAYS FILLED FOR INSTRUCTION CACHE------------------
int waysFilledI(instruction cache) {
	int ways_filled = 0;
	for (int i=0;i<4;i++) {
		if (cache.LINE[i].MESI != I) {
			ways_filled++;
		}
	}
	if(NormalMode) {
		cout<< "Ways filled - " <<ways_filled<<endl;	
	}
	return ways_filled;
}

//-------------CHECK EMPTY WAY FOR INSTRUCTION CACHE------------------
int checkWayI(instruction cache) {
	int empty_way;
	for(int i=0;i<4;i++) {
		if (cache.LINE[i].MESI == I) {
			empty_way = i;
			break;
		}
	}
	if (empty_way>3) {
		empty_way = 4;
		cout<<"All ways are full!"<<endl;
	}
	if(NormalMode) {
		cout<<"Empty way - "<<empty_way<<endl;
	}
	return empty_way;
}

//----------------------------------Snoop Result--------------------------------
int GetSnoopResult(unsigned int address){
    if((address & 3)==0){              //address with LSB 00
        return HIT;
    }

    else if((address & 3)==1){         //address with LSB 01
        return HIT;
    }

    else{                              //address ending with 10 or 11
        return NOHIT;
    }
}
//-----------------------Bus Operation---------------------------------
void BusOperation(int BusOp, unsigned int address, int *SnoopResult){
    *SnoopResult = GetSnoopResult(address);
	string temp_snoop;
	if (*SnoopResult == 0){
	temp_snoop = "NOHIT";
	}
	else if (*SnoopResult == 1){
	temp_snoop = "HIT";
	}
	else if (*SnoopResult == 2){
	temp_snoop = "HITM";
	}
	
	if(NormalMode){
		if (BusOp == 1) {
			cout<<"Bus Operation: READ"<<"\tAddress: "<<std::hex <<address<<"\tSnoopResult: "<<temp_snoop<<endl;
		}
		else if (BusOp == 2) {
			cout<<"Bus Operation: WRITE"<<"\tAddress: "<<std::hex<<address<<"\tSnoopResult: "<<temp_snoop<<endl;
		}
		else if (BusOp == 3) {
			cout<<"Bus Operation: INVALIDATE"<<"\tAddress: "<<std::hex<<address<<"\tSnoopResult: "<<temp_snoop<<endl;
		}
		else if (BusOp == 4) {
			cout<<"Bus Operation: RWIM\t\t"<<"\tAddress: "<<std::hex<<address<<"\tSnoopResult: "<<temp_snoop<<endl;
		}
	}
	
}
char getMesiName(int mesi) 
{
   switch (mesi) 
   {
      case 0: MESI_name= 'I'; break;
      case 1: MESI_name= 'E'; break;
      case 2: MESI_name= 'S'; break;
      case 3: MESI_name= 'M'; break;	  
   }
   return MESI_name;
}

//---------------------------Main Program-----------------------------------

int main(int argc, char* argv[]){
	std::string line;
	std::ifstream infile;
	unsigned int trace[2];
	int set, tag;
	int ways_filled;
	int cache_read=0;
	int cache_hit=0;
	int cache_miss=0;
	int cache_write=0;
	int hit;
	int curr_way;
	int ps;
	int SnoopResult;
	
	switch(argc){
		case 1: infile.open("trace.log");
				//check if file exist or not
				if(infile){
					cout<<"File exists"<<endl;
					File = 1;
				}
				else{
					cout<<"File - 'trace.log' does not exists"<<endl;
					File = 0;
				}
				//bydefault Normal Mode
				NormalMode=1;
				SilentMode=0;
				break;
				
		case 2: infile.open(argv[1]);
				//check if file exist or not
				if(infile){
					cout<<"File exists"<<endl;
					File = 1;
				}
				else{
					cout<<"File "<<argv[1]<<" does not exists"<<endl;
					File = 0;
				}
				//bydefault Normal Mode
				NormalMode=1;
				SilentMode=0;
				break;
				
		case 3: infile.open(argv[1]);
				//check if file exist or not
				if(infile){
					cout<<"File exists"<<endl;
					File = 1;
				}
				else{
					cout<<"File "<<argv[1]<<" does not exists"<<endl;
					File = 0;
				}
				//check if the mode is correct or not
				if(strcmp(argv[2],"SilentMode")==0){
					SilentMode=1;
					NormalMode=0;
				}
				else if(strcmp(argv[2],"SilentMode")!=0){
					SilentMode=0;
					NormalMode=0;
					cout<<"This mode does not exist"<<endl;
				}
				else{
					SilentMode=0;
					NormalMode=1;
				}
				break;
	}
	
	if(File & (NormalMode | SilentMode)){
		datac DATACACHE[16384];
		instruction INSTRCACHE[16384];
		//Setting everything to I state 
		for(int i=0;i<16384;i=i+1){
			for(int j=0;j<8;j=j+1){
				DATACACHE[i].LINE[j].MESI=I;
			}
		}
		//Setting everything to I state 
		for(int i=0;i<16384;i=i+1){
			for(int j=0;j<4;j=j+1){
				INSTRCACHE[i].LINE[j].MESI=I;
			}
		}
		while (std::getline(infile, line)){                  //read trace file line by line
			parse_line(line,trace);
			instr = trace[0];
			address = trace[1];
			if(NormalMode)
				cout<<"n: "<<instr<<" address: "<<std::hex<<address<<endl;
//----------------------------------Read Data---------------------------------
		int curr_way;
		int ways_filled;
			if(instr==0){
				if(NormalMode){
					cout<<"read data request to L1 data cache for address "<<std::hex<<address<<endl;
				}
				data_cache_read++;
				set=get_set(address);
				tag=get_tag(address);
				ways_filled= waysFilledD(DATACACHE[set]);
				if(ways_filled<8){
					hit=0;
					for(int i=0; i<8; i++){
						if(DATACACHE[set].LINE[i].MESI!=I && DATACACHE[set].LINE[i].TAG==tag){
							data_cache_hit++;
							if(NormalMode){
								cout<<"Read Hit"<<endl;
							}
							hit=1;
							//UpdateLRU
							int temp= DATACACHE[set].LINE[i].LRU;
							for (int j = 0; j < 8; j++) {
								DATACACHE[set].LINE[j].LRU = (DATACACHE[set].LINE[j].LRU < temp) ? DATACACHE[set].LINE[j].LRU + 1 : DATACACHE[set].LINE[j].LRU;
							}
							DATACACHE[set].LINE[i].LRU=0;
							break;
							cout<< "Read from L2 address       "<<std::hex<<address<<endl;			
								
						}
					}
					
					if(hit==0 | ways_filled==0){
						curr_way= checkWayD(DATACACHE[set]);
						BusOperation(READ, address, &(SnoopResult));
						ps = DATACACHE[set].LINE[curr_way].MESI;
						if (DATACACHE[set].LINE[curr_way].MESI == I) {	
							if(SnoopResult==0){
								DATACACHE[set].LINE[curr_way].MESI = E;
								if(NormalMode){
									cout<< "Read Miss. Setting MESI bit from "<<getMesiName(ps)<<" to "<<getMesiName(DATACACHE[set].LINE[curr_way].MESI)<<endl;
								}
							}

							else if(SnoopResult==1){
								DATACACHE[set].LINE[curr_way].MESI = S;
								if(NormalMode){
									cout<< "Read Miss. Setting MESI bit from "<<getMesiName(ps)<<" to "<<getMesiName(DATACACHE[set].LINE[curr_way].MESI)<<endl;
								}
							}

							else if(SnoopResult==2){
								DATACACHE[set].LINE[curr_way].MESI = S;
								if(NormalMode){
								cout<< "Read Miss. Setting MESI bit from "<<getMesiName(ps)<<" to "<<getMesiName(DATACACHE[set].LINE[curr_way].MESI)<<endl;
								}
							}
							
							DATACACHE[set].LINE[curr_way].TAG = tag;
							//UpdateLRU
							int temp= DATACACHE[set].LINE[curr_way].LRU;
							for (int j = 0; j < curr_way; j++) {
								DATACACHE[set].LINE[j].LRU = DATACACHE[set].LINE[j].LRU + 1;
							}
							DATACACHE[set].LINE[curr_way].LRU=0;
							data_cache_miss++;
							cout<< "Read from L2 address       "<<std::hex<<address<<endl;	
						}
						
					}
				}
				else if(ways_filled==8){
					int hit = 0;
					for(int i=0;i<8;i++) {
						if (DATACACHE[set].LINE[i].MESI != I && DATACACHE[set].LINE[i].TAG == tag) {	
							data_cache_hit++;		
							if(NormalMode){
								cout<< "Read Hit."<<endl;
							}
							hit = 1;
							//UpdateLRU
							int temp= DATACACHE[set].LINE[i].LRU;
							for (int j = 0; j < 8; j++) {
								DATACACHE[set].LINE[j].LRU = (DATACACHE[set].LINE[j].LRU < temp) ? DATACACHE[set].LINE[j].LRU + 1 : DATACACHE[set].LINE[j].LRU;
							}
							DATACACHE[set].LINE[i].LRU=0;
							
							break;
						}
					}
					if(hit == 0){	
						cout<<"Evict Line"<<endl;
						for(int i=0; i<8; i++){
							if(DATACACHE[set].LINE[i].LRU==7)
								curr_way=i;
						}
						ps = DATACACHE[set].LINE[curr_way].MESI;
						if (DATACACHE[set].LINE[curr_way].MESI == M) {
							BusOperation(WRITE, address, &SnoopResult);
						}
						BusOperation(READ, address, &SnoopResult);
						if(SnoopResult==0){
							DATACACHE[set].LINE[curr_way].MESI = E;
							if(NormalMode){
								cout<< "Read Miss. Setting MESI bit from "<<getMesiName(ps)<<" to "<<getMesiName(DATACACHE[set].LINE[curr_way].MESI)<<endl;
							}
						}
						else if(SnoopResult==1){
							DATACACHE[set].LINE[curr_way].MESI = S;
							if(NormalMode){
								cout<< "Read Miss. Setting MESI bit from "<<getMesiName(ps)<<" to "<<getMesiName(DATACACHE[set].LINE[curr_way].MESI)<<endl;
							}
						}
						else if(SnoopResult==2){
							DATACACHE[set].LINE[curr_way].MESI = S;
							if(NormalMode){
								cout<< "Read Miss. Setting MESI bit from "<<getMesiName(ps)<<" to "<<getMesiName(DATACACHE[set].LINE[curr_way].MESI)<<endl;
							}
						}
						
						DATACACHE[set].LINE[curr_way].TAG = tag;
						//UpdateLRU
						int temp= DATACACHE[set].LINE[curr_way].LRU;
						for (int j = 0; j < 8; j++) {
							DATACACHE[set].LINE[j].LRU = (DATACACHE[set].LINE[j].LRU < temp) ? DATACACHE[set].LINE[j].LRU + 1 : DATACACHE[set].LINE[j].LRU;
						}
						DATACACHE[set].LINE[curr_way].LRU=0;
						data_cache_miss++;
						cout<< "Read from L2 address       "<<std::hex<<address<<endl;
					}
					
				}
				//for(int i=0; i<8; i++)
					//cout<<DATACACHE[set].LINE[i].LRU<<"\t";
				if(NormalMode){
					cout<<endl;
					cout<<endl;
				}
				
			}
//----------------------Write Data--------------------------------------------------
			else if(instr==1){
				if(NormalMode){
					cout<<"write data request to L1 data cache for address "<<std::hex<<address<<endl;
				}
				data_cache_write++;
				set=get_set(address);
				tag=get_tag(address);
				ways_filled= waysFilledD(DATACACHE[set]);
				if (ways_filled<8) {														
					int hit = 0;
					for(int i=0;i<8;i++) {
						if (DATACACHE[set].LINE[i].MESI != I && DATACACHE[set].LINE[i].TAG == tag) {	
							data_cache_hit++;
							hit = 1;
							ps = DATACACHE[set].LINE[i].MESI;
							//UpdateLRU
							int temp= DATACACHE[set].LINE[curr_way].LRU;
							for (int j = 0; j < 8; j++) {
								DATACACHE[set].LINE[j].LRU = (DATACACHE[set].LINE[j].LRU < temp) ? DATACACHE[set].LINE[j].LRU + 1 : DATACACHE[set].LINE[j].LRU;
							}
							DATACACHE[set].LINE[curr_way].LRU=0;
							if (DATACACHE[set].LINE[i].MESI == S){
								DATACACHE[set].LINE[i].MESI = M;
								if(NormalMode){
									cout<< "Write Hit. Setting MESI bit from "<<getMesiName(ps)<<" to "<<getMesiName(DATACACHE[set].LINE[i].MESI)<<endl;
								}
								BusOperation(INVALIDATE, address, &SnoopResult);		
							
							}
							else if (DATACACHE[set].LINE[i].MESI == E){
								DATACACHE[set].LINE[i].MESI = M;
								if(NormalMode){
									cout<< "Write Hit. Setting MESI bit from "<<getMesiName(ps)<<" to "<<getMesiName(DATACACHE[set].LINE[i].MESI)<<endl;
								}
									
							}
							else {
								if(NormalMode){
									cout<< "Write Hit. Setting MESI bit from "<<getMesiName(ps)<<" to "<<getMesiName(DATACACHE[set].LINE[i].MESI)<<endl;
								}
							}
							cout<< "Write to L2      "<<std::hex<<address<<endl;
							break;
						}
					}
					if(hit == 0 || ways_filled == 0){													
						curr_way = checkWayD(DATACACHE[set]);
						BusOperation(RWIM, address, &SnoopResult);
						if (DATACACHE[set].LINE[curr_way].MESI == I) {	
							ps = DATACACHE[set].LINE[curr_way].MESI;
							DATACACHE[set].LINE[curr_way].MESI = M;
							DATACACHE[set].LINE[curr_way].TAG = tag;
							//UpdateLRU
							int temp= DATACACHE[set].LINE[curr_way].LRU;
							for (int j = 0; j < curr_way; j++) {
								DATACACHE[set].LINE[j].LRU = DATACACHE[set].LINE[j].LRU + 1;
							}
							DATACACHE[set].LINE[curr_way].LRU=0;
							if(NormalMode){
								//cout<< "Write Miss. Setting MESI bit from "<<getMesiName(ps)<<" to "<<getMesiName(CACHE[temp_SET].LINE[curr_way].MESI)<<endl;
							cout<< "Read for Ownership from L2       "<<std::hex<<address<<endl;
							}
							
						}
						
						cout<< "Read for Ownership from L2       "<<std::hex<<address<<endl;
						cout<< "Write to L2      "<<std::hex<<address<<endl;
						data_cache_miss++;
					}				
				}
				else if (ways_filled == 8) {													
					int hit = 0;
					for(int i=0;i<8;i++) {
						if (DATACACHE[set].LINE[i].MESI != I && DATACACHE[set].LINE[i].TAG == tag) {	
							data_cache_hit++;		
							hit = 1;
							ps = DATACACHE[set].LINE[i].MESI;
							//UpdateLRU
							int temp= DATACACHE[set].LINE[i].LRU;
							for (int j = 0; j < 8; j++) {
								DATACACHE[set].LINE[j].LRU = (DATACACHE[set].LINE[j].LRU < temp) ? DATACACHE[set].LINE[j].LRU + 1 : DATACACHE[set].LINE[j].LRU;
							}
							DATACACHE[set].LINE[i].LRU=0;
							if (DATACACHE[set].LINE[i].MESI == S){
								DATACACHE[set].LINE[i].MESI = M;
								if(NormalMode){
									cout<< "Write Hit. Setting MESI bit from "<<getMesiName(ps)<<" to "<<getMesiName(DATACACHE[set].LINE[i].MESI)<<endl;
								}
								BusOperation(INVALIDATE, address, &SnoopResult);		
								
							}
							else if (DATACACHE[set].LINE[i].MESI == E){
								DATACACHE[set].LINE[i].MESI = M;
								if(NormalMode){
									cout<< "Write Hit. Setting MESI bit from "<<getMesiName(ps)<<" to "<<getMesiName(DATACACHE[set].LINE[i].MESI)<<endl;
								}
									
							}
							else {
								if(NormalMode){
									cout<< "Write Hit. Setting MESI bit from "<<getMesiName(ps)<<" to "<<getMesiName(DATACACHE[set].LINE[i].MESI)<<endl;
								}
							}
							cout<< "Write to L2      "<<std::hex<<address<<endl;
							break;
						}
					}
					if(hit == 0){																	 
						cout<<"Evict Line"<<endl;
						for(int i=0; i<8; i++){
							if(DATACACHE[set].LINE[i].LRU==7)
								curr_way=i;
						}
						ps = DATACACHE[set].LINE[curr_way].MESI;
						if (DATACACHE[set].LINE[curr_way].MESI == M) {
							BusOperation(WRITE, address, &SnoopResult);
						}
						BusOperation(RWIM, address, &SnoopResult);
						DATACACHE[set].LINE[curr_way].MESI = M;
						DATACACHE[set].LINE[curr_way].TAG = tag;
						//UpdateLRU
						int temp= DATACACHE[set].LINE[curr_way].LRU;
						for (int j = 0; j < 8; j++) {
							DATACACHE[set].LINE[j].LRU = (DATACACHE[set].LINE[j].LRU < temp) ? DATACACHE[set].LINE[j].LRU + 1 : DATACACHE[set].LINE[j].LRU;
						}
						DATACACHE[set].LINE[curr_way].LRU=0;				
						if(NormalMode){
							cout<< "Write Miss. Setting MESI bit from "<<getMesiName(ps)<<" to "<<getMesiName(DATACACHE[set].LINE[curr_way].MESI)<<endl;
						}
						cout<< "Write to L2      "<<std::hex<<address<<endl;
						data_cache_miss++;		
					}
				}
				//for(int i=0; i<8; i++)
					//cout<<DATACACHE[set].LINE[i].LRU<<"\t";
				if(NormalMode){
					cout<<endl;
					cout<<endl;
				}
			}
//----------------------------------Instruction Fetch---------------------------------
			else if(instr==2){
				if(NormalMode){
					cout<<"instruction fetch for address "<<std::hex<<address<<endl;
				}
				instruction_cache_read++;
				set=get_set(address);
				tag=get_tag(address);
				ways_filled= waysFilledI(INSTRCACHE[set]);
				if(ways_filled<4){
					hit=0;
					for(int i=0; i<4; i++){
						if(INSTRCACHE[set].LINE[i].MESI!=I && INSTRCACHE[set].LINE[i].TAG==tag){
							instruction_cache_hit++;
							if(NormalMode){
								cout<<"Read Hit"<<endl;
							}
							hit=1;
							//UpdateLRU
							int temp= INSTRCACHE[set].LINE[i].LRU;
							for (int j = 0; j < 4; j++) {
								INSTRCACHE[set].LINE[j].LRU = (INSTRCACHE[set].LINE[j].LRU < temp) ? INSTRCACHE[set].LINE[j].LRU + 1 : INSTRCACHE[set].LINE[j].LRU;
							}
							INSTRCACHE[set].LINE[i].LRU=0;
							break;
							cout<< "Read from L2 address       "<<std::hex<<address<<endl;			
								
						}
					}
					
					if(hit==0 | ways_filled==0){
						curr_way= checkWayI(INSTRCACHE[set]);
						BusOperation(READ, address, &(SnoopResult));
						ps = INSTRCACHE[set].LINE[curr_way].MESI;
						if (INSTRCACHE[set].LINE[curr_way].MESI == I) {	
							if(SnoopResult==0){
								INSTRCACHE[set].LINE[curr_way].MESI = E;
								if(NormalMode){
									cout<< "Read Miss. Setting MESI bit from "<<getMesiName(ps)<<" to "<<getMesiName(INSTRCACHE[set].LINE[curr_way].MESI)<<endl;
								}
							}

							else if(SnoopResult==1){
								INSTRCACHE[set].LINE[curr_way].MESI = S;
								if(NormalMode){
									cout<< "Read Miss. Setting MESI bit from "<<getMesiName(ps)<<" to "<<getMesiName(INSTRCACHE[set].LINE[curr_way].MESI)<<endl;
								}
							}

							else if(SnoopResult==2){
								INSTRCACHE[set].LINE[curr_way].MESI = S;
								if(NormalMode){
								cout<< "Read Miss. Setting MESI bit from "<<getMesiName(ps)<<" to "<<getMesiName(INSTRCACHE[set].LINE[curr_way].MESI)<<endl;
								}
							}
							
							INSTRCACHE[set].LINE[curr_way].TAG = tag;
							//UpdateLRU
							int temp= INSTRCACHE[set].LINE[curr_way].LRU;
							for (int j = 0; j < curr_way; j++) {
								INSTRCACHE[set].LINE[j].LRU = INSTRCACHE[set].LINE[j].LRU + 1;
							}
							INSTRCACHE[set].LINE[curr_way].LRU=0;
							instruction_cache_miss++;
							cout<< "Read from L2 address       "<<std::hex<<address<<endl;	
						}
						
					}
				}
				else if(ways_filled==4){
					int hit = 0;
					for(int i=0;i<4;i++) {
						if (INSTRCACHE[set].LINE[i].MESI != I && INSTRCACHE[set].LINE[i].TAG == tag) {	
							instruction_cache_hit++;		
							if(NormalMode){
								cout<< "Read Hit."<<endl;
							}
							hit = 1;
							//UpdateLRU
							int temp= INSTRCACHE[set].LINE[i].LRU;
							for (int j = 0; j < 4; j++) {
								INSTRCACHE[set].LINE[j].LRU = (INSTRCACHE[set].LINE[j].LRU < temp) ? INSTRCACHE[set].LINE[j].LRU + 1 : INSTRCACHE[set].LINE[j].LRU;
							}
							INSTRCACHE[set].LINE[i].LRU=0;
							
							break;
						}
					}
					if(hit == 0){	
						cout<<"Evict Line"<<endl;
						for(int i=0; i<4; i++){
							if(INSTRCACHE[set].LINE[i].LRU==3)
								curr_way=i;
						}
						ps = INSTRCACHE[set].LINE[curr_way].MESI;
						if (INSTRCACHE[set].LINE[curr_way].MESI == M) {
							BusOperation(WRITE, address, &SnoopResult);
						}
						BusOperation(READ, address, &SnoopResult);
						if(SnoopResult==0){
							INSTRCACHE[set].LINE[curr_way].MESI = E;
							if(NormalMode){
								cout<< "Read Miss. Setting MESI bit from "<<getMesiName(ps)<<" to "<<getMesiName(INSTRCACHE[set].LINE[curr_way].MESI)<<endl;
							}
						}
						else if(SnoopResult==1){
							INSTRCACHE[set].LINE[curr_way].MESI = S;
							if(NormalMode){
								cout<< "Read Miss. Setting MESI bit from "<<getMesiName(ps)<<" to "<<getMesiName(INSTRCACHE[set].LINE[curr_way].MESI)<<endl;
							}
						}
						else if(SnoopResult==2){
							INSTRCACHE[set].LINE[curr_way].MESI = S;
							if(NormalMode){
								cout<< "Read Miss. Setting MESI bit from "<<getMesiName(ps)<<" to "<<getMesiName(INSTRCACHE[set].LINE[curr_way].MESI)<<endl;
							}
						}
						
						INSTRCACHE[set].LINE[curr_way].TAG = tag;
						//UpdateLRU
						int temp= INSTRCACHE[set].LINE[curr_way].LRU;
						for (int j = 0; j < 4; j++) {
							INSTRCACHE[set].LINE[j].LRU = (INSTRCACHE[set].LINE[j].LRU < temp) ? INSTRCACHE[set].LINE[j].LRU + 1 : INSTRCACHE[set].LINE[j].LRU;
						}
						INSTRCACHE[set].LINE[curr_way].LRU=0;
						instruction_cache_miss++;
						cout<< "Read from L2 address       "<<std::hex<<address<<endl;
					}
					
				}
				//for(int i=0; i<4; i++)
					//cout<<INSTRCACHE[set].LINE[i].LRU<<"\t";
				if(NormalMode){
					cout<<endl;
					cout<<endl;
				}
			}
//----------------------------------Invalidate Command---------------------------------
			else if(instr==3){
				if(NormalMode){
					cout<<"invalidate command from L2 for address "<<std::hex<<address<<endl;
				}
				set=get_set(address);
				tag=get_tag(address);
				for(int i=0; i<8; i++){
					if(DATACACHE[set].LINE[i].TAG==tag && DATACACHE[set].LINE[i].MESI!=I)
						DATACACHE[set].LINE[i].MESI=I;
				}
				
				cout<<endl;
				
			}
//----------------------------------Data Request from L2---------------------------------
			else if(instr==4){
				if(NormalMode){
					cout<<"Return data to L2 address \t"<<std::hex<<address<<endl;
				}
				set=get_set(address);
				tag=get_tag(address);
				cout<<endl;
				for(int i=0;i<8;i++) {
					if (DATACACHE[set].LINE[i].MESI != I && DATACACHE[set].LINE[i].TAG == tag) {	
						if (DATACACHE[set].LINE[i].MESI == M){
							DATACACHE[set].LINE[i].MESI = I;
							cout<<"Return data to L2 address \t"<<std::hex<<address<<endl;
						}
						else
							DATACACHE[set].LINE[i].MESI = I;
					}
				}
			}
//----------------------------------Clear Cache---------------------------------
			else if(instr==8){
				if(NormalMode){
					cout<<"clear the cache and reset all state for address "<<std::hex<<address<<endl;
				}
				cout<<"Clearing the Cache!"<<endl;
				for(int i=0;i<16384;i=i+1){
					for(int j=0;j<8;j=j+1){
						DATACACHE[i].LINE[j].MESI=I;
					}
				}
				for(int i=0;i<16384;i=i+1){
					for(int j=0;j<4;j=j+1){
						INSTRCACHE[i].LINE[j].MESI=I;
					}
				}
			}
//----------------------------------Print Contents---------------------------------
			else if(instr==9){
				double s=0;
				if(NormalMode){
					cout<<"print contents and state of the cache"<<endl;
				}
				for(int i=0;i<16384;i=i+1){
				for(int j=0;j<8;j=j+1){
					if(DATACACHE[i].LINE[j].MESI==I){
						s++;
					}
				}
				}
				for(int i=0;i<16384;i=i+1){
				for(int j=0;j<4;j=j+1){
					if(INSTRCACHE[i].LINE[j].MESI==I){
						s++;
					}
				}
				}

			if ((s==16384*8)||(s==16384*4)){
				cout<<"-----------EMPTY CACHE----------"<<endl;
			}

			else{
				cout<< endl;
				cout<<"DATA CACHE"<<endl;
				cout<<"-------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
				cout<<" SET"<<" \t\t W0 \t\t W1 \t\t W2 \t\t W3 \t\t W4 \t\t W5 \t\t W6 \t\t W7 \t\t    "<<endl;
				cout<<"-------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
				for(int i=0;i<16384;i=i+1){
					for(int j=0;j<8;j=j+1){
						if (DATACACHE[i].LINE[j].MESI!=I) {
							cout<<" "<<std::hex<<i<<" -\t\t";
							for (int k=0;k<8;k=k+1) {
								if (DATACACHE[i].LINE[k].MESI!=I) {
									cout<<" "<<std::hex<<DATACACHE[i].LINE[k].TAG<<"("<<getMesiName(DATACACHE[i].LINE[k].MESI)<<")["<<DATACACHE[i].LINE[k].LRU<<"]";
								}
								else if (DATACACHE[i].LINE[k].MESI==I) {
									cout<<"X"<<"("<<getMesiName(DATACACHE[i].LINE[k].MESI)<<")";
								}
							cout<<"\t";
							}
							cout<<"   ";
							cout<<endl;
							cout<<"-------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
							break;
						}
					}
				}
				cout<< endl;
				cout<<"INSTRUCTION CACHE"<<endl;
				cout<<"-------------------------------------------------------------------------------------------"<<endl;
				cout<<" SET"<<" \t\t W0 \t W1 \t W2 \t W3 \t     "<<endl;
				cout<<"-------------------------------------------------------------------------------------------"<<endl;
				for(int i=0;i<16384;i=i+1){
				for(int j=0;j<4;j=j+1){
					if (INSTRCACHE[i].LINE[j].MESI!=I) {
						cout<<" "<<std::hex<<i<<" -\t\t";
						for (int k=0;k<4;k=k+1) {
							if (INSTRCACHE[i].LINE[k].MESI!=I) {
								cout<<" "<<std::hex<<INSTRCACHE[i].LINE[k].TAG<<"("<<getMesiName(INSTRCACHE[i].LINE[k].MESI)<<")["<<INSTRCACHE[i].LINE[k].LRU<<"]";
							}
							else if (INSTRCACHE[i].LINE[k].MESI==I) {
								cout<<"X"<<"("<<getMesiName(INSTRCACHE[i].LINE[k].MESI)<<")";
							}
						cout<<"\t";
						}
						cout<<"   ";
						cout<<endl;
						cout<<"-------------------------------------------------------------------------------------------"<<endl;
						break;
					}
				}
				}
				
			}
				
				cout<<endl;
			}
		}
		infile.close();
		DATAhitratio= ((data_cache_hit)*100.0f)/(data_cache_hit + data_cache_miss);
		INSTRhitratio= ((instruction_cache_hit)*100.0f)/(instruction_cache_hit + instruction_cache_miss);
		hitratio= ((data_cache_hit+instruction_cache_hit)*100.0f)/(data_cache_hit + data_cache_miss + instruction_cache_hit + instruction_cache_miss);
		//hitratio= round1(hitratio);
		cout<< endl;
		cout<<"DATA CACHE STATISTICS"<<endl;
		cout<<"------------------------------------------"<<endl;
		std::cout << std::fixed << std::setprecision(4);
		cout<<"| Number of Cache Hits:    \t|  "<<data_cache_hit<<"       \t  |"<<std::dec<<endl;
		cout<<"| Number of Cache Misses:   \t|  "<<data_cache_miss<<"     \t  |"<<std::dec<<endl;
		cout<<"| Number of Cache Reads:   \t|  "<<data_cache_read<<"      \t  |"<<std::dec<<endl;
		cout<<"| Number of Cache Writes:   \t|  "<<data_cache_write<<"    \t  |"<<std::dec<<endl;
		cout<<"| Cache Hit Ratio (%):  \t|  "<<DATAhitratio<<"    \t  |"<<endl;
		cout<<"------------------------------------------"<<endl;
		cout<< endl;
		
		cout<<"INSTRUCTION CACHE STATISTICS"<<endl;
		cout<<"------------------------------------------"<<endl;
		std::cout << std::fixed << std::setprecision(4);
		cout<<"| Number of Cache Hits:    \t|  "<<instruction_cache_hit<<"       \t  |"<<std::dec<<endl;
		cout<<"| Number of Cache Misses:   \t|  "<<instruction_cache_miss<<"     \t  |"<<std::dec<<endl;
		cout<<"| Number of Cache Reads:   \t|  "<<instruction_cache_read<<"      \t  |"<<std::dec<<endl;
		cout<<"| Cache Hit Ratio (%):  \t|  "<<INSTRhitratio<<"    \t  |"<<endl;
		cout<<"------------------------------------------"<<endl;
		cout<< endl;
		
		
		cout<<"CACHE STATISTICS"<<endl;
		cout<<"------------------------------------------"<<endl;
		std::cout << std::fixed << std::setprecision(4);
		cout<<"| Number of Cache Hits:    \t|  "<<data_cache_hit+instruction_cache_hit<<"       \t  |"<<std::dec<<endl;
		cout<<"| Number of Cache Misses:   \t|  "<<data_cache_miss+instruction_cache_miss<<"    \t  |"<<std::dec<<endl;
		cout<<"| Number of Cache Reads:   \t|  "<<data_cache_read + instruction_cache_read<<"   \t  |"<<std::dec<<endl;
		cout<<"| Number of Cache Writes:   \t|  "<<data_cache_write<<"      \t  |"<<std::dec<<endl;
		cout<<"| Cache Hit Ratio (%):  \t|  "<<hitratio<<"    \t  |"<<endl;
		cout<<"------------------------------------------"<<endl;
		
	}
	return 0;
}




