
#ifndef _SR_H_
#define _SR_H_

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include "Eigen/Dense"
#include "readcsv.h"
#include "basicoperator.h"
using namespace std;
using namespace Eigen;

template<class DataType>
class SR {

public:

	//���캯��
	SR();
	SR(vector<string> DicFilePath);

	//�����ֵ� ��0���ֵ��ӦDicFilePath[0]
	bool LoadDic(vector<string> DicFilePath);

	//ϡ���ʾ���ࣨһ��������
	int SRClassify(vector<DataType>& y, DataType min_residual, int sparsity);

	//ϡ���ʾ���ࣨ����������
	int SRClassify(vector<vector<DataType>>& y, DataType min_residual, int sparsity);

	//��������
	~SR();

private:

	vector<vector<DataType>> dic;		//����ֵ� ��vector<DataType>���У�
	vector<int> dicclassnum;			//ÿ���ֵ������
	int	classnum;						//��ĸ���


private:
	DataType Dot(vector<DataType> vec1, vector<DataType> vec2);

	DataType Norm(vector<DataType> vec);

	int Max(vector<int> vec);

	//�����С��������
	bool solve(vector<vector<DataType>> phi, vector<DataType>& y, vector<DataType>& x);

	//OMP�㷨
	bool OrthMatchPursuit(vector<DataType>& y, DataType min_residual, int sparsity, vector<DataType>& x, vector<int>& patch_indices);



};

//���캯��
template<class DataType>
SR<DataType>::SR()
{
	this->classnum = 0;
	dicclassnum.clear();
}
template<class DataType>
SR<DataType>::SR(vector<string> DicFilePath)
{
	this->LoadDic(DicFilePath);
}

template<class DataType>
int SR<DataType>::SRClassify(vector<DataType>& y, DataType min_residual, int sparsity)
{
	/*SRClassify		ϡ���ʾʶ��
	*y					����
	*min_residual		��С�в�
	*sparsity			ϡ���
	*return�����:��0, 1��2��3��...�� -1������
	*author:ys
	*date:2016.05.05
	*/
	int fsize = y.size(); //����ά��
	if (!fsize){
		cerr << "SRClassify:error." << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return -1;
	}
	int dcol = dic.size();      //�ֵ������
	if (!dcol){
		cerr << "SRClassify:error." << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return -1;
	}
	int drow = dic[0].size();   //�ֵ������
	if (!drow || drow != fsize){
		cerr << "SRClassify:error." << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return -1;
	}
	int i, j;
	vector<int> patch_indices;	//ϡ��ϵ�����к�
	vector<DataType> coefficient;//ϡ��ϵ��

	if ( !OrthMatchPursuit(y, min_residual, sparsity, coefficient, patch_indices) )//OMP��ϡ��ϵ��
	{
		vector<int>().swap(patch_indices);//�ͷ��ڴ�
		vector<DataType>().swap(coefficient);//�ͷ��ڴ�
		//patch_indices.clear();//�ͷ��ڴ�
		//coefficient.clear();//�ͷ��ڴ�
		cerr << "SRClassify:OMP error." << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return -1;
	}

	vector<DataType> x(dcol, 0.0); //ϡ���
	for (i = 0; i < patch_indices.size(); i++) {
		x[patch_indices[i]] = coefficient[i];
	}

	vector<int>().swap(patch_indices);//�ͷ��ڴ�
	vector<DataType>().swap(coefficient);//�ͷ��ڴ�
	//patch_indices.clear();//�ͷ��ڴ�
	//coefficient.clear();//�ͷ��ڴ�

	int result = 0;//������ �����ֵ��Ӧ����

	int start = 0;//ĳһ�࿪ʼλ��
	DataType mindist = 100000000;
	for (i = 0; i < classnum; i++)
	{
		vector<DataType> tmp(fsize, 0.0);
		for (j = start; j < dicclassnum[i]; j++)
		{
			if (x[j] != 0.0)
			{
				try
				{
					tmp = tmp + dic[j] * x[j];
				}
				catch (runtime_error &e)
				{
					vector<DataType>().swap(tmp);//�ͷ��ڴ�
					//tmp.clear();//�ͷ��ڴ�

					vector<DataType>().swap(x);//�ͷ��ڴ�
					//x.clear();//�ͷ��ڴ�

					cerr << "SRClassify:error." << endl;
					cerr << e.what() << endl;
					cerr << "file:" << __FILE__ << endl;
					cerr << "line: " << __LINE__ << endl;
					cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
					return -1;

				}
				
			}

		}
		DataType dist;
		try
		{
			dist = Norm(y - tmp);
		}
		catch (runtime_error &e)
		{
			vector<DataType>().swap(tmp);//�ͷ��ڴ�
			//tmp.clear();//�ͷ��ڴ�

			vector<DataType>().swap(x);//�ͷ��ڴ�
			//x.clear();//�ͷ��ڴ�

			cerr << "SRClassify:error." << endl;
			cerr << e.what() << endl;
			cerr << "file:" << __FILE__ << endl;
			cerr << "line: " << __LINE__ << endl;
			cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
			return -1;

		}

		vector<DataType>().swap(tmp);//�ͷ��ڴ�
		//tmp.clear();//�ͷ��ڴ�

		if (mindist > dist)
		{
			mindist = dist;
			result = i;//
		}

		start += dicclassnum[i];
	}
	vector<DataType>().swap(x);//�ͷ��ڴ�
	//x.clear();//�ͷ��ڴ�

	return result; //result��0��1��2��3��...��

}

