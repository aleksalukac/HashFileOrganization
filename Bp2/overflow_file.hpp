#include "hash_file.hpp"
#include "bucket.hpp"
#include "hash_file.hpp"

#pragma warning(disable:4996)
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>

using namespace std; 

#define BLOCKS 50		// broj blokova -- faktor blokiranja 1


struct Block
{
	int nextPosition = -1;
	int previousPosition = -1;
	int currentPosition = -1;
	Record record; //faktor blokiranja je 1
};

string reverse_string(string s);
int transformKey(int key);
string overflow(string name);
FILE* createOverflowFile(string fileName);
void printRecord(Record record, int header);
void printBlock(Block block);
int readBlock(FILE* pFile, Block* pBlock, int blockIndex);
int saveBlock(FILE* pFile, Block* pBlock, int blockIndex);
void printContentOverflow(FILE* pFile, int key);
FoundRecord findRecordOverflow(FILE* pFile, int key);
int deleteBlock(FILE* pFile, int key);
int getNextEmptyBlockIndex(FILE* pFile);
bool insertRecordOverflow(FILE* pFile, Record record);
int changeTheaterOverflow(FILE* pFile, int key);
FILE* safeFopenOverflow(string fileName);
void showRecordsDurationOverflow(FILE* pFile, int minduration, int maxduration);