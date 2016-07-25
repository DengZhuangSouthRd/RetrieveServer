#include "imagethumb.h"

#define ThumbWidth 200

void BufArrayUint16ToByte(ushort *&arrayIn, int dataLength, ushort lower, ushort upper)
{
	// 
	float a = (float)(255)/(float)(upper - lower);
	float b = (float)(0) - a * (float)lower;

	
	for (int i = 0; i < dataLength; i++)
	{
		*(arrayIn + i) = (ushort)((float)(*(arrayIn + i) < lower ?
lower : *(arrayIn + i) > upper ? upper : *(arrayIn + i)) * a + b) ;  
	}

} 

void GetHistogramUInt16(ushort *&pDataBuffer,int size)
{	
	// 
	vector<long> histogram(65536); 

	for (int j = 0; j < size; j++)
	{
		histogram[*(pDataBuffer +j)]++;    // 统计直方图
	}
	ushort lower=(ushort)GetHistogramCutLower(histogram, size);
	ushort upper=(ushort)GetHistogramCutUpper(histogram, size);
	BufArrayUint16ToByte(pDataBuffer,size,lower,upper);

} 

int GetHistogramCutLower(vector<long> histogram, long total)
{
	const double D_THRESHOLD = 0.02;
	long cutNum = (long) ceil((double)total * D_THRESHOLD);

	long currentNum = 0;
	for (int i = 0; i < histogram.size(); i++)
	{
		currentNum += histogram[i];
		if (currentNum >= cutNum)
		{
			return i;
		}
	}
	return -1;
} 

int GetHistogramCutUpper(vector<long> histogram, long total)
{
	const double D_THRESHOLD = 0.02;
	long cutNum = (long) ceil((double)total * D_THRESHOLD);

	long currentNum = 0;
	for (int i = histogram.size() - 1; i >= 0; i--)
	{
		currentNum += histogram[i];
		if (currentNum >= cutNum)
		{
			return i;
		}
	}
	return -1;
} 