//ϡ���ʾ���ࣨ����������
template<class DataType>
int SR<DataType>::SRClassify(vector<vector<DataType>>& y, DataType min_residual, int sparsity){
	int i;
	int size = y.size();
	vector<int> result(this->classnum, 0);
	for (i = 0; i < size; i++)
	{
		int tmp = SRClassify(y[i], min_residual, sparsity);
		if (tmp < 0)
		{
			cerr << "SRClassify:error." << endl;
			cerr << "file:" << __FILE__ << endl;
			cerr << "line: " << __LINE__ << endl;
			cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
			vector<int>().swap(result);//�ͷ��ڴ�
			return -1;
		}
		result[tmp]++;
	}
	
	int maxindex;
	try
	{
		maxindex = Max(result);
	}
	catch (runtime_error &e)
	{
		vector<int>().swap(result);//�ͷ��ڴ�
		//result.clear();//�ͷ��ڴ�

		cerr << "SRClassify:error." << endl;
		cerr << e.what() << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return -1;

	}
	vector<int>().swap(result);//�ͷ��ڴ�
	//result.clear();//�ͷ��ڴ�

	return maxindex;
}

//��������
template<class DataType>
SR<DataType>::~SR(){
	//�ͷ��ֵ��ڴ�
	int i;
	int size = dic.size();
	for (i = 0; i < size; i++) 
	{
		vector<DataType>().swap(dic[i]);
		//dic[i].clear();
	}
	vector<vector<DataType>>().swap(dic);
	//dic.clear();
	vector<int>().swap(dicclassnum);
}

//���������ڻ�
template<class DataType>
DataType SR<DataType>::Dot(vector<DataType> vec1, vector<DataType> vec2)
{
	/*Norm				������������ڻ�
	*vec1				���飨N��
	*vec2				���飨N��
	*return��DataType
	*author:ys
	*date:2016.05.05
	*/
	if (!vec1.size() || !vec2.size() || vec1.size() != vec2.size()){
		cerr << "Dot:error." << endl;
		throw runtime_error("Dot:error");
	}
	DataType sum = 0;
	int i;
	int len = vec1.size();
	for (i = 0; i < len; i++)
	{
		sum += vec1[i] * vec2[i];
	}

	return sum;
}

//����2����
template<class DataType>
DataType SR<DataType>::Norm(vector<DataType> vec)
{
	/*Norm				�������2����
	*vec				���飨N��
	*return��DataType
	*author:ys
	*date:2016.05.05
	*/
	if (!vec.size()){
		cerr << "Norm:error." << endl;
		throw runtime_error("Norm:error");
	}
	DataType norm = 0;
	int i;
	int len = vec.size();
	for (i = 0; i < len; i++)
	{
		norm += vec[i] * vec[i];
	}

	return sqrt(norm);
}

template<class DataType>
int SR<DataType>::Max(vector<int> vec){
	if (!vec.size()){
		cerr << "Max:error." << endl;
		throw runtime_error("Max:error");
	}
	int index = 0;
	int i;
	int len = vec.size();
	for (i = 1; i < len; i++)
	{
		if (vec[1] > vec[index])
		{
			index = i;
		}
	}

	return index;
}

template<class DataType>
bool SR<DataType>::LoadDic(vector<string> DicFilePath){
	this->classnum = DicFilePath.size();
	if (this->classnum == 0) {
		cerr << "LoadDic:Dic Path error." << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return false;
	} else {
		cout << "the classnum is " << this->classnum << "." << endl;
	}

	ReadCsvFile<DataType> readcsv;
	int i;
	int num;
	for (i = 0; i < this->classnum; i++) {
		cout << "loading " << DicFilePath[i] << ". ";
		if (!readcsv.ReadCsv(DicFilePath[i], this->dic, num)) {
			cerr << "LoadDic:ReadCsv error." << endl;
			cerr << "file:" << __FILE__ << endl;
			cerr << "line: " << __LINE__ << endl;
			cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		}	
		cout << "the cols of class " << i << " are: " << num << endl;
		dicclassnum.push_back(num);
	}

	return true;
}

