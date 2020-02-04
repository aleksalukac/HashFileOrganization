#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <fstream>
#pragma warning(disable:4996)
using namespace std;

#define B 7             // broj baketa
#define STEP 1          // fiksan korak
#define BUCKET_SIZE 5   // faktor baketiranja
#define BLOCKS 50		// broj blokova

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
void showRecordsDurationOverflow(FILE* pFile, FILE* serialFile, int minduration, int maxduration);
void showRecordsDuration(FILE* pFile, FILE* ovFile, FILE* serialFile, int minduration, int maxduration);

int transformKey(int key);
void printRecord(Record record, int header);
void printBucket(Bucket bucket);
Record scanRecord();
int scanKey();
void printHeader();

void createHashFile(FILE* pFile, string fileName);
FoundRecord findRecord(FILE* pFile, FILE* ovFile, int key);
bool insertRecord(FILE* pFile, FILE* ovFile, Record record);
bool removeRecord(FILE* pFile, FILE* ovFile, int key);
void printContent(FILE* pFile, FILE* ovFile);
bool changeTheater(FILE* pFile, FILE* ovFile, int key);
int readBucket(FILE* pFile, Bucket* pBucket, int bucketIndex);

FILE* createSerialFile();
bool insertRecordSerial(FILE* pFile, Record record);
void printRecordsFromSerialFile(FILE* pFile);

int menu();
FILE* safeFopen(string filename);

int main() 
{
	FILE* pFile = NULL, * pInputSerialFile, * pInputTxtFile;
	FILE* ovFile = NULL;
	FILE* serialFile = NULL;

	string fileName;
	int option = -1;
	int key;
	Record record;

	while (option)
	{
		option = menu();
		switch (option) 
		{
		case 1:
			if (pFile != NULL) fclose(pFile);
			printf("\nUnesite naziv datoteke: ");
			cin >> fileName;
			pFile = safeFopen(fileName);
			ovFile = safeFopenOverflow(fileName);
			break;

		case 2:
			if (pFile == NULL)
				cout << "Nijedna datoteka nije otvorena" << endl;
			else
				cout << "Otvorena datoteka: " << fileName << endl;
			break;

		case 3:
			if (pFile == NULL)
			{
				cout << "Nijedna datoteka nije otvorena" << endl;
				break;
			}
			record = scanRecord();
			if(insertRecord(pFile, ovFile, record))
			{
				cout << "Uspesno uneseno." << endl;
			}
			else
			{
				cout << "Neuspesno uneseno." << endl;
			}
			break;

		case 4:
		{
			if (pFile == NULL)
			{
				cout << "Nijedna datoteka nije otvorena" << endl;
				break;
			}
			cout << "Unesite ID sloga koji trazite: ";
			cin >> key;

			FoundRecord fr = findRecord(pFile, ovFile, key);
			if (!fr.found)
			{
				cout << "Slog nije pronadjen ili je obrisan" << endl;
			}
			else 
			{
				cout << "\t Broj baketa: " << fr.bucketIndex << "\n\t Pozicija u baketu: " << fr.position;
				if (fr.overflowRecord)
					cout << " Overflow fajl" << endl;
				else
					cout << " Nije overflow fajl" << endl;
				printRecord(fr.record, 1); 
			}
		}
			break;

		case 5:
			if (pFile == NULL)
			{
				cout << "Nijedna datoteka nije otvorena" << endl;
				break;
			}
			printContent(pFile, ovFile);
			break;

		case 6:
			if (pFile == NULL)
			{
				cout << "Nijedna datoteka nije otvorena" << endl;
				break;
			}
			key = scanKey();
			if (removeRecord(pFile, ovFile, key))
			{
				cout << "Uspesno izbrisano" << endl;
			}
			else
			{
				cout << "Neuspesno izbrisano" << endl;
			}
			//handleResult(removeRecord(pFile, key));
			break;

		case 7:
			if (pFile == NULL)
			{
				cout << "Nijedna datoteka nije otvorena" << endl;
				break;
			}
			key = scanKey();
			changeTheater(pFile, ovFile, key);
			break;

		case 8:
		{
			if (pFile == NULL)
			{
				cout << "Nijedna datoteka nije otvorena" << endl;
				break;
			}

			cout << "Unesite ime csv fajla" << endl;
			string s;
			cin >> s;
			ifstream csvfile(s.c_str());
			int n;

			while (getline(csvfile, s, ',')) 
			{
				Record record;

				s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
				//cout << "ID: " << s << " ";
				record.movieProjection.setId(atoi(s.c_str()));

				getline(csvfile, s, ',');
				//cout << "Ime: " << s << " ";
				record.movieProjection.setMovieName(s);

				getline(csvfile, s, ',');
				//cout << "Datum: " << s << " ";
				record.movieProjection.setDateTime(s);

				getline(csvfile, s, ',');
				//cout << "Sala: " << s << " ";
				record.movieProjection.setTheaterCode(s);

				getline(csvfile, s, ',');
				//cout << "Trajanje: " << s << " ";
				record.movieProjection.setMovieDuration(atoi(s.c_str()));

				removeRecord(pFile, ovFile, record.movieProjection.getId());
				insertRecord(pFile, ovFile, record);
			}
		}
		break;

		case 9:
			if (pFile == NULL)
			{
				cout << "Nijedna datoteka nije otvorena" << endl;
				break;
			}
			int minduration, maxduration;
			do
			{
				cout << "Unesite minimalnu duzinu trajanja filma: ";
				cin >> minduration;
			} while (minduration < 0);

			do
			{
				cout << "Unesite maksimalnu duzinu trajanja filma: ";
				cin >> maxduration;
			} while (maxduration < 0);

			serialFile = createSerialFile();
			showRecordsDuration(pFile, ovFile, serialFile, minduration, maxduration);
			printRecordsFromSerialFile(serialFile);
			break;

		default:
			break;
		}
	}

	if (pFile != NULL) fclose(pFile);

	return 0;
}

