#include "extract_feature.h"

#include "gdalio.h"
#include "demo_lib_sift.h"

#include "library.h"
#include "frot.h"
#include "fproj.h"
#include "compute_asift_keypoints.h"
#include "compute_asift_matches.h"
using namespace std;

bool AsiftFeature(const string Output_FileName,string Input_FilePath,vector<vector<float>> &features){

    int i,j,m,n;
    float *GrayImg=NULL;  //灰度图像
    float *Img = NULL;    //图像

    int height ;		  //图像高度
    int width ;			  //图像宽度
    int bandcount;        //图像波段数

    ofstream fout(Output_FileName.c_str());
    if (!fout)
    {
        cerr << "AsiftFeature:Output File Name Error!" << endl;
        cerr<<"file: "<<__FILE__<<endl;
        cerr<<"line: "<<__LINE__<<endl;
        cerr<<"time: "<<__DATE__<<" "<<__TIME__<<endl;
        return false;
    }

    cout<<"Extract Features From£∫"<<Input_FilePath<<endl;
    cout<<"Processing. "<<endl;

    /*读图像*/
    ReadImageToBuff(Input_FilePath.c_str(), &Img, height,width,bandcount);

    cout<<"image heigh : "<<height<<endl;
    cout<<"image width : "<<width<<endl;
    cout<<"image band  : "<<bandcount<<endl;

    GrayImg = NULL;
    GrayImg=new(std::nothrow) float[height*width];
    if (NULL == GrayImg)
    {
        cerr<<"AsiftFeature:Memory Error."<<endl;
        cerr<<"file£∫"<<__FILE__<<endl;
        cerr<<"line£∫"<<__LINE__<<endl;
        cerr<<"time£∫"<<__DATE__<<" "<<__TIME__<<endl;
        return false;
    }
    if (bandcount==3)
    {
        //GrayImg=Gdal_rgb2gray(Img,height,width,bandcount);
        for(i=0;i<height*width;i++)
        {
            GrayImg[i] = 0.212671 * Img[i] + 0.715160 * Img[i+height*width] + 0.072169 * Img[i+height*width*2];
        }
    }
    else if(bandcount==1)
    {


        for(i=0;i<height*width;i++)
        {
            GrayImg[i]=Img[i];
        }
    }
    else
    {
        cerr<<"AsiftFeature:Image Band Error."<<endl;
        cerr<<"file£∫"<<__FILE__<<endl;
        cerr<<"line£∫"<<__LINE__<<endl;
        cerr<<"time£∫"<<__DATE__<<" "<<__TIME__<<endl;
        delete[] Img;Img = NULL;
        delete[] GrayImg;GrayImg = NULL;
        return false;
    }

    //释放内存
    delete[] Img;Img = NULL;
    /*图像归一化*/
    //Normalize(GrayImg, width, height);

    vector<float> ipixels(GrayImg, GrayImg + width * height);
    //vector<float> ipixels;

    //ipixels.resize(width * height);
    //for (i=0;i<width * height;i++){
    //	ipixels[i] = GrayImg[i];

    //}

    float wS;
    float hS;
    int  flag_resize=0;   // 是否resize
    //判断是否resize
    if (width>300)
    {
        wS = 300;
        hS = floor(height*wS/width); //
        flag_resize=1;//resize
    }
    //释放内存
    delete [] GrayImg;GrayImg=NULL;

    float zoom1=0;
    int wS1=0, hS1=0;
    vector<float> ipixels1_zoom;
    if (flag_resize != 0)
    {
        cout << "WARNING: The input images are resized to " << wS << "x" << hS << " for ASIFT. " << endl
        << "         But the results will be normalized to the original image size." << endl << endl;

        float InitSigma_aa = 1.6;

        float fproj_p, fproj_bg;
        char fproj_i;
        float *fproj_x4, *fproj_y4;
        int fproj_o;

        fproj_o = 3;
        fproj_p = 0;
        fproj_i = 0;
        fproj_bg = 0;
        fproj_x4 = 0;
        fproj_y4 = 0;

        float areaS = wS * hS;

        // Resize image 1
        float area1 = width * height;
        zoom1 = sqrt(area1/areaS);

        wS1 = (int) (width / zoom1);
        hS1 = (int) (height / zoom1);

        int fproj_sx = wS1;
        int fproj_sy = hS1;

        float fproj_x1 = 0;
        float fproj_y1 = 0;
        float fproj_x2 = wS1;
        float fproj_y2 = 0;
        float fproj_x3 = 0;
        float fproj_y3 = hS1;

        /* Anti-aliasing filtering along vertical direction */
        if ( zoom1 > 1 )		//
        {
            float sigma_aa = InitSigma_aa * zoom1 / 2;
            GaussianBlur1D(ipixels,width,height,sigma_aa,1);
            GaussianBlur1D(ipixels,width,height,sigma_aa,0);
        }

        // simulate a tilt: subsample the image along the vertical axis by a factor of t.



        ipixels1_zoom.resize(wS1*hS1);
        fproj (ipixels, ipixels1_zoom, width, height, &fproj_sx, &fproj_sy, &fproj_bg, &fproj_o, &fproj_p,
               &fproj_i , fproj_x1 , fproj_y1 , fproj_x2 , fproj_y2 , fproj_x3 , fproj_y3, fproj_x4, fproj_y4);



    }
    else
    {

        ipixels1_zoom.resize(width*height);
        ipixels1_zoom = ipixels;
        wS1 = width;
        hS1 = height;
        zoom1 = 1;

    }
    //释放内存
    vector<float>().swap(ipixels);
    //pixels.clear();

    /*提取特征*/
    // Compute ASIFT keypoints
    // number N of tilts to simulate t = 1, \sqrt{2}, (\sqrt{2})^2, ..., {\sqrt{2}}^(N-1)

    int num_of_tilts = 7;
    //	int num_of_tilts1 = 1;
    //	int num_of_tilts2 = 1;
    int verb = 0;
    // Define the SIFT parameters
    siftPar siftparameters;
    default_sift_parameters(siftparameters);

    vector< vector< keypointslist > > keys;

    int num_keys=0;

    num_keys = compute_asift_keypoints(ipixels1_zoom, wS1, hS1, num_of_tilts, verb, keys, siftparameters);

    //释放内存
    vector<float>().swap(ipixels1_zoom);
    //pixels1_zoom.clear();

    /*将特征保存到文件中*/

    int tt,rr,ii;

    vector<float> feature(VecLength,0);
    for (tt = 0; tt < (int) keys.size(); tt++)
    {
        for (rr = 0; rr < (int) keys[tt].size(); rr++)
        {
            keypointslist::iterator ptr = keys[tt][rr].begin();

            for(i=0; i < (int) keys[tt][rr].size(); i++, ptr++)
            {
                for (ii = 0; ii < (int) VecLength-1 ; ii++)
                {
                    fout << ptr->vec[ii] << ',';
                    feature[ii]=ptr->vec[ii];
                }
                fout << ptr->vec[VecLength-1];
                fout << endl;
                features.push_back(feature);
            }

        }
    }
    //fout<<endl;

    for (tt = 0; tt < (int) keys.size(); tt++)
    {
        for (rr = 0; rr < (int) keys[tt].size(); rr++)
        {
            keypointslist().swap(keys[tt][rr]);

        }
        vector< keypointslist >().swap(keys[tt]);
    }
    vector< vector< keypointslist > >().swap(keys);

    cout<<"extract done."<<endl;
    fout.close();

    return true;
}
