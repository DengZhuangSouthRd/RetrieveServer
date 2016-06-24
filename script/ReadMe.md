# 脚本

样本收集按类放入不同文件夹中，文件夹命名规则：类号_类名
注：同一目标的普通图像与遥感图像视为不同类（类名相同，类号不同）


2016.06.24
t7dictionary.py 将字典录入testdb数据库的t7dictionary表，字典命名规则为：类号_类名_d.csv。

t4_1piccap.py 将普通图片写入testdb数据库的t4_1piccap表，
              (1)本地每类文件夹命名规则为：类号_类名
              (2)服务器每类文件夹命名规则为：类号，保证存储路径无中文字符

t5remotecap.py 将遥感图片写入testdb数据库的t5remotecap表，
               (1)本地每类文件夹命名规则为：类号_类名
               (2)服务器每类文件夹命名规则为：类号，保证存储路径无中文字符

t3targetinfo.py 根据字典名称将目标信息录入testdb数据库的t3targetinfo表

