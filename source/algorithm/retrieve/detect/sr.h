
#ifndef _SR_H_
#define _SR_H_

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include "Eigen/Dense"
#include "readcsv.h"
#include "basicoperator.h"

#ifdef _OPENMP

#if defined(__clang__)
#include <libiomp/omp.h>
#elif defined(__GNUG__) || defined(__GNUC__)
#include <omp.h>
#endif

#endif

using namespace std;
using namespace Eigen;

template<class DataType>
class SR {

public:

	//构造函数
	SR();
	SR(vector<string> DicFilePath);

	//加载字典 第0类字典对应DicFilePath[0]
	bool LoadDic(vector<string>& DicFilePath);

	//稀疏表示分类（一组特征）
	int SRClassify(vector<DataType>& y, DataType min_residual, int sparsity);

	//稀疏表示分类（多组特征）
	bool SRClassify(vector<vector<DataType>>& y, DataType min_residual, int sparsity, vector<int> &srres, vector<int> &srprob);
	//析构函数
	~SR();

private:

	vector<vector<DataType>> dic;		//存放字典 （vector<DataType>：列）
	vector<int> dicclassnum;			//每类字典的列数
	int	classnum;						//类的个数


private:
	//两个数组内积
	DataType Dot(vector<DataType>& vec1, vector<DataType>& vec2);

	//数组2范数
	DataType Norm(vector<DataType> vec);

	//数组的最大值
	int Max(vector<int>& vec);

	//求解最小二乘问题
	bool solve(vector<vector<DataType>>& phi, vector<DataType>& y, vector<DataType>& x);

	//OMP算法
	bool OrthMatchPursuit(vector<DataType>& y, DataType min_residual, int sparsity, vector<DataType>& x, vector<int>& patch_indices);



};

//构造函数
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

//稀疏表示分类（一组特征）
template<class DataType>
int SR<DataType>::SRClassify(vector<DataType>& y, DataType min_residual, int sparsity)
{
	/*SRClassify		稀疏表示识别
	*y					特征
	*min_residual		最小残差
	*sparsity			稀疏度
	*return：类别:（0, 1，2，3，...） -1：错误
	*author:ys
	*date:2016.05.05
	*/
	int fsize = y.size(); //特征维数
	if (!fsize){
		cerr << "SRClassify:error." << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return -1;
	}
	int dcol = dic.size();      //字典的列数
	if (!dcol){
		cerr << "SRClassify:error." << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return -1;
	}
	int drow = dic[0].size();   //字典的行数
	if (!drow || drow != fsize){
		cerr << "SRClassify:error." << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return -1;
	}
	int i, j;
	vector<int> patch_indices;	//稀疏系数的列号
	vector<DataType> coefficient;//稀疏系数

	if ( !OrthMatchPursuit(y, min_residual, sparsity, coefficient, patch_indices) )//OMP求稀疏系数
	{
		vector<int>().swap(patch_indices);//释放内存
		vector<DataType>().swap(coefficient);//释放内存
		//patch_indices.clear();//释放内存
		//coefficient.clear();//释放内存
		cerr << "SRClassify:OMP error." << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return -1;
	}

	vector<DataType> x(dcol, 0.0); //稀疏解
	for (i = 0; i < patch_indices.size(); ++i)
	{
		//cout << coefficient[i] << "  ";
		x[patch_indices[i]] = coefficient[i];
	}
	//cout << endl;

	vector<int>().swap(patch_indices);//释放内存
	vector<DataType>().swap(coefficient);//释放内存
	//patch_indices.clear();//释放内存
	//coefficient.clear();//释放内存

	int result = 0;//分类结果 属于字典对应的类

	int start = 0;//某一类开始位置
	DataType mindist = 100000000;
	for (i = 0; i < classnum; ++i)
	{
		vector<DataType> tmp(fsize, 0.0);
		for (j = start; j < start + dicclassnum[i]; ++j)
		{
			if (x[j] != 0.0)
			{
				tmp = tmp + dic[j] * x[j];		
			}

		}
		DataType dist = Norm(y - tmp);

		vector<DataType>().swap(tmp);//释放内存
		//tmp.clear();//释放内存

		if (mindist > dist)
		{
			mindist = dist;
			result = i;//
		}

		start += dicclassnum[i];
	}
	vector<DataType>().swap(x);//释放内存
	//x.clear();//释放内存

	return result; //result（0，1，2，3，...）

}

