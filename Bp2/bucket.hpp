#ifndef BUCKET_HPP
#define BUCKET_HPP
#pragma warning(disable:4996)

#define B 7             // broj baketa
#define STEP 1          // fiksan korak
#define BUCKET_SIZE 5   // faktor baketiranja

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "overflow_file.hpp"

using namespace std;

typedef enum { EMPTY = 0, ACTIVE, LOGDELETED } RECORD_STATUS;


class MovieProjection
{
private:
	unsigned int Id;
	char MovieName[43];
	char DateTime[25];
	char TheaterCode[7];
	unsigned int MovieDuration;

public:
	MovieProjection()
	{
		Id = 0;
		memset(MovieName, '0', 40 * sizeof(char));
		//DateTime = "01.01.1970. 00:00:00";
		memset(DateTime, '0', 25 * sizeof(char));
		//TheaterCode = "00000";
		memset(TheaterCode, '0', 7 * sizeof(char));
		MovieDuration = 0;
	}

	bool setId(unsigned int id)
	{
		if (9999999 >= id || 100000000 <= id)
		{
			return false;
		}

		this->Id = id;
		return true;
	}

	unsigned int getId()
	{
		return this->Id;
	}

	bool setMovieName(string name)
	{
		if (name.size() > 41)
		{
			return false;
		}

		strcpy(this->MovieName, name.c_str());
		//this->MovieName = name;
		return true;
	}

	string getMovieName()
	{
		return this->MovieName;
	}

	bool setDateTime(string dateTime)
	{
		//this->DateTime = dateTime;
		strcpy(this->DateTime, dateTime.c_str());
		return true;
	}

	string getDateTime()
	{
		return this->DateTime;
	}

	bool setTheaterCode(string theaterCode)
	{
		if (theaterCode.size() != 5)
			return false;

		strcpy(this->TheaterCode, theaterCode.c_str());
		//this->TheaterCode = theaterCode;
		return true;
	}

	string getTheaterCode()
	{
		return this->TheaterCode;
	}

	bool setMovieDuration(unsigned int duration)
	{
		if (duration >= 2000)
			return false;

		this->MovieDuration = duration;
		return true;
	}

	unsigned int getMovieDuration()
	{
		return this->MovieDuration;
	}

};

struct Record {
	MovieProjection movieProjection;
	RECORD_STATUS status = EMPTY;
};

struct Bucket
{
	Record records[BUCKET_SIZE];
	//bool hasOverflow = false;
};

struct FoundRecord
{
	Record record;
	bool found = false;
	int bucketIndex;
	int position;
	bool overflowRecord = false;
};

struct File
{
	FILE* file;
	string name;
};

typedef struct {
	int ind1;           // indikator uspesnosti trazenja, 0 - uspesno, 1 - neuspesno
	int ind2;           // indikator postojanja slobodnih lokacija, 0 - nema, 1 - ima
	Bucket bucket;
	Record record;
	int bucketIndex = 4;    // indeks baketa sa nadjenim slogom
	int recordIndex = -1;    // indeks sloga u baketu
} FindRecordResult;

int transformKey(int key);
int nextBucketIndex(int currentIndex);
void printRecord(Record record, int header);
void printBucket(Bucket bucket);
Record scanRecord();
int scanKey();

#endif
