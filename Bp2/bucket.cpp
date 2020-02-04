#include "bucket.hpp"
#pragma warning(disable:4996)
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>

using namespace std;

int nextBucketIndex(int currentIndex) 
{
	//TODO
	return transformKey(currentIndex + STEP);
}

void printHeader() 
{
	printf("status \t id \t\t Name \t Date \t Theater Code \t Duration\n");
}

void printRecord(Record record, int header) 
{
	if(header)
		printHeader();
	cout << record.status << "\t";
	cout << record.movieProjection.getId() << "\t";
	cout << record.movieProjection.getMovieName() << "\t";
	cout << record.movieProjection.getDateTime() << "\t";
	cout << record.movieProjection.getTheaterCode() << "\t\t";
	cout << record.movieProjection.getMovieDuration() << "\n";
	//printf("%d \t %d \t %s \t %s\n", record.status, record.getId(), record.code, record.date);
}

void printBucket(Bucket bucket) 
{
	int i;
	printHeader();
	for (i = 0; i < BUCKET_SIZE; i++) 
	{
		printRecord(bucket.records[i], 0);
	}
}

Record scanRecord() 
{
	Record record;

	cout << endl << "\tUnesite informacije:" << endl;

	unsigned int Id;
	unsigned int trajanje;
	string temp;

	do
	{
		cout << "Id projekcije = ";
		cin >> Id;
	} while (!record.movieProjection.setId(Id));

	do
	{
		cout << "Ime projekcije = ";
		cin >> temp;
	} while (!record.movieProjection.setMovieName(temp));

	do
	{
		cout << "Datum projekcije = ";
		cin >> temp;
	} while (!record.movieProjection.setDateTime(temp));

	do
	{
		cout << "Sala projekcije = ";
		cin >> temp;
	} while (!record.movieProjection.setTheaterCode(temp));

	do
	{
		cout << "Trajanje filma = ";
		cin >> trajanje;
	} while (!record.movieProjection.setMovieDuration(trajanje));

	return record;
}

int scanKey() 
{
	int key;
	cout << "key = ";
	cin >> key;
	return key;
}