int menu() {
	int option;

	cout << endl << "------------------------------" << endl;
	cout << "1. Izbor datoteke (formiranje nove ili aktivacija postojece ako postoji)" << endl;
	cout << "2. Prikaz naziva aktivne datoteke" << endl;
	cout << "3. Upis novog sloga u aktivnu datoteku" << endl;
	cout << "4. Trazenje sloga u aktivnoj datoteci" << endl;
	cout << "5. Prikaz svih slogova u bazi" << endl;
	cout << "6. Logicko brisanje sloga iz aktivne datoteke" << endl;
	cout << "7. Promena vrednosti oznake sale u odabranom slogu" << endl;
	cout << "8. Unos podataka u aktuelnu datoteku iz csv fajla" << endl;
	cout << "9. Izbor filmova sa duzinom u zadatom intervalu" << endl;
	cout << "0. Izlaz" << endl;
	cout << endl << "------------------------------" << endl << endl;

	printf(">>");

	scanf("%d", &option);

	return option;
}

FILE* safeFopen(string fileName) {
	FILE* pFile;

	pFile = fopen(fileName.c_str(), "rb+");

	if (pFile == NULL) 
	{
		pFile = fopen(fileName.c_str(), "wb+"); 
		createHashFile(pFile, fileName);
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

void printRecordsFromSerialFile(FILE* pFile)
{
	printHeader();
	Block* block = new Block();
	readBlock(pFile, block, 0);

	while (block->record.status != EMPTY)
	{
		printRecord(block->record, 0);
			
		if (block->nextPosition == -1)
			return;
		readBlock(pFile, block, block->nextPosition);
	}
}

bool insertRecordSerial(FILE* pFile, Record record)
{
	Block* block = new Block();
	readBlock(pFile, block, 0);
	while (block->record.status != EMPTY)
	{
		if (block->nextPosition == -1)
			return false;

		readBlock(pFile, block, block->nextPosition);
	}
	block->record = record;
	block->record.status = ACTIVE;
	saveBlock(pFile, block, block->currentPosition);
	return true;
}

FILE* createSerialFile()
{
	string fileName;
	cout << "Unesite ime serijske datoteke: ";
	cin >> fileName;

	FILE* pFile;
	pFile = fopen(fileName.c_str(), "rb+");

	if (pFile == NULL)
	{
		pFile = fopen(fileName.c_str(), "wb+");

		Block* emptyContent = (Block*)calloc(BLOCKS, sizeof(Block));

		for (int i = 0; i < BLOCKS; i++)
		{
			emptyContent[i].previousPosition = i - 1;
			if (i == 0)
				emptyContent[i].previousPosition = 0;
			emptyContent[i].currentPosition = i;
			emptyContent[i].nextPosition = i + 1;

			if (i == BLOCKS - 1)
				emptyContent[i].nextPosition = -1;
		}

		fseek(pFile, 0, SEEK_SET);
		fwrite(emptyContent, sizeof(Block), BLOCKS, pFile);
		fflush(pFile);
		free(emptyContent);


		//cout << "Kreirana prazna serijska datoteka." << endl;
	}
	else
	{
		cout << "Greska prilikom kreiranja serijske datoteke" << endl;
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
		cout << "Kreirana prazna Overflow datoteka." << endl;
	}
	else
	{
		cout << "Otvorena postojeca Overflow datoteka." << endl;
	}

	if (pFile == NULL)
	{
		cout << "Nije moguce otvoritili ili kreirati datoteku sa zadatim imenom" << endl;
	}

	return pFile;
}

void showRecordsDuration(FILE* pFile, FILE* ovFile, FILE* serialFile, int minduration, int maxduration)
{
	//printHeader();

	for (int i = 0; i < B; i++)
	{
		Bucket* bucket = new Bucket();
		readBucket(pFile, bucket, i);
		for (int j = 0; j < BUCKET_SIZE; j++)
		{
			if (bucket->records[j].status == ACTIVE && bucket->records[j].movieProjection.getMovieDuration() >= minduration)
				if (bucket->records[j].movieProjection.getMovieDuration() <= maxduration)
					//printRecord(bucket->records[i], 0);
					insertRecordSerial(serialFile, bucket->records[j]);
		}
	}
	showRecordsDurationOverflow(ovFile, serialFile, minduration, maxduration);
}

void showRecordsDurationOverflow(FILE* pFile, FILE* serialFile, int minduration, int maxduration)
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
				insertRecordSerial(serialFile, block->record);
				//printRecord(block->record, 0);
			index = block->nextPosition;
		}
	}
}

void printHeader()
{
	printf("status \t id \t\t Name \t Date \t Theater Code \t Duration\n");
}

void printRecord(Record record, int header)
{
	if (header)
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
	fr.position = -1;
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
			bucket->records[i] = record;
			bucket->records[i].status = ACTIVE;
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
