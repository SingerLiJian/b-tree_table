#ifndef TABLE_H
#define TABLE_H
#include <iostream>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "BPlusTree.h"
#include "Mutex.h"
using namespace std;
#define TABLE_FILE_NAME "TABLE"
#define byte8 8

class table
{
public:
    table();
    ~table();
    int appendRAND();//加一行，内容随机
    int readLine();
    int readTable();
    int searchAttr(int Attr,int lowerl,int upperl);
    int searchBPlus();
    BPlusTree* indexCreate(int ind, int Attr);
    int recursionWrite(CNode* pNode, int ind);
    CNode* recursionCreate(int ind, int round);
    int showTree();
    int SetPreLeaf(CLeafNode* Leaf)
    {
        preLeaf = Leaf;
    }
    CLeafNode* GetPreLeaf()
    {
        return preLeaf;
    }
private:
    //Fd代表文件访问，cursor代表游标。
    int Fd,ind[100],cursor,flag,alCreate[100];//flag数组为1表示表被修改;alCreate为1代表已经建立对应属性索引。
    BPlusTree* bTree;
    CLeafNode* preLeaf;//用于依文件建树
    Mutex MutexFd;//表的互斥量
    Mutex MutexBtree;//B+树的互斥量
    Mutex MUtexIndex[100];//索引文件的互斥量
};

