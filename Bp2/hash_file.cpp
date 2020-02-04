#include "hash_file.hpp"
#pragma warning(disable:4996)
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "overflow_file.hpp"

using namespace std;


string overflow(string name)
{
	return "overf_" + name;
}

void createHashFile(FILE* pFile, string fileName) 
{
	Bucket* emptyContent = (Bucket*)calloc(B, sizeof(Bucket));               
	fseek(pFile, 0, SEEK_SET);                                      
	fwrite(emptyContent, sizeof(Bucket), B, pFile);
	free(emptyContent);
}

int saveBucket(FILE* pFile, Bucket* pBucket, int bucketIndex) 
{
	fseek(pFile, bucketIndex * sizeof(Bucket), SEEK_SET);
	int retVal = fwrite(pBucket, sizeof(Bucket), 1, pFile) == 1;
	fflush(pFile);
	return retVal;
}

int readBucket(FILE* pFile, Bucket* pBucket, int bucketIndex) 
{
	fseek(pFile, bucketIndex * sizeof(Bucket), SEEK_SET);
	return fread(pBucket, sizeof(Bucket), 1, pFile) == 1;
}

int readBlock(FILE* pFile, Block* block, int blockIndex)
{
	fseek(pFile, blockIndex * sizeof(Block), SEEK_SET);
	return fread(block, sizeof(Block), 1, pFile) == 1;
}

int findNextBlock(FILE* pFile, Block* block, int previousIndex, int& index)
{
	fseek(pFile, previousIndex * sizeof(Block), SEEK_SET);
	fread(block, sizeof(Block), 1, pFile) == 1;
	index = block->nextPosition;
	if (index != -1)
	{
		return readBlock(pFile, block, index);
	}
	return 0;
}

int readRecordFromSerialFile(FILE* pFile, Record* pRecord) 
{
	return fread(pRecord, sizeof(Record), 1, pFile);
}

int saveRecordToOverflowFile(FILE* pFile, Record* pRecord) 
{
	return fwrite(pRecord, sizeof(Record), 1, pFile);
}

bool isBucketFull(Bucket bucket) 
{
	return bucket.records[BUCKET_SIZE - 1].status != EMPTY;
}

FoundRecord findRecord(FILE* pFile, FILE* ovFile, int key)
{
	FoundRecord fr;
	fr.bucketIndex = transformKey(key);
	Bucket bucket;

	FILE* activeFile = pFile;
	//string fileName = file.name;

	fr.overflowRecord = false;
	fr.found = false;
	fr.position= -1;
	readBucket(activeFile, &bucket, fr.bucketIndex);

	while (++fr.position < BUCKET_SIZE)
	{
		Record currentRecord = bucket.records[fr.position];

		if (currentRecord.status == EMPTY)
		{
			return fr;
		}

		if (key == currentRecord.movieProjection.getId())
		{
			fr.found = true; 
			if (currentRecord.status != ACTIVE)
				fr.found = false;
			fr.record = currentRecord;
			return fr;
		}
	}

	fr.overflowRecord = true;
	return findRecordOverflow(ovFile, key);
}

bool insertRecord(FILE* pFile, FILE* ovFile, Record record)
{
	FoundRecord fr = findRecord(pFile, ovFile, record.movieProjection.getId());

	if (fr.found)
	{
		return false;
	}

	Bucket* bucket = new Bucket();
	readBucket(pFile, bucket, fr.bucketIndex);

	for (int i = 0; i < BUCKET_SIZE; i++)
	{
		if (bucket->records[i].status != ACTIVE)
		{
			bucket->records[i].status = ACTIVE;
			bucket->records[i] = record;
			saveBucket(pFile, bucket, fr.bucketIndex);
			return true;
		}
	}
	return insertRecordOverflow(ovFile, record);
}

bool changeTheater(FILE* pFile, FILE* ovFile, int key)
{
	//record = findRecord()
	FoundRecord fr = findRecord(pFile, ovFile, key);
	Record record = fr.record;

	if (!fr.found)
	{
		return false;
	}

	if (fr.overflowRecord)
	{
		return changeTheaterOverflow(ovFile, key);
	}

	Bucket* bucket = new Bucket();
	readBucket(pFile, bucket, fr.bucketIndex);

	string newTc;
	do
	{
		cout << "Unesite novo ime sale (5 znakova): ";
		cin >> newTc;
	} while (!fr.record.movieProjection.setTheaterCode(newTc));
	
	bucket->records[fr.position] = fr.record;
	saveBucket(pFile, bucket, fr.bucketIndex);
	return true;	
}

bool removeRecord(FILE* pFile, FILE* ovFile, int key) 
{
	FoundRecord fr = findRecord(pFile, ovFile, key);
	if (!fr.found)
		return false;

	if (fr.overflowRecord)
		return deleteBlock(ovFile, key);

	Bucket* bucket = new Bucket();
	readBucket(pFile, bucket, fr.bucketIndex);
	
	bucket->records[fr.position].status = LOGDELETED;
	saveBucket(pFile, bucket, fr.bucketIndex);
	
	return true;
}

void printContent(FILE* pFile, FILE* ovFile) {
	int i;
	Bucket bucket;

	for (i = 0; i < B; i++) {
		readBucket(pFile, &bucket, i);
		printf("\n####### BUCKET - %d #######\n", i + 1);
		printBucket(bucket);
		printContentOverflow(ovFile, i);
	}
}