template<class DataType>
bool SR<DataType>::solve(vector<vector<DataType>> phi, vector<DataType>& y, vector<DataType>& x) {
	int col = phi.size();
	if (col <= 0) {
		cerr << "solve: error." << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return false;
	}
	int row = phi[0].size();
	if (row != y.size() || col != x.size()) {
		cerr << "solve: error." << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return false;
	}
	MatrixXd A(row, col);
	VectorXd b(row);
	int i, j;
	for (i = 0; i < row; i++)
	{
		b(i) = y[i];
		for (j = 0; j < col; j++)
			A(i, j) = phi[j][i];
	}
	//jacobiSvd ��ʽ:Slow (but fast for small matrices)
	//cout << "The least-squares solution is:\n"<< A.jacobiSvd(ComputeThinU | ComputeThinV).solve(b) << std::endl;
	//colPivHouseholderQr����:fast
	//std::cout << "The least-squares solution is:\n"<< A.colPivHouseholderQr().solve(b) << std::endl;

	VectorXd result = A.colPivHouseholderQr().solve(b);
	for (i = 0; i < col; i++)
	{
		x[i] = result(i);
	}

	return true;
}

//OMP�㷨
template<class DataType>
bool SR<DataType>::OrthMatchPursuit(vector<DataType>& y, DataType min_residual, int sparsity, vector<DataType>& x, vector<int>& patch_indices) {
	int fsize = y.size();
	if (!fsize)
	{
		cerr << "OrthMatchPursuit:error." << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return false;
	}
	int dcol = dic.size();      //�ֵ������
	if (!dcol)
	{
		cerr << "OrthMatchPursuit:error." << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return false;
	}
	int drow = dic[0].size();   //�ֵ������
	if (!drow || drow != fsize)
	{
		cerr << "OrthMatchPursuit:error." << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return false;
	}

	vector<DataType> residual(y); //�в�

	vector<vector<DataType>> phi; //������ѡ����ԭ������

	x.clear();

	DataType max_coefficient;
	unsigned int patch_index;
	int i;

	for (;;)
	{
		max_coefficient = 0;
		for (i = 0; i < dcol; i++)
		{
			DataType coefficient;
			try
			{
				coefficient = (DataType)Dot(dic[i], residual);
			}
			catch (runtime_error &e)
			{
				vector<DataType>().swap(residual);//�ͷ��ڴ�
				//residual.clear();//�ͷ��ڴ�
				for (i = 0; i < phi.size(); i++) vector<DataType>().swap(phi[i]);
				vector<vector<DataType>>().swap(phi);
				//phi.clear();//�ͷ��ڴ�
				cerr << "OrthMatchPursuit:error." << endl;
				cerr << e.what() << endl;
				cerr << "file:" << __FILE__ << endl;
				cerr << "line: " << __LINE__ << endl;
				cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
				return false;
			}
			if (fabs(coefficient) > fabs(max_coefficient))
			{
				max_coefficient = coefficient;
				patch_index = i;
			}
		}

		patch_indices.push_back(patch_index);

		phi.push_back(dic[patch_index]);

		x.push_back(0.0);

		if ( !solve(phi, y, x) )//�����С��������
		{
			cerr << "OrthMatchPursuit:error." << endl;
			cerr << "file:" << __FILE__ << endl;
			cerr << "line: " << __LINE__ << endl;
			cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
			vector<DataType>().swap(residual);//�ͷ��ڴ�
			//residual.clear();//�ͷ��ڴ�
			for (i = 0; i < phi.size(); i++) vector<DataType>().swap(phi[i]);
			vector<vector<DataType>>().swap(phi);
			//phi.clear();//�ͷ��ڴ�
			return false;
		}

		DataType res_norm;
		try
		{
			residual = y - phi *x;
			res_norm = (DataType)Norm(residual);
		}
		catch (runtime_error &e)
		{
			vector<DataType>().swap(residual);//�ͷ��ڴ�
			//residual.clear();//�ͷ��ڴ�
			for (i = 0; i < phi.size(); i++) vector<DataType>().swap(phi[i]);
			vector<vector<DataType>>().swap(phi);
			//phi.clear();//�ͷ��ڴ�
			cerr << "OrthMatchPursuit:error." << endl;
			cerr << e.what() << endl;			
			cerr << "file:" << __FILE__ << endl;
			cerr << "line: " << __LINE__ << endl;
			cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
			return false;

		}
		

		//if (x.size() >= sparsity || res_norm <= min_residual) //����ϡ������Ͳв���Ϊ��ֹ����
		if (x.size() >= sparsity)
			break;
	}

	vector<DataType>().swap(residual);//�ͷ��ڴ�
	//residual.clear();//�ͷ��ڴ�
	for (i = 0; i < phi.size(); i++) vector<DataType>().swap(phi[i]);
	vector<vector<DataType>>().swap(phi);
	//phi.clear();//�ͷ��ڴ�

	return true;
}



#endif
