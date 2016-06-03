#ifndef _SR_H_
#define _SR_H_

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

#include "../../third_party/Eigen/Dense"
#include "readcsv.h"
#include "basicoperator.h"

using namespace std;
using namespace Eigen;

template<class DataType>
class SR {

public:
    static SR* p_SingleSR;
    static SR* getSingleInstance();

private:
	SR();
	SR(vector<string> DicFilePath);
    ~SR();

public:
    void setDictFilePaths(vector<string> DictFilePath);
	//加载字典 第0类字典对应DicFilePath[0]
	void LoadDic(vector<string> DicFilePath);

	//稀疏表示分类（一组特征）
	int SRClassify(vector<DataType>& y, DataType min_residual, int sparsity);

	//稀疏表示分类（多组特征）
	int SRClassify(vector<vector<DataType>>& y, DataType min_residual, int sparsity);

private:
	vector<vector<DataType>> dic;		//存放字典 （vector<DataType>：列）
	vector<int> dicclassnum;			//每类字典的列数
	int	classnum;						//类的个数

private:
	//两个数组内积
	DataType Dot(vector<DataType> vec1, vector<DataType> vec2);

	//数组2范数
	DataType Norm(vector<DataType> vec);

	//数组的最大值
	int Max(vector<DataType> vec);

	//求解最小二乘问题
	void solve(vector<vector<DataType>> phi, vector<DataType>& y, vector<DataType>& x);

	//OMP算法
	void OrthMatchPursuit(vector<DataType>& y, DataType min_residual, int sparsity, vector<DataType>& x, vector<int>& patch_indices);
};

template<class DataType>
SR* SR<DataType>::p_SingleSR = new (std::nothrow) SR<double>();

template<class DataType>
SR* SR<DataType>::getSingleInstance() {
    return p_SingleSR;
}

template<class DataType>
SR<DataType>::SR() {
	this->classnum = 0;
	dicclassnum.clear();
}

template<class DataType>
SR<DataType>::SR(vector<string> DicFilePath) {
	this->LoadDic(DicFilePath);
}

template<class DataType>
void SR<DataType>::setDictFilePaths(vector<string> DictFilePath) {
    this->LoadDic(DictFilePath);
}

//稀疏表示分类（一组特征）
template<class DataType>
int SR<DataType>::SRClassify(vector<DataType>& y, DataType min_residual, int sparsity) {
	/*SRClassify		稀疏表示识别
	*dic				字典（列*行）
	*dicclassnum		字典中每个类别的列数
	*y					特征
	*min_residual		最小残差
	*sparsity			稀疏度
	*return：类别（1，2，3，...）
	*author:ys
	*date:2016.05.05
	*/

	int fsize = y.size(); //特征维数
	if (!fsize){
		cerr << "SRClassify:error." << endl;
        throw "SRClassify:error";
	}

	int dcol = dic.size();      //字典的列数
	if (!dcol){
		cerr << "SRClassify:error." << endl;
		throw "SRClassify:error";
	}
	int drow = dic[0].size();   //字典的行数
	if (!drow || drow != fsize){
		cerr << "SRClassify:error." << endl;
		throw "SRClassify:error";
	}
	int i, j;
	vector<int> patch_indices;	//稀疏系数的列号
	vector<double> coefficient;//稀疏系数

	OrthMatchPursuit(y, min_residual, sparsity, coefficient, patch_indices);//OMP求稀疏系数

	vector<double> x(dcol, 0.0); //稀疏解
    for (i = 0; i < patch_indices.size(); i++) {
		cout << coefficient[i] << "  ";
		x[patch_indices[i]] = coefficient[i];
	}
	cout << endl;

	patch_indices.clear();//释放内存
	coefficient.clear();//释放内存

	int result = 0;//分类结果 属于字典对应的类

	int start = 0;//某一类开始位置
	DataType mindist = 100000000;
    for (i = 0; i < classnum; i++) {
		vector<DataType> tmp(fsize, 0.0);
        for (j = start; j < dicclassnum[i]; j++) {
            if (x[j] != 0.0) {
				tmp = tmp + dic[j] * x[j];
			}
		}
		DataType dist = Norm(y - tmp);

		tmp.clear();//释放内存

        if (mindist > dist) {
			mindist = dist;
			result = i;//
		}

		start += dicclassnum[i];
	}

	x.clear();//释放内存

	return result; //result（0，1，2，3，...）
}

//稀疏表示分类（多组特征）
template<class DataType>
int SR<DataType>::SRClassify(vector<vector<DataType>>& y, DataType min_residual, int sparsity) {
    int size = y.size(),i=0;
	vector<int> result(this->classnum, 0);
    for (i = 0; i < size; i++) {
		int tmp = SRClassify(y[i], min_residual, sparsity);
		result[tmp]++;
	}
	int maxindex = Max(result);
	result.clear();
	return maxindex;
}

//析构函数
template<class DataType>
SR<DataType>::~SR() {
    int size = dic.size(),i=0;
    for (i = 0; i < size; i++) {
		dic[i].clear();
	}
	dic.clear();
}