//生成单波段灰度缩略图，填充成正方形
bool CreateSquareThumb( const string input , const string output , int bandID)
{
	/*CreateSquareThumb		生成正方形缩略图
	*input					原图像路径
	*bandID					选择波段（默认为第一波段）
	*author	：YS
	*date：2016.05.17  
	*/
	//int comma=input.find_last_of(".");
	//string output=input.substr(0,comma)+"_thumb.jpg";//生成缩略图存储路径
	//if (_access(output.c_str(),0) != -1)//判断文件是否存在，存在不再生成，返回true
	//{
	//	cout<<output<<" exists."<<endl;
	//	return true;
	//}

	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");   //设置支持中文路径
	GDALDataset *pDataset = (GDALDataset *) GDALOpen( input.c_str(), GA_ReadOnly );
	if( NULL == pDataset )
		return false;

	int nSamples = ((GDALDataset *)pDataset)->GetRasterXSize(); //列数
	int nLines = ((GDALDataset *)pDataset)->GetRasterYSize();	//行数
	int nBands = ((GDALDataset *)pDataset)->GetRasterCount();	//波段数

	if(bandID > nBands)	//如果选择的波段大于图像波段数  报错
	{
		cerr<<"bandID error."<<endl;
		GDALClose( (GDALDatasetH) pDataset );
		return false;
	}

	int i , j ;
	ushort *pBuffer = new(std::nothrow) ushort[ nSamples ];  //存储图像每行数据
	ushort *ImgData = new(std::nothrow) ushort[ nSamples*nLines ];  //存储图像每行数据
	if ( NULL == pBuffer )
	{
		cerr<<"memory error."<<endl;
		GDALClose( (GDALDatasetH) pDataset );
		return false;
	}	
	if ( NULL == ImgData)
	{
		cerr<<"memory error."<<endl;
		delete[] pBuffer;
		GDALClose( (GDALDatasetH) pDataset );
		return false;
	}
	//读取图像
	GDALRasterBand *pBandRead = ((GDALDataset *)pDataset)->GetRasterBand( bandID ); //获取指定波段的数据
	//int overview[1]={4};
	//pBandRead->BuildOverviews("NEAREST",1,overview,GDALDummyProgress,NULL);
	//pBandRead->GetOverview(0)->GetXSize();
	//pBandRead->GetOverview(0)->GetYSize();
	if (pBandRead->RasterIO(GF_Read,0,0,nSamples,nLines,ImgData,nSamples,nLines,GDT_UInt16, NULL, 0, 0)== CE_Failure)
	{
		cerr<<"read data error."<<endl;
		delete[] pBuffer;
		delete[] ImgData;
		GDALClose( (GDALDatasetH) pDataset );
		return false;
	}			
	//拉伸处理 保存到原数据集
	GetHistogramUInt16(ImgData,nSamples*nLines);

	//
	GDALDriver * pDriver = (GDALDriver*)GDALGetDriverByName( "MEM" );
	if( NULL == pDriver )
	{
		cerr<<"create MEM driver error."<<endl;
		delete[] pBuffer;
		delete[] ImgData;
		GDALClose( (GDALDatasetH) pDataset );
		return false;
	}

	int stepSize = max(nSamples,nLines)>=ThumbWidth? max(nSamples,nLines)/ThumbWidth:max(nSamples,nLines);	//下采样步长
	int outSample = int(nSamples / stepSize) + ((nSamples % stepSize) == 0 ? 0:1);//采样后列数
	int outLine = int(nLines / stepSize) + ((nLines % stepSize) == 0 ? 0:1 );	  //采样后行数	
	int out = max(outSample,outLine); //输出正方形

	GDALDataset* pDSOut = pDriver->Create( "tmpFile",out,out,1,GDT_Byte,NULL);	
	if( NULL == pDSOut )		
	{
		cerr<<"create tmpfile error."<<endl;
		delete[] pBuffer;
		delete[] ImgData;
		GDALClose( (GDALDatasetH) pDataset );
		return false;
	}

	uchar* pBufferOut = new(std::nothrow) uchar[ out ];//输出缓存
	if ( NULL == pBufferOut )
	{
		cerr<<"memory error."<<endl;
		delete[] pBuffer;
		delete[] ImgData;
		GDALClose( (GDALDatasetH) pDataset );
		return false;
	}
	for (i=0;i<out;i++)	//	初值
		pBufferOut[i] = 255;


	GDALRasterBand *pBandWrite = ((GDALDataset *)pDSOut)->GetRasterBand( 1 );
	if( NULL == pBandWrite )		
	{
		cerr<<"get raster band error."<<endl;
		delete[] pBuffer;
		delete[] ImgData;
		delete[] pBufferOut;
		GDALClose( (GDALDatasetH) pDataset );
		return false;
	}
	int offset;
	int offsetY = 0;
	//写入图像
	for (i=0;i<(out-outLine)/2;i++)	//	加边
	{
		if( pBandWrite->WriteBlock( 0 , offsetY , pBufferOut ) != CE_None ) //数据写入文件
		{
			cerr<<"write image error."<<endl;
			delete[] pBuffer;
			delete[] ImgData;
			delete[] pBufferOut;
			GDALClose( (GDALDatasetH) pDataset );
			GDALClose( (GDALDatasetH) pDSOut );
			return false;
		}
		offsetY++;
	}
	for ( i = 0 ; i < nLines ; i+= stepSize )
	{
		//按行读
		for (j = 0 ; j < nSamples ; j++)
		{
			pBuffer[j] = ImgData[i*nSamples+j];
		}
		offset = (out-outSample)/2;
		for( j = 0 ; j < nSamples ; j+= stepSize ) //下采样并灰度拉伸到0~255
		{			
			pBufferOut[ offset ] = (uchar)( pBuffer[ j ]);
			offset++;			
		}
		if( pBandWrite->WriteBlock( 0 , offsetY , pBufferOut ) != CE_None ) //数据写入文件
		{
			cerr<<"write image error."<<endl;
			delete[] pBuffer;
			delete[] ImgData;
			delete[] pBufferOut;
			GDALClose( (GDALDatasetH) pDataset );
			GDALClose( (GDALDatasetH) pDSOut );
			return false;
		}
		offsetY++;
	}



	for (i=0;i<out;i++)	//	加边
		pBufferOut[i] = 255;
	for (i=offsetY;i<out;i++)	//
	{
		if( pBandWrite->WriteBlock( 0 , offsetY , pBufferOut ) != CE_None ) //数据写入文件
		{
			cerr<<"write image error."<<endl;
			delete[] pBuffer;
			delete[] ImgData;
			delete[] pBufferOut;
			GDALClose( (GDALDatasetH) pDataset );
			GDALClose( (GDALDatasetH) pDSOut );
			return false;
		}
		offsetY++;
	}
	delete[] pBuffer;
	delete[] ImgData;
	delete[] pBufferOut;

	GDALDriver * poDriver = GetGDALDriverManager()->GetDriverByName("JPEG");
	if( NULL == poDriver )
	{
		cerr<<"get driver error."<<endl;
		GDALClose( (GDALDatasetH) pDataset );
		GDALClose( (GDALDatasetH) pDSOut );
		return false;	
	}
	GDALDataset * poDataset=poDriver->CreateCopy(output.c_str(),pDSOut,false,NULL,NULL,NULL);
	if( NULL == poDataset )
	{
		cerr<<"driver copy error."<<endl;
		GDALClose( (GDALDatasetH) pDataset );
		GDALClose( (GDALDatasetH) pDSOut );
		return false;	
	}
	//不保存".aux.xml" 文件
	GDALPamDataset *pamDs=dynamic_cast<GDALPamDataset*>(poDataset);
	if(NULL == pamDs )
	{
		int pamFlags=pamDs->GetPamFlags();
		pamFlags|=GPF_NOSAVE;
		pamDs->SetPamFlags(pamFlags);	
	}

	if( NULL == pDSOut )
		GDALClose( (GDALDatasetH) pDSOut );
	pDriver->Delete( "tmpFile" );
	if( NULL == pDataset )
		GDALClose( (GDALDatasetH) pDataset );
	if( NULL == poDataset )
		GDALClose( (GDALDatasetH) poDataset );
	return true;
}

