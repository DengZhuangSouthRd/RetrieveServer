#include "extract_feature.h"

#include "gdalio.h"
#include "demo_lib_sift.h"

#include "library.h"
#include "frot.h"
#include "fproj.h"
#include "compute_asift_keypoints.h"
#include "compute_asift_matches.h"

# define IM_X 160
# define IM_Y 120

using namespace std;

bool ASIFT_Ext_Features_Gdal(const string Output_FileName, string Input_FilePath, vector<vector<double> > &feature_vec) {
    int i;
    float *GrayImg=NULL;  //存储灰度图像像素值
    float *Img = NULL;    //存储图像像素值

    int height, width, bandcount;
    float wS = IM_X, hS = IM_Y;
    int  flag_resize=0;   //是否resize

    int status = ReadImageToBuff(Input_FilePath.c_str(), &Img, height,width,bandcount);
    if(status != 0) {
        throw runtime_error("ReadImageToBuff failed !");
    }

    GrayImg = new(std::nothrow) float[height*width];
    if (NULL == GrayImg) {
        cerr<<"Memory Error."<<endl;
        if(Img != NULL) {
            delete[] Img;
            Img=NULL;
        }
        throw runtime_error("ImageRetrieve Memort Error !");
    }

    if (bandcount==3) {
        for(i=0;i<height*width;i++) {
            GrayImg[i] = 0.212671 * Img[i] + 0.715160 * Img[i+height*width] + 0.072169 * Img[i+height*width*2];
        }
    } else if(bandcount==1) {
        for(i=0;i<height*width;i++) {
            GrayImg[i]=Img[i];
        }
    } else {
        cerr<<"Image Band Error."<<endl;
        delete[] GrayImg;
        GrayImg = NULL;
        if(Img != NULL) {
            delete[] Img;
            Img=NULL;
        }
        throw runtime_error("ImageRetrieve Image Band Error !");
    }

    //释放内存
    if(Img != NULL) {
        delete[] Img;
        Img=NULL;
    }

    vector<float> ipixels(GrayImg, GrayImg + width * height);
    //释放内存
    delete [] GrayImg;
    GrayImg=NULL;

    float zoom1=0;
    int wS1=0, hS1=0;
    vector<float> ipixels1_zoom;
    if (flag_resize != 0) {
        cerr << "WARNING: The input images are resized to " << wS << "x" << hS << " for ASIFT. " << endl;
        cerr << "But the results will be normalized to the original image size." << endl;

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
        //缩小图像
        if ( zoom1 > 1 ) {
            float sigma_aa = InitSigma_aa * zoom1 / 2;
            GaussianBlur1D(ipixels,width,height,sigma_aa,1);
            GaussianBlur1D(ipixels,width,height,sigma_aa,0);
        }

        // simulate a tilt: subsample the image along the vertical axis by a factor of t.
        ipixels1_zoom.resize(wS1*hS1);
        fproj (ipixels, ipixels1_zoom, width, height, &fproj_sx, &fproj_sy, &fproj_bg, &fproj_o, &fproj_p,
               &fproj_i , fproj_x1 , fproj_y1 , fproj_x2 , fproj_y2 , fproj_x3 , fproj_y3, fproj_x4, fproj_y4);

    } else {
        ipixels1_zoom.resize(width*height);
        ipixels1_zoom = ipixels;
        wS1 = width;
        hS1 = height;
        zoom1 = 1;
    }
    ipixels.clear();

    /*特征提取*/
    int num_of_tilts = 7;
    int verb = 0;

    // Define the SIFT parameters
    siftPar siftparameters;
    default_sift_parameters(siftparameters);

    vector< vector< keypointslist > > keys;

    compute_asift_keypoints(ipixels1_zoom, wS1, hS1, num_of_tilts, verb, keys, siftparameters);
    ipixels1_zoom.clear();

    ofstream fout(Output_FileName.c_str());
    if (!fout) {
        cerr << "Output File Name Error!" << endl;
        throw runtime_error("ImageRetrieve OutPut Feature File Error !");
    }

    /*将特征写入文件*/
    int tt,rr;
    for (tt = 0; tt < (int) keys.size(); tt++) {
        for (rr = 0; rr < (int) keys[tt].size(); rr++) {
            keypointslist::iterator ptr = keys[tt][rr].begin();
            for(; ptr!=keys[tt][rr].end(); ptr++) {

                vector<double> tmp;
                tmp.resize(VecLength);
                tmp.assign(ptr->vec, ptr->vec+VecLength);
                feature_vec.push_back(tmp);

                for (i = 0; i < (int) VecLength-1 ; i++)
                    fout << ptr->vec[i] << ',';
                fout << ptr->vec[VecLength-1];
                fout << endl;
            }
        }
    }

    fout.close();
    keys.clear();
    return true;
}

