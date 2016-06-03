#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

using namespace std;

//数组与数乘法
template<class DataType>
vector<DataType> operator*(vector<DataType> vec, DataType data) {
	int len = vec.size();
    if (!len) {
		cerr << "operator*:eeror." << endl;
		throw "operator*:eeror";
	}

	vector<DataType> result;

	int i;
	for (i = 0; i < len; i++)
		result.push_back(vec[i] * data);
	return result;
}

//数组减法
template<class DataType>
vector<DataType> operator-(vector<DataType> vec1, vector<DataType> vec2) {
    if (!vec1.size() || !vec2.size() || vec1.size() != vec2.size()) {
		cerr << "operator-:error." << endl;
		throw "operator-:error";
	}
	vector<DataType> result;
	int i;
	int len = vec1.size();
    for(i = 0; i < len; i++)
		result.push_back(vec1[i] - vec2[i]);

	return result;
}

//数组加法
template<class DataType>
vector<DataType> operator+(vector<DataType> vec1, vector<DataType> vec2) {
    if (!vec1.size() || !vec2.size() || vec1.size() != vec2.size()) {
		cerr << "operator+:error." << endl;
		throw "operator+:error";
	}
	vector<DataType> result;
	int i;
	int len = vec1.size();
	for (i = 0; i < len; i++)
		result.push_back(vec1[i] + vec2[i]);

	return result;
}

//矩阵与数组乘法
template<class DataType>
vector<DataType> operator*(vector<vector<DataType>> mat, vector<DataType> vec) {
	int row = mat.size();
    if (!row || row != vec.size()) {
		cerr << "operator*:eeror." << endl;
		throw "operator*:eeror";
	}
	int col = mat[0].size();
    if (!col) {
		cerr << "operator*:eeror." << endl;
		throw "operator*:eeror";
	}

	vector<DataType> result;
	DataType tmp;
	int i, j;
    for (i = 0; i < col; i++) {
		tmp = 0.0;
        for (j = 0; j < row; j++) {
			tmp += mat[j][i] * vec[j];
		}
		result.push_back(tmp);
	}
	return result;
}
