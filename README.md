# 算法后台服务器

[TOC]

## 算法说明

## 环境搭建

## 使用说明
### 程序运行所提供的模块
- log module
- rpc module
- serialize module
- json configure

### Image Retrieve Algorithm
#### 使用ZEROC
```bash
slice2cpp
```

#### 使用hiredis
```bash
sudo apt-get install libhiredis-dev
when you complie, you should -lhiredis
```

#### 使用pgxx操作pg数据库
```bash
sudo apt-get install libpqxx-dev
在编译时使用，-lpqxx -lpq
```

## 参考文献


## 开源代码引使用说明
1. ASIFT[http://www.ipol.im/pub/art/2011/my-asift/?utm_source=doi]
2. EIGEN[https://github.com/RLovelett/eigen]
3. libMatch[https://github.com/flowgrammable/libmatch]