table::table()
{
    Fd = open(TABLE_FILE_NAME, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    flag = 0;
    for (int i = 0; i < 100; i++)//初始化
        alCreate[i] = 0;
    bTree = new BPlusTree;
    preLeaf = NULL;
}

table::~table()
{
    close(Fd);
}

int table::appendRAND()
{
    char numR[9];//留一个位置给\0
    int num;
    unsigned int byte2=2,byte1=1;
    MutexFd.Lock();//必须整段封锁，因为加一整行不可以被打断
    cursor = lseek(Fd, 0 ,SEEK_END);
    sleep(10);//用于测试
    for (int i = 0; i < 100; i++)
    {
        num= rand() % 100000000;
        sprintf(numR,"%d",num);
        int r= write(Fd, numR, byte8);
    }
    MutexFd.Unlock();
    flag = 1;//表示已经修改
    return 0;
}

int table::readLine()//取出存储表一行
{
    char numR[9];
    int r=1;
    MutexFd.Lock();//读取一行必须完整进行，进行互斥
    for (int i = 0; i < 100; i++)
    {
        r = read(Fd , numR , byte8);
        if (r==0)
            break;
        cout<<numR<<" ";
    }
    MutexFd.Unlock();//解除互斥
    cout<<endl;
    return r;
}

int table::readTable()//取出整个表
{
    int r;
    cursor= lseek(Fd, 0 ,SEEK_SET);
    while (r=readLine()!=0);
    return 0;
}

int table::searchAttr(int Attr,int lowerl,int upperl)//线性查找
{
    char numR[9];
    int num=0,line[10],i=0,j=0;//i代表第几个符合的行，j代表在总的第几行
    MutexFd.Lock();
    cursor = lseek(Fd, 8*Attr ,SEEK_SET);
    while (read(Fd , numR , byte8)!=0)
    {
        sscanf(numR,"%d",&num);
        if ((num>=lowerl)&&(num<=upperl))
        {
            line[i++]=j;
        }
        if (i==11)//最多显示十行
            break;
        cursor = lseek(Fd, 792 ,SEEK_CUR);
        j++;
    }
    MutexFd.Unlock();
    int k=0;
    while (k<=i)//循环输出符合条件的行
    {
        cursor = lseek(Fd, 800*line[k] ,SEEK_SET);
        int r = readLine();
        k++;
    }
    return 0;
}

int table::searchBPlus()
{
    int order,Attr,lowerl,upperl;
    char fileName[3];//最多100个属性
    int a[10];
    cout<<"输入属性 下限 上限"<<endl;
    cin>>Attr>>lowerl>>upperl;
    sprintf(fileName,"%d",Attr);
    if ((flag==0)&&(alCreate[Attr]==1))//表未被修改且存在对应索引
    {
        MUtexIndex[Attr].Lock();//同一时间同意属性的索引文件只可被一个子程序访问
        sleep(20);//测试用
        ind[Attr] = open(fileName, O_RDWR | O_APPEND, S_IRUSR | S_IWUSR);//利用文件构建树
        if (ind[Attr] == -1)
        {
            return -1;
        }
        MutexBtree.Lock();//自此树结构不许更改；
        recursionCreate(ind[Attr], 0);
    }
    else//不存在可用索引或被修改表，需重新创建
    {
        MUtexIndex[Attr].Lock();//同一时间同意属性的索引文件只可被一个子程序访问
        //sleep(20);//测试用
        if (alCreate[Attr]==1){//双保险
            cout<<"索引文件更新，请重新使用该功能"<<endl;
            MUtexIndex[Attr].Unlock();
            return -2;
        }
        ind[Attr] = open(fileName, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);//假设文件打开数没有限制。
        MutexBtree.Lock();//自此树结构不许更改；
        //sleep(20);//测试用
        bTree = indexCreate(ind[Attr],Attr);
        alCreate[Attr]=1;//表示成功创建索引
    }
    close(ind[Attr]);
    MUtexIndex[Attr].Unlock();
    bTree->SearchInterval(lowerl,upperl,a);
    MutexBtree.Unlock();
    for (int i = 0; i < 10; i++)
    {
        if (a[i]==-1)
        {
            break;
        }
        cursor = lseek(Fd, 800*a[i] ,SEEK_SET);
        int r = readLine();
    }
    return 0;
}
//创建索引
BPlusTree* table::indexCreate(int ind,int Attr)
{
    char numR[9];
    int num=0,i=0;//i用于存储行
    nodeData temp;
    MutexFd.Lock();//整段不许更改表
    cursor = lseek(Fd, 8*Attr ,SEEK_SET);
    while (read(Fd , numR , byte8)!=0)//在内存中建立b+树
    {
        sscanf(numR,"%d",&num);
        temp.data = num;
        temp.rows = i;
        (void)bTree->Insert(temp);
        cursor = lseek(Fd, 792 ,SEEK_CUR);
        i++;
    }
    MutexFd.Unlock();
    //将树存进索引文件里
    CNode* pNode = bTree->GetRoot();
    recursionWrite(pNode, ind);
    return bTree;
}

//递归存
int table::recursionWrite(CNode* pNode, int ind)
{
    char state[60];
    unsigned int leng=0;
    if (pNode == NULL)
    {
        return 0;
    }
    if (pNode->GetType() == NODE_TYPE_INTERNAL)
    {
        sprintf(state,"%d",NODE_TYPE_INTERNAL);
        leng += 1;
        sprintf(state+leng,"%d",pNode->GetCount());
        leng += 1;
        for (int i = 1; i <= pNode->GetCount(); i++)
        {
            sprintf(state+leng,"%8d",pNode->GetElement(i).data);
            leng += 8;
            sprintf(state+leng,"%4d",pNode->GetElement(i).rows);//限制了行数最大9999
            leng += 4;
        }
        int r= write(ind, state, leng);
        for (int i = 1; i <= pNode->GetCount()+1; i++)
            recursionWrite(pNode->GetPointer(i), ind);
    }
    else if (pNode->GetType() == NODE_TYPE_LEAF)//叶子就是出口
    {
        sprintf(state,"%d",NODE_TYPE_LEAF);
        leng += 1;
        sprintf(state+leng,"%d",pNode->GetCount());
        leng += 1;
        for (int i = 1; i <= pNode->GetCount(); i++)
        {
            sprintf(state+leng,"%8d",pNode->GetElement(i).data);
            leng += 8;
            sprintf(state+leng,"%4d",pNode->GetElement(i).rows);//%4d限制了行数最大9999
            leng += 4;
        }
        int r= write(ind, state, leng);
        return 0;
    }
}

//递归取
CNode* table::recursionCreate(int ind, int round)
{
    int num;
    char num8[8],num4[4],num1[1];
    nodeData temp;
    read(ind , num1 , 1);//读取索引文件1字节表示结点类型
    sscanf(num1,"%d",&num);
    if (num == NODE_TYPE_INTERNAL)
    {
        CNode* pNode= new CInternalNode;
        read(ind , num1 , 1);
        sscanf(num1,"%d",&num);
        pNode->SetCount(num);
        for (int i = 1; i <= pNode->GetCount(); i++)//中间节点赋值
        {
            read(ind , num8 , 8);
            sscanf(num8,"%d",&num);
            temp.data = num;
            read(ind , num4 , 4);
            sscanf(num4,"%d",&num);
            temp.rows = num;
            pNode->SetElement(i,temp);
        }
        if (round==0)//接收到第一个结点直接必为根节点
        {
            bTree->SetRoot(pNode);
        }
        for (int i = 1; i <= pNode->GetCount()+1; i++)//递归寻找子节点
        {
            pNode->SetPointer(i, recursionCreate(ind, round++));
        }
    }
    else if (num == NODE_TYPE_LEAF)
    {
        CNode* pNode= new CLeafNode;
        read(ind , num1 , 1);
        sscanf(num1,"%d",&num);
        pNode->SetCount(num);
        for (int i = 1; i <= pNode->GetCount(); i++)//叶子节点则赋值
        {
            read(ind , num8 , 8);
            sscanf(num8,"%d",&num);
            temp.data = num;
            read(ind , num4 , 4);
            sscanf(num4,"%d",&num);
            temp.rows = num;
            pNode->SetElement(i,temp);
        }
        //实现前后叶子相连链表
        if (preLeaf != NULL)
        {
            preLeaf->m_pNextNode = (CLeafNode*)pNode;
        }
        preLeaf = (CLeafNode*)pNode;
        return pNode;
    }
}

int table::showTree()
{
    MutexBtree.Lock();
    if (bTree->GetRoot() == NULL)
    {
        return -1;
    }
    bTree->PrintTree();
    MutexBtree.Unlock();
    return 0;
}

#endif