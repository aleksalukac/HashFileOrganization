#include "overflow_file.hpp"
#pragma warning(disable:4996)
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include "hash_file.hpp"
#include "bucket.hpp"

#define BLOCKS 50

using namespace std;

string reverse_string(string s)
{
	string newS = "";
	for (int i = s.length() - 1; i >= 0; i--)
	{
		newS += s[i];
	}
	return newS;
}

int transformKey(int key/*, unsigned int length = 2u*/)
{
	unsigned int length = 2u;
	string s = to_string(key);
	s = reverse_string(s);
	//reverse(s.begin(), s.end());

	while (s.length() % length != 0)
	{
		s += "0";
	}

	int i = 0;
	int k = 1;
	int sum = 0;
	while (i < s.length())
	{
		string str = s.substr(i, length);
		if (k % 2)
			str = reverse_string(str);
		//reverse(str.begin(), str.end());

		int r = atoi(str.c_str());
		sum += r;
		i += length;
		k++;
	}

	return (sum + 1) % B;
}

string overflow(string name)
{
	return "overf_" + name;
}

FILE* createOverflowFile(string fileName)
{
	fileName = overflow(fileName);
	FILE* pFile;
	pFile = fopen(fileName.c_str(), "rb+");

	if (pFile == NULL)
	{
		pFile = fopen(fileName.c_str(), "wb+");

		Block* emptyContent = (Block*)calloc(BLOCKS, sizeof(Block));
		emptyContent[0].nextPosition = B + 1;
		emptyContent[0].previousPosition = -1;
		emptyContent[0].currentPosition = 0;


		for (int i = 1; i < BLOCKS; i++)
		{
			if (i < 8)
			{
				emptyContent[i].nextPosition = -1;
				emptyContent[i].currentPosition = i;
				emptyContent[i].previousPosition = -1;
			}
			else
			{
				emptyContent[i].previousPosition = i - 1;
				if (i == 8)
					emptyContent[i].previousPosition = 0;
				emptyContent[i].currentPosition = i;
				emptyContent[i].nextPosition = i + 1;

				if (i == BLOCKS - 1)
					emptyContent[i].nextPosition = -1;
			}
		}

		fseek(pFile, 0, SEEK_SET);
		fwrite(emptyContent, sizeof(Block), BLOCKS, pFile);
		fflush(pFile);
		free(emptyContent);


		cout << "Kreirana prazna overflow datoteka." << endl;
	}
	else
	{
		cout << "Greska prilikom kreiranja overflow datoteke" << endl;
		exit(0);
	}
	return pFile;
}

void printBlock(Block block)
{
	printRecord(block.record, 1);
	cout << endl << "\tNext position: " << block.nextPosition << endl;
}

int readBlock(FILE* pFile, Block* pBlock, int blockIndex)
{
	fseek(pFile, blockIndex * sizeof(Block), SEEK_SET);
	return fread(pBlock, sizeof(Block), 1, pFile) == 1;
}

int saveBlock(FILE* pFile, Block* pBlock, int blockIndex)
{
	fseek(pFile, blockIndex * sizeof(Block), SEEK_SET);
	int retVal = fwrite(pBlock, sizeof(Block), 1, pFile) == 1;
	fflush(pFile);
	return retVal;
}

void printContentOverflow(FILE* pFile, int key)
{
	//printHeader();
	int index = key + 1;
	while (index != -1)
	{
		Block* block = new Block();
		readBlock(pFile, block, index);
		if (block->record.status == ACTIVE)
			printRecord(block->record, 0);
		index = block->nextPosition;
	}
}

FoundRecord findRecordOverflow(FILE* pFile, int key)
{
	int blockIndex = transformKey(key) + 1;
	FoundRecord fr;
	fr.bucketIndex = blockIndex - 1;
	fr.overflowRecord = true;
	fr.found = false;

	do
	{
		Block* qBlock = new Block();
		fr.position = blockIndex;
		readBlock(pFile, qBlock, blockIndex);
		if (qBlock->record.movieProjection.getId() == key)
		{
			fr.record = qBlock->record;
			fr.found = true;
			//fr.position = qBlock->currentPosition;
			break;
		}
		blockIndex = qBlock->nextPosition;

	} while (blockIndex != -1);

	return fr;
}

