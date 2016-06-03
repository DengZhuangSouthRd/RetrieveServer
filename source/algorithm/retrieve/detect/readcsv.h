#ifndef _READCSVFILE_H_
#define _READCSVFILE_H_

#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

#define MAX_LINE_SIZE 100000

template<class DataType>
class ReadCsvFile{
public:
	//按行读取数据
	int ReadCsv(const string CsvFilePath, vector<vector<DataType>> &data);
	
private:
	//获取文件行数
	int GetTotalLineCount(FILE *fp);
};

//按行读取数据
template<class DataType>
int ReadCsvFile<DataType>::ReadCsv(const string CsvFilePath, vector<vector<DataType>> &data) {
    FILE* fcsv = NULL;
	char strline[MAX_LINE_SIZE];
	int row;
    if (fopen_s(&fcsv, CsvFilePath.c_str(), "r") != 0) {
		cerr << "Open File Error." << endl;
		throw "open file error";
    } else {
		row = GetTotalLineCount(fcsv);
        if (row == 0) {
			cerr << "file has no data." << endl;
            throw runtime_error("file has no data");
		}
	}
	int i;
	vector<DataType> tmp;
	char *nextoken = NULL;
	char delims[] = ",";//分割符
    for (i = 0; i < row; i++) {
		tmp.clear();
        if (fgets(strline, MAX_LINE_SIZE, fcsv)) {
			char * token = strtok_s(strline, delims, &nextoken);
            while (token != NULL) {
				tmp.push_back(atof(token));
				token = strtok_s(NULL, delims, &nextoken);
			}
			data.push_back(tmp);
        } else {
			cerr << "read csv file error." << endl;
            throw runtime_error("read csv file error");
		}
	}

	fclose(fcsv);//关闭文件

	return row;
}

//获取文件行数
template<class DataType>
int ReadCsvFile<DataType>::GetTotalLineCount(FILE *fp) {
	int i = 0;
	char strline[MAX_LINE_SIZE];
	fseek(fp, 0, SEEK_SET);
	while (fgets(strline, MAX_LINE_SIZE, fp))
		i++;
	fseek(fp, 0, SEEK_SET);
	return i;
}

#endif
