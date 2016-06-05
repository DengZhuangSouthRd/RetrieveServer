#ifndef _READCSVFILE_H_
#define _READCSVFILE_H_

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "../ASIFT/demo_lib_sift.h"

using namespace std;

#define MAX_LINE_SIZE 10000

template<class DataType>
class ReadCsvFile{

public:

	bool ReadCsv(const string CsvFilePath, vector<vector<DataType>> &data, int &size);

private:

	int GetTotalLineCount(FILE *fp);
};

template<class DataType>
bool ReadCsvFile<DataType>::ReadCsv(const string CsvFilePath, vector<vector<DataType>> &data, int &size) {
	FILE* fcsv = fopen(CsvFilePath.c_str(), "r");
	char strline[MAX_LINE_SIZE];
	if (!fcsv) {
		cerr << "ReadCsv:Open File Error." << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return false;
	} else {
		size = GetTotalLineCount(fcsv);
		if (size == 0) {
			cerr << "Warning:" << CsvFilePath<<" file has no data." << endl;
			cerr << "file:" << __FILE__ << endl;
			cerr << "line: " << __LINE__ << endl;
			cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
			return true;
		}
	}
	int i;
	vector<DataType> tmp;
	char delims[] = ",";
	for (i = 0; i < size; i++) {
		vector<DataType>().swap(tmp);
		if (fgets(strline, MAX_LINE_SIZE, fcsv)) {
			char * token = strtok(strline, delims);
			while (token != NULL)
			{
				//cout << atof(token) << endl;
				tmp.push_back(atof(token));
				token = strtok(NULL, delims);
			}
			if (tmp.size() != VecLength)
			{
				cerr << "ReadCsv:read csv file error." << endl;
				cerr << "file:" << __FILE__ << endl;
				cerr << "line: " << __LINE__ << endl;
				cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
				return false;
			}
			delete token; token = NULL;
			data.push_back(tmp);
		}
		else
		{
			cerr << "ReadCsv:read csv file error." << endl;
			cerr << "file:" << __FILE__ << endl;
			cerr << "line: " << __LINE__ << endl;
			cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
			return false;
		}
	}

	fclose(fcsv);

	return true;

}

template<class DataType>
int ReadCsvFile<DataType>::GetTotalLineCount(FILE *fp) {
	int i = 0;
	char strline[MAX_LINE_SIZE];
	fseek(fp, 0, SEEK_SET);

	while (fgets(strline, MAX_LINE_SIZE, fp))
	{
		i++;
	}
	fseek(fp, 0, SEEK_SET);

	return i;
}

#endif