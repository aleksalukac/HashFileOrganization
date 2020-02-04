#ifndef HASHFILE_HPP
#define HASHFILE_HPP

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>

using namespace std;

#include "bucket.hpp"
#include "overflow_file.hpp"

void createHashFile(FILE* pFile, string fileName);
int initHashFile(FILE* pFile, FILE* pFilein);

FoundRecord findRecord(FILE* pFile, FILE* ovFile, int key);
FoundRecord findRecord(File file, FILE* ovFile, int key);
int insertRecord(FILE* pFile, FILE* ovFile, Record record);
int modifyRecord(FILE* pFile, FILE* ovFile, Record record);
int removeRecord(FILE* pFile, FILE* ovFile, int key);
void printContent(FILE* pFile, FILE* ovFile);
void changeTheater(FILE* pFile, FILE* ovFile, int key);

#endif
