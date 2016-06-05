#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
using namespace std;

//���������˷�
template<class DataType>
vector<DataType> operator*(vector<DataType> vec, DataType data)
{
	/*operator*			�����������
	*vec				���飨N��
	*data				��
	*return��vector<DataType>��N��
	*author:ys
	*date:2016.05.05
	*/


	int len = vec.size();
	if (!len)
	{
		cerr << "operator*:eeror." << endl;
		throw runtime_error("operator*:error");
	}


	vector<DataType> result;

	int i;
	for (i = 0; i < len; i++)
	{
		result.push_back(vec[i] * data);
	}
	return result;
}

//�������
template<class DataType>
vector<DataType> operator-(vector<DataType> vec1, vector<DataType> vec2)
{
	/*operator-			���������vec1-vec2��
	*vec1				���飨N��
	*vec2				���飨N��
	*return��vector<DataType>��N��
	*author:ys
	*date:2016.05.05
	*/
	if (!vec1.size() || !vec2.size() || vec1.size() != vec2.size()){
		cerr << "operator-:error." << endl;
		throw runtime_error("operator-:error");
	}
	vector<DataType> result;
	int i;
	int len = vec1.size();
	for (i = 0; i < len; i++)
	{
		result.push_back(vec1[i] - vec2[i]);
	}


	return result;
}

//����ӷ�
template<class DataType>
vector<DataType> operator+(vector<DataType> vec1, vector<DataType> vec2)
{
	/*operator-			���������vec1-vec2��
	*vec1				���飨N��
	*vec2				���飨N��
	*return��vector<DataType>��N��
	*author:ys
	*date:2016.05.05
	*/
	if (!vec1.size() || !vec2.size() || vec1.size() != vec2.size()){
		cerr << "operator+:error." << endl;
		throw runtime_error("operator+:error");
	}
	vector<DataType> result;
	int i;
	int len = vec1.size();
	for (i = 0; i < len; i++)
	{
		result.push_back(vec1[i] + vec2[i]);
	}


	return result;
}

//����������˷�
template<class DataType>
vector<DataType> operator*(vector<vector<DataType>> mat, vector<DataType> vec)
{
	/*operator*			�������������
	*mat				����N*M��
	*vec				���飨N��
	*return��vector<DataType>��M��
	*author:ys
	*date:2016.05.05
	*/


	int row = mat.size();
	if (!row || row != vec.size())
	{
		cerr << "operator*:eeror." << endl;
		throw runtime_error("operator*:error");
	}
	int col = mat[0].size();
	if (!col)
	{
		cerr << "operator*:eeror." << endl;
		throw runtime_error("operator*:error");
	}

	vector<DataType> result;
	DataType tmp;
	int i, j;
	for (i = 0; i < col; i++)
	{
		tmp = 0.0;
		for (j = 0; j < row; j++)
		{
			tmp += mat[j][i] * vec[j];
		}
		result.push_back(tmp);
	}
	return result;
}