int deleteBlock(FILE* pFile, int key)
{
	FoundRecord fr = findRecordOverflow(pFile, key);
	if (!fr.found)
		return 0;

	Block* block = new Block();
	readBlock(pFile, block, fr.position);

	Block* prevBlock = new Block();
	readBlock(pFile, prevBlock, block->previousPosition);
	prevBlock->nextPosition = block->nextPosition;
	saveBlock(pFile, prevBlock, prevBlock->currentPosition);

	Block* nextBlock = new Block();

	if (block->nextPosition != -1)
	{
		readBlock(pFile, nextBlock, block->nextPosition);
		nextBlock->previousPosition = block->currentPosition;
		saveBlock(pFile, nextBlock, nextBlock->currentPosition);
	}

	Block* zero = new Block();
	readBlock(pFile, zero, 0);
	block->nextPosition = zero->nextPosition;

	readBlock(pFile, nextBlock, block->nextPosition);
	nextBlock->previousPosition = block->currentPosition;
	saveBlock(pFile, nextBlock, nextBlock->currentPosition);

	zero->nextPosition = block->currentPosition;
	saveBlock(pFile, zero, 0);

	block->record.status = LOGDELETED;
	saveBlock(pFile, block, block->currentPosition);

	return 1;
}

int getNextEmptyBlockIndex(FILE* pFile)
{
	Block* zero = new Block();
	readBlock(pFile, zero, 0);
	int blockIndex = zero->nextPosition;
	Block* next = new Block();
	readBlock(pFile, next, blockIndex);
	readBlock(pFile, next, next->nextPosition);
	zero->nextPosition = next->currentPosition;
	next->previousPosition = 0;

	saveBlock(pFile, zero, 0);
	saveBlock(pFile, next, next->currentPosition);

	return blockIndex;
}

bool insertRecordOverflow(FILE* pFile, Record record)
{
	FoundRecord fr = findRecordOverflow(pFile, record.movieProjection.getId());

	if (!fr.found)
	{
		//zauzimamo novu praznu lokaciju, prevezemo listu praznih, dodamo ovog na kraj liste za taj bucket
		int newBlockIndex = getNextEmptyBlockIndex(pFile);

		Block* previousBlock = new Block();
		Block* newBlock = new Block();
		readBlock(pFile, previousBlock, fr.position);
		previousBlock->nextPosition = newBlockIndex;
		saveBlock(pFile, previousBlock, previousBlock->currentPosition);

		readBlock(pFile, newBlock, newBlockIndex);
		newBlock->previousPosition = fr.position;
		newBlock->nextPosition = -1;
		newBlock->record = record;
		newBlock->record.status = ACTIVE;

		saveBlock(pFile, newBlock, newBlockIndex);

		return true;
	}
	return false;
}

int changeTheaterOverflow(FILE* pFile, int key)
{
	//record = findRecord()
	FoundRecord fr = findRecordOverflow(pFile, key);
	if (!fr.found)
		return 0;

	string newTc;
	do
	{
		cout << "Unesite novo ime sale (5 znakova): ";
		cin >> newTc;
	} while (!fr.record.movieProjection.setTheaterCode(newTc));

	Block* bl = new Block();
	readBlock(pFile, bl, fr.position);
	bl->record = fr.record;
	saveBlock(pFile, bl, fr.position);

	return 1;
}

FILE* safeFopenOverflow(string fileName) {
	FILE* pFile;

	pFile = fopen(overflow(fileName).c_str(), "rb+");

	if (pFile == NULL)
	{
		pFile = createOverflowFile(fileName);
		cout << "Kreirana prazna datoteka." << endl;
	}
	else
	{
		cout << "Otvorena postojeca datoteka." << endl;
	}

	if (pFile == NULL)
	{
		cout << "Nije moguce otvoritili ili kreirati datoteku sa zadatim imenom" << endl;
	}

	return pFile;
}

void showRecordsDurationOverflow(FILE* pFile, int minduration, int maxduration)
{
	//printHeader();

	for (int i = 0; i < B; i++)
	{
		int index = i + 1;
		while (index != -1)
		{
			Block* block = new Block();
			readBlock(pFile, block, index);
			if (block->record.status == ACTIVE && minduration <= block->record.movieProjection.getMovieDuration() && maxduration >= block->record.movieProjection.getMovieDuration())
				printRecord(block->record, 0);
			index = block->nextPosition;
		}
	}
}