//稀疏表示分类（多组特征）
template<class DataType>
bool SR<DataType>::SRClassify(vector<vector<DataType>>& y, DataType min_residual, int sparsity, vector<int> &srres, vector<int> &srprob){
	/*SRClassify		稀疏表示识别
	*y					特征
	*min_residual		最小残差
	*sparsity			稀疏度
	*srres              排序后的目标索引
	*srprob             与srres对应目标的概率
	*return				true|false
	*author:ys
	*date:2016.06.02
	*/
	int i,j;
	int size = y.size();
	vector<int> result(this->classnum, 0);

	vector<int> results(size,0);
	
#pragma omp parallel for
	for (i = 0; i < size; ++i) {
		results[i] = SRClassify(y[i], min_residual, sparsity);
	}

	for (i = 0; i < size; ++i)
	{
		if (results[i] < 0)
		{
			cerr << "SRClassify:error." << endl;
			cerr << "file:" << __FILE__ << endl;
			cerr << "line: " << __LINE__ << endl;
			cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
			return false;
		}
		result[results[i]]++;
	}

	srres.resize(this->classnum);
	for(i = 0; i < this->classnum; ++i){
		srres[i] = i;
//		cout << result[i] << endl;
	}

	for(i = this->classnum-1; i >= 0; --i){ //将识别后为0的类别删除
		if(result[i] == 0){
			result.erase(result.begin() + i);
			srres.erase(srres.begin() + i);
		}
	}

	int resnum = result.size(); //识别结果个数
	srprob.resize(resnum);
	//对目标进行排序(递减)
	for(i = 0; i < resnum - 1; ++i){
		int index = i;
		for(j = i + 1; j < resnum; ++j){
			if(result[index] < result[j]){
				index = j;
			}
		}
		int tmp = result[index];
		result[index] = result[i];
		result[i] = tmp;

		tmp = srres[index];
		srres[index] = srres[i];
		srres[i] = tmp;
		//保留两位
		srprob[i] = 0.5+(100.0*result[i])/size;

	}
	

	/*
	int maxindex;
	try
	{
		maxindex = Max(result);
	}
	catch (runtime_error &e)
	{
		vector<int>().swap(result);//释放内存
		//result.clear();//释放内存

		cerr << "SRClassify:error." << endl;
		cerr << e.what() << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return false;

	}
	vector<int>().swap(result);//释放内存
	//result.clear();//释放内存
	*/
	return true;
}

//析构函数
template<class DataType>
SR<DataType>::~SR(){
	//释放字典内存
	int i;
	int size = dic.size();
	for (i = 0; i < size; ++i) 
	{
		vector<DataType>().swap(dic[i]);
		//dic[i].clear();
	}
	vector<vector<DataType>>().swap(dic);
	//dic.clear();
	vector<int>().swap(dicclassnum);
}

//两个数组内积
template<class DataType>
DataType SR<DataType>::Dot(vector<DataType>& vec1, vector<DataType>& vec2)
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
		throw runtime_error("Dot:error");
	}
	DataType sum = 0;
	int i;
	int len = vec1.size();
	for (i = 0; i < len; ++i)
	{
		sum += vec1[i] * vec2[i];
	}

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
		throw runtime_error("Norm:error");
	}
	DataType norm = 0;
	int i;
	int len = vec.size();
	for (i = 0; i < len; ++i)
	{
		norm += vec[i] * vec[i];
	}

	return sqrt(norm);
}

//数组的最大值
template<class DataType>
int SR<DataType>::Max(vector<int>& vec){
	/*Max				求数组的最大值
	*vec				数组（N）
	*return：下标
	*author:ys
	*date:2016.05.05
	*/
	if (!vec.size()){
		cerr << "Max:error." << endl;
		throw runtime_error("Max:error");
	}
	int index = 0;
	int i;
	int len = vec.size();
	for (i = 1; i < len; ++i)
	{
		if (vec[1] > vec[index])
		{
			index = i;
		}
	}

	return index;
}

//加载字典 第0类字典对应DicFilePath[0]
template<class DataType>
bool SR<DataType>::LoadDic(vector<string>& DicFilePath){

	/*LoadDic				按0，1，2，...，n类依次加载字典，并记录每类字典的列数
	*DicFilePath			每类对应的字典
	*return					true|false
	*author:ys
	*date:2016.05.05
	*/
	this->classnum = DicFilePath.size();
	if (this->classnum == 0)
	{
		cerr << "LoadDic:Dic Path error." << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return false;
	}
	else
	{
		cout << "the classnum is " << this->classnum << "." << endl;
	}

	ReadCsvFile<DataType> readcsv;
	int i;
	int num;
	for (i = 0; i < this->classnum; ++i)
	{
		cout << "loading " << DicFilePath[i] << ". ";
		if (!readcsv.ReadCsv(DicFilePath[i], this->dic, num))
		{
			cerr << "LoadDic:ReadCsv error." << endl;
			cerr << "file:" << __FILE__ << endl;
			cerr << "line: " << __LINE__ << endl;
			cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
			return false;
		}	
		cout << "the cols of class " << i << " are: " << num << endl;
		dicclassnum.push_back(num);
	}

	return true;
}