//生成RGB彩色缩略图，填充成正方形
bool CreateSquareThumb( const string input ,const string output, int redBand,int greenBand, int blueBand)
{
	/*CreateThumb		生成正方形缩略图
	*input				原图像路径
	*redBand			选择波段作为R波段
	*greenBand			选择波段作为G波段
	*blueBand			选择波段作为B波段
	*author	：YS
	*date：2016.05.17  
	*/
	//int comma=input.find_last_of(".");
	//stringstream ss;
	//ss<<redBand<<"_"<<greenBand<<"_"<<blueBand<<".jpg";
	//string output = input.substr(0,comma)+"_thumb_"+ss.str();//生成缩略图存储路径
	//ss.clear();
	//if (_access(output.c_str(),0) != -1)//判断文件是否存在，存在不再生成，返回true
	//{
	//	cout<<output<<" exists."<<endl;
	//	return true;
	//}

	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");   

	GDALDataset *pDataset = (GDALDataset *) GDALOpen( input.c_str(), GA_ReadOnly );

	if( NULL == pDataset )
		return false;

	int nSamples = ((GDALDataset *)pDataset)->GetRasterXSize();//列数
	int nLines = ((GDALDataset *)pDataset)->GetRasterYSize();  //行数
	int nBands = ((GDALDataset *)pDataset)->GetRasterCount();  //波段数
	int rgbBand[] = {redBand,greenBand,blueBand};
	int i,j,k;
		
	if(redBand > nBands || greenBand > nBands || blueBand > nBands)
	{
		cerr<<"bandID error."<<endl;
		GDALClose( (GDALDatasetH) pDataset );
		return false;
	}

	ushort *pBuffer = new(std::nothrow) ushort[ nSamples ];  //存储图像每行数据
	ushort *ImgData = new(std::nothrow) ushort[ nSamples*nLines ];  //存储图像每行数据
	if ( NULL == pBuffer )
	{
		cerr<<"memory error."<<endl;
		GDALClose( (GDALDatasetH) pDataset );
		return false;
	}
	if ( NULL == ImgData)
	{
		cerr<<"memory error."<<endl;
		delete[] pBuffer;
		GDALClose( (GDALDatasetH) pDataset );
		return false;
	}
	//打开写入图像
	GDALDriver * pDriver = (GDALDriver*)GDALGetDriverByName( "MEM" );
	if( NULL == pDriver )
	{
		cerr<<"create MEM driver error."<<endl;
		delete[] pBuffer;
		delete[] ImgData;
		GDALClose( (GDALDatasetH) pDataset );
		return false;
	}

	int stepSize = max(nSamples,nLines)>=ThumbWidth? max(nSamples,nLines)/ThumbWidth:max(nSamples,nLines);	//下采样步长
	int outSample = int(nSamples / stepSize) + ((nSamples % stepSize) == 0 ? 0:1);//采样后列数
	int outLine = int(nLines / stepSize) + ((nLines % stepSize) == 0 ? 0:1 );	  //采样后行数
	int out = max(outSample,outLine); //输出正方形

	GDALDataset* pDSOut = pDriver->Create( "tmpFile",out,out,3,GDT_Byte,NULL);	
	if( NULL == pDSOut )		
	{
		cerr<<"create tmpfile error."<<endl;
		delete[] pBuffer;
		delete[] ImgData;
		GDALClose( (GDALDatasetH) pDataset );
		return false;
	}

	//写入图像
	unsigned char* pBufferOut = new(std::nothrow) unsigned char[ out ];//输出缓存
	if (NULL == pBufferOut )
	{
		cerr<<"memory error."<<endl;
		delete[] pBuffer;
		delete[] ImgData;
		GDALClose( (GDALDatasetH) pDataset );
		return false;
	}
	for (i=0;i<out;i++)	//	初值
		pBufferOut[i] = 255;

	GDALRasterBand *pBandWrite=NULL;
	GDALRasterBand *pBandRead=NULL;

	for( k=0 ; k<3 ; k++ )
	{

		pBandWrite = ((GDALDataset *)pDSOut)->GetRasterBand( k +1 );
		pBandRead = ((GDALDataset *)pDataset)->GetRasterBand( rgbBand[k] );
		if( NULL == pBandWrite || pBandRead == NULL )		
		{
			cerr<<"get raster band error."<<endl;
			delete[] pBuffer;
			delete[] ImgData;
			delete[] pBufferOut;
			GDALClose( (GDALDatasetH) pDataset );
			return false;
		}
		//读取某个波段
		if (pBandRead->RasterIO(GF_Read,0,0,nSamples,nLines,ImgData,nSamples,nLines,GDT_UInt16, NULL, 0, 0)== CE_Failure)
		{
			cerr<<"read data error."<<endl;
			delete[] pBuffer;
			delete[] ImgData;
			delete[] pBufferOut;
			GDALClose( (GDALDatasetH) pDataset );
			return false;
		}			
		//拉伸处理 保存到原数据集
		GetHistogramUInt16(ImgData,nSamples*nLines);
		//
		int offset;
		int offsetY = 0;
		for (i=0;i<(out-outLine)/2;i++)	//	加边
		{
			if( pBandWrite->WriteBlock( 0 , offsetY , pBufferOut ) != CE_None ) //数据写入文件
			{
				cerr<<"write image error."<<endl;
				delete[] pBuffer;
				delete[] ImgData;
				delete[] pBufferOut;
				GDALClose( (GDALDatasetH) pDataset );
				GDALClose( (GDALDatasetH) pDSOut );
				return false;
			}
			offsetY++;
		}
		for ( i = 0 ; i < nLines ; i+= stepSize )
		{
			//按行读
			for (j = 0 ; j < nSamples ; j++)
			{
				pBuffer[j] = ImgData[i*nSamples+j];
			}

			offset = (out-outSample)/2;
			for( j = 0 ; j < nSamples ; j+= stepSize )//下采样并灰度拉伸到0~255
			{			
				pBufferOut[ offset ] = (uchar)(pBuffer[ j ]);
				offset++;			
			}
			if( pBandWrite->WriteBlock( 0 , offsetY , pBufferOut )!= CE_None )
			{
				cerr<<"write image error."<<endl;
				delete[] pBuffer;
				delete[] ImgData;
				delete[] pBufferOut;
				GDALClose( (GDALDatasetH) pDataset );
				GDALClose( (GDALDatasetH) pDSOut );
				return false;
			}
			offsetY++;
		}
		for (i=0;i<out;i++)	//	加边
			pBufferOut[i] = 255;
		for (i=offsetY;i<out;i++)	//
		{
			if( pBandWrite->WriteBlock( 0 , offsetY , pBufferOut ) != CE_None ) //数据写入文件
			{
				cerr<<"write image error."<<endl;
				delete[] pBuffer;
				delete[] ImgData;
				delete[] pBufferOut;
				GDALClose( (GDALDatasetH) pDataset );
				GDALClose( (GDALDatasetH) pDSOut );
				return false;
			}
			offsetY++;
		}
	}

	delete[] pBuffer;
	delete[] ImgData;
	delete[] pBufferOut;

	GDALDriver * poDriver = GetGDALDriverManager()->GetDriverByName("JPEG");
	if( NULL == poDriver )
	{
		cerr<<"get driver error."<<endl;
		GDALClose( (GDALDatasetH) pDataset );
		GDALClose( (GDALDatasetH) pDSOut );
		return false;	
	}
	GDALDataset * poDataset=poDriver->CreateCopy(output.c_str(),pDSOut,false,NULL,NULL,NULL);
	if( NULL == poDataset )
	{
		cerr<<"driver copy error."<<endl;
		GDALClose( (GDALDatasetH) pDataset );
		GDALClose( (GDALDatasetH) pDSOut );
		return false;	
	}
	//不保存".aux.xml" 文件
	GDALPamDataset *pamDs=dynamic_cast<GDALPamDataset*>(poDataset);
	if(NULL == pamDs )
	{
		int pamFlags=pamDs->GetPamFlags();
		pamFlags|=GPF_NOSAVE;
		pamDs->SetPamFlags(pamFlags);	
	}

	if( NULL == pDSOut )
		GDALClose( (GDALDatasetH) pDSOut );
	pDriver->Delete( "tmpFile" );
	if( NULL == pDataset )
		GDALClose( (GDALDatasetH) pDataset );
	if( NULL == poDataset )
		GDALClose( (GDALDatasetH) poDataset );
	return true;
}