//两个数组内积
template<class DataType>
DataType SR<DataType>::Dot(vector<DataType> vec1, vector<DataType> vec2)
{
	/*Norm				求两个数组的内积
	*vec1				数组（N）
	*vec2				数组（N）
	*return：DataType
	*author:ys
	*date:2016.05.05
	*/
	if (!vec1.size() || !vec2.size() || vec1.size() != vec2.size()){
		cerr << "Dot:error." << endl;
		throw "Dot:error";
	}
	DataType sum = 0;
	int i;
	int len = vec1.size();
	for (i = 0; i < len; i++)
		sum += vec1[i] * vec2[i];

	return sum;
}

//数组2范数
template<class DataType>
DataType SR<DataType>::Norm(vector<DataType> vec)
{
	/*Norm				求数组的2范数
	*vec				数组（N）
	*return：DataType
	*author:ys
	*date:2016.05.05
	*/
	if (!vec.size()){
		cerr << "Norm:error." << endl;
		throw "Norm:error";
	}
	DataType norm = 0;
	int i;
	int len = vec.size();
	for (i = 0; i < len; i++)
		norm += vec[i] * vec[i];

	return sqrt(norm);
}

//数组的最大值
template<class DataType>
int SR<DataType>::Max(vector<DataType> vec){
	/*Max				求数组的最大值
	*vec				数组（N）
	*return：下标
	*author:ys
	*date:2016.05.05
	*/
	if (!vec.size()){
		cerr << "Max:error." << endl;
		throw "Max:error";
	}
	int index = 0;
	int i;
	int len = vec.size();
	for (i = 1; i < len; i++)
		if (vec[1] > vec[index])
			index = i;

	return index;
}

//加载字典 第0类字典对应DicFilePath[0]
template<class DataType>
void SR<DataType>::LoadDic(vector<string> DicFilePath){

	/*LoadDic				按0，1，2，...，n类依次加载字典，并记录每类字典的列数
	*DicFilePath			每类对应的字典
	*author:ys
	*date:2016.05.05
	*/
	this->classnum = DicFilePath.size();
    if (this->classnum == 0) {
		cerr << "dic path error." << endl;
		throw "dic path error";
    } else {
		cout << "the classnum is " << this->classnum << "." << endl;
	}

	ReadCsvFile<DataType> readcsv;
	int i;
	int num;
    for (i = 0; i < this->classnum; i++) {
		num = readcsv.ReadCsv(DicFilePath[i], this->dic);
        //cout << "the cols of class " << i << " are: " << num << endl;
		dicclassnum.push_back(num);
	}
}

//求解最小二乘问题
template<class DataType>
void SR<DataType>::solve(vector<vector<DataType>> phi, vector<DataType>& y, vector<DataType>& x)
{
	/*solve				求解最小二乘问题
	*phi				矩阵（列*行）
	*y					特征
	*x					求解系数
	*author:ys
	*date:2016.05.05
	*/
	int col = phi.size();
    if (col <= 0) {
		cerr << "solve: error." << endl;
		throw "solve: error";
	}
	int row = phi[0].size();
    if (row != y.size() || col != x.size()) {
		cerr << "solve: error." << endl;
		throw "solve: error";
	}
	MatrixXd A(row, col);
	VectorXd b(row);
	int i, j;
    for (i = 0; i < row; i++) {
		b(i) = y[i];
		for (j = 0; j < col; j++)
			A(i, j) = phi[j][i];
    }

	VectorXd result = A.colPivHouseholderQr().solve(b);
	for (i = 0; i < col; i++)
		x[i] = result(i);
}

//OMP算法
template<class DataType>
void SR<DataType>::OrthMatchPursuit(vector<DataType>& y, DataType min_residual, int sparsity, vector<DataType>& x, vector<int>& patch_indices)
{
	/*OrthMatchPursuit	正交匹配追踪算法（OMP）
	*dic				字典（列*行）
	*y					特征
	*min_residual		最小残差
	*sparsity			稀疏度
	*x					返回每个原子对应的系数
	*patch_indices		返回选出的原子序号
	*return：true：成功	| false：失败
	*author:ys
	*date:2016.05.05
	*/
	int fsize = y.size(); //特征维数
    if (!fsize) {
		cerr << "OrthMatchPursuit:error." << endl;
		throw "OrthMatchPursuit:error";
	}
	int dcol = dic.size();      //字典的列数
    if (!dcol) {
		cerr << "OrthMatchPursuit:error." << endl;
		throw "OrthMatchPursuit:error";
	}
	int drow = dic[0].size();   //字典的行数
    if (!drow || drow != fsize) {
		cerr << "OrthMatchPursuit:error." << endl;
		throw "OrthMatchPursuit:error";
	}

	vector<DataType> residual(y); //残差

	vector<vector<DataType>> phi; //保存已选出的原子向量

	x.clear();

	DataType max_coefficient;
	unsigned int patch_index;
	int i;

    while(true) {
		max_coefficient = 0;
        for (i = 0; i < dcol; i++) {
			DataType coefficient = (DataType)Dot(dic[i], residual);

            if (fabs(coefficient) > fabs(max_coefficient)) {
				max_coefficient = coefficient;
				patch_index = i;
			}
		}

		patch_indices.push_back(patch_index);

		phi.push_back(dic[patch_index]);

		x.push_back(0.0);

		solve(phi, y, x); //求解最小二乘问题

		residual = y - phi *x;
		if (x.size() >= sparsity)
			break;
	}

	residual.clear();//释放内存
	phi.clear();//释放内存
}



#endif
