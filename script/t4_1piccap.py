# -*- coding: utf-8 -*-
"""
Created on Mon Jun 06 15:59:08 2016

@author: yangsong
"""
import os
import os.path
import datetime

import psycopg2
path="e:/CRTData/download/pic"
path1="/opt/tomcat/webapps/targetreg/admindata/photo"
foldername=''
targetno=0
targetname=''
filename=''
t=()
folderlist=os.listdir(path)
for folder in folderlist:
    foldername=folder
    t=foldername.split('_')
    targetno=t[0]
    targetname=t[1]
    print targetno
    
    print targetname
    
    conn=psycopg2.connect(database="testdb",user="postgres",password="csuduc",host="10.2.3.119",port="5432")

    oldpath=path+"/"+folder
    filelist=os.listdir(oldpath)
    for file in filelist:
        newpath=path1+'/'+targetno+"/"
        print newpath
        filename=file
        print filename
        cur=conn.cursor()
        cur.execute("INSERT INTO t4_1piccap(targetno,targetname,cappath,capname,timeadd,isusercap)VALUES(%s,%s,%s,%s,%s,'0')",(targetno,targetname.decode("gbk").encode("utf8"),newpath.decode("gbk").encode("utf8"),filename,datetime.datetime.now()))
        conn.commit()
        cur.close()
    
    

#conn=psycopg2.connect(database="testdb",user="postgres",password="csuduc",host="10.2.3.119",port="5432")
#cur=conn.cursor()

#cur.execute("INSERT INTO t4pic(picpath,picname,targetname,targetno,featurepath,dicpath,status_)VALUES(%s,%s,%s,%s,%s,%s,%s)",('f','P','f',1,'f','f',2));
#conn.commit()
#cur.close()
#print "success"


