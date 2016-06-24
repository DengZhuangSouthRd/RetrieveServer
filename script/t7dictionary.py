# -*- coding: utf-8 -*-
"""
Created on Mon Jun 06 15:59:08 2016

@author: yangsong
"""
import os
import os.path
import psycopg2
import datetime
path="/opt/Dic_test/v1.0/"
windowspath='e:/CRTData/download/dic/'
print windowspath
targetno=0
targetname=''
filename=''
filelist=os.listdir(windowspath)
print filelist
for file in filelist:
    filename=file
    print filename    
    
    t=filename.split('_')
    targetno=t[0]
    targetname=t[1]
    filepath=os.path.join(path,file)
    print filepath
    conn=psycopg2.connect(database="testdb",user="postgres",password="csuduc",host="10.2.3.119",port="5432")
    cur=conn.cursor()   
    cur.execute("INSERT INTO t7dictionary(dicpath,targetname,targetno,timeadd)VALUES(%s,%s,%s,%s)",(filepath.decode("gbk").encode("utf8"),targetname.decode("gbk").encode("utf8"),targetno,datetime.datetime.now()))
    conn.commit()
    cur.close()
    
