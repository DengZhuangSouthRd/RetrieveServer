/*
主要是用来设计海量图像检索的RPC调用接口
*/

module RPCImgRecong {
    struct WordWiki {
        string key; //关键字
        string abstr; //
        string descr; //描述信息
    };
	struct ImgInfo {
        int id; //数据库id
        string path; //文件路径 /a/b/c/d/
        string name; //文件名 用于WordRes时 只存储关键词
    };
    sequence<ImgInfo> ListString;
    dictionary<string, string> DictStr2Str;

    struct ImgRes {
	    int status;
        ListString imgRemote;
        ListString imgPic;
    };
    struct WordRes {
	    int status; //-1:错误 0:没有目标 1:正常-普通图像 2：正常-遥感图像
        ListString keyWords;
    };
    
    interface ImgRetrieval {
        WordWiki wordGetKnowledge(string word); // 通过图像的名字，得到图像的知识库信息

        /*
        "id" : "sdmdsfdsfdk"
        "word" : "北京站"
        */
        WordRes wordSearch(DictStr2Str mapArg); // 通过单词搜索得到单词对象列表

        /*
        "id" : "sdmdsfdsfdk"
        "word" : "北京站"
        */
        ImgRes wordSearchImg(DictStr2Str mapArg); // 通过单词搜索得到图像地址列表

        /*
        "id" : "sdmdsfdsfdk"
        "ppath" : "/home/data/test/"
        "pname" : "test.jpg"
        "upleftx" : ""
        "uplefty" : ""
        "height"  : ""
        "width"   : ""
        "saveurl" : "" 路径+名称  后缀同源文件
        "featureurl" : "" 路径+名称	.csv
        */
        int imgSearchAsync(DictStr2Str mapArg); //通过图像搜索得到文字列表, 提交的是异步任务，提交成功返回直 > 0, 提交失败返回直 < 0
        WordRes fetchImgSearchResult(DictStr2Str mapArg); // 根据提交的任务ID，取回识别结果
        WordRes imgSearchSync(DictStr2Str mapArg); // 通过图像搜索得到文字列表，提交的是同步任务
    };
};
