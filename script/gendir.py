# -*- coding: utf-8 -*-
"""
Created on Fri Jul 22 16:27:48 2016
根据已有类别图像 生成遥感图像文件夹
@author: yangsong
"""

import os
import os.path
path="E:/CRTData/download/pic/"
remotepath="E:/CRTData/download/remote/"
classnum = 7
s = "(遥感)"
targetno=0
targetname=''
filename=''
t=()
filelist=os.listdir(path)
for file in filelist:
    filename=file
    t=filename.split('_')
    targetno=t[0]
    targetname=t[1]
    #print (remotepath+str(int(targetno)+7)+targetname).decode("gbk").encode("utf8")
    print s
    os.mkdir((path+str(int(targetno)+classnum)+'_'+targetname+s.decode("utf8").encode("gbk")));