//求解最小二乘问题
template<class DataType>
bool SR<DataType>::solve(vector<vector<DataType>>& phi, vector<DataType>& y, vector<DataType>& x)
{
	/*solve				求解最小二乘问题
	*phi				矩阵（列*行）
	*y					特征
	*x					求解系数
	*return				true|false
	*author:ys
	*date:2016.05.05
	*/
	int col = phi.size();
	if (col <= 0)
	{
		cerr << "solve: error." << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return false;
	}
	int row = phi[0].size();
	if (row != y.size() || col != x.size())
	{
		cerr << "solve: error." << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return false;
	}
	MatrixXd A(row, col);
	VectorXd b(row);
	int i, j;
	for (i = 0; i < row; ++i)
	{
		b(i) = y[i];
		for (j = 0; j < col; ++j)
			A(i, j) = phi[j][i];
	}
	//jacobiSvd 方式:Slow (but fast for small matrices)
	//cout << "The least-squares solution is:\n"<< A.jacobiSvd(ComputeThinU | ComputeThinV).solve(b) << std::endl;
	VectorXd result = A.jacobiSvd(ComputeThinU | ComputeThinV).solve(b);
	//colPivHouseholderQr方法:fast
	//std::cout << "The least-squares solution is:\n"<< A.colPivHouseholderQr().solve(b) << std::endl;
	//VectorXd result = A.colPivHouseholderQr().solve(b);
	for (i = 0; i < col; ++i)
	{
		x[i] = result(i);
	}

	return true;
}

//OMP算法
template<class DataType>
bool SR<DataType>::OrthMatchPursuit(vector<DataType>& y, DataType min_residual, int sparsity, vector<DataType>& x, vector<int>& patch_indices)
{
	/*OrthMatchPursuit	正交匹配追踪算法（OMP）
	*dic				字典（列*行）
	*y					特征
	*min_residual		最小残差
	*sparsity			稀疏度
	*x					返回每个原子对应的系数
	*patch_indices		返回选出的原子序号
	*return					true|false
	*author:ys
	*date:2016.05.05
	*/
	int fsize = y.size(); //特征维数
	if (!fsize)
	{
		cerr << "OrthMatchPursuit:error." << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return false;
	}
	int dcol = dic.size();      //字典的列数
	if (!dcol)
	{
		cerr << "OrthMatchPursuit:error." << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return false;
	}
	int drow = dic[0].size();   //字典的行数
	if (!drow || drow != fsize)
	{
		cerr << "OrthMatchPursuit:error." << endl;
		cerr << "file:" << __FILE__ << endl;
		cerr << "line: " << __LINE__ << endl;
		cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
		return false;
	}

	vector<DataType> residual(y); //残差

	vector<vector<DataType>> phi; //保存已选出的原子向量

	x.clear();

	DataType max_coefficient;
	unsigned int patch_index;
	int i;
	vector<DataType> coefficient(dcol,0);
	
	for (;;)
	{
		max_coefficient = 0;
		/*
		for (i = 0; i < dcol; ++i)
		{
			DataType coefficient;

			coefficient = (DataType)Dot(dic[i], residual);

			if (fabs(coefficient) > fabs(max_coefficient))
			{
				max_coefficient = coefficient;
				patch_index = i;
			}
		}
		*/
#pragma omp parallel for shared(coefficient)
		for (i = 0; i < dcol; ++i)
		{
			coefficient[i] = (DataType)Dot(dic[i], residual);
		}

		for (i = 0; i < dcol; ++i)
		{
			if (fabs(coefficient[i]) > fabs(max_coefficient))
			{
				max_coefficient = coefficient[i];
				patch_index = i;
			}
		}		

		patch_indices.push_back(patch_index);

		phi.push_back(dic[patch_index]);

		x.push_back(0.0);

		if ( !solve(phi, y, x) )//求解最小二乘问题
		{
			cerr << "OrthMatchPursuit:error." << endl;
			cerr << "file:" << __FILE__ << endl;
			cerr << "line: " << __LINE__ << endl;
			cerr << "time: " << __DATE__ << " " << __TIME__ << endl;
			vector<DataType>().swap(residual);//释放内存
			//residual.clear();//释放内存
			for (i = 0; i < phi.size(); ++i) vector<DataType>().swap(phi[i]);
			vector<vector<DataType>>().swap(phi);
			//phi.clear();//释放内存
			return false;
		}

		residual = y - phi *x;
		DataType res_norm = (DataType)Norm(residual);

		//if (x.size() >= sparsity || res_norm <= min_residual) //根据稀疏个数和残差作为终止条件
		if (x.size() >= sparsity)
			break;
	}

	vector<DataType>().swap(residual);//释放内存
	//residual.clear();//释放内存
	for (i = 0; i < phi.size(); ++i) vector<DataType>().swap(phi[i]);
	vector<vector<DataType>>().swap(phi);
	//phi.clear();//释放内存

	return true;
}



#endif
