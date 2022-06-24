#ifndef BPLUSTREE_H
#define BPLUSTREE_H
#define ORDER_V 2

#define MAXNUM_KEY (ORDER_V * 2) 
#define MAXNUM_POINTER (MAXNUM_KEY + 1) 
#define MAXNUM_DATA (ORDER_V * 2)  

#include "stdio.h"
#include "stdlib.h"


/* 结点类型 */
enum NODE_TYPE
{
NODE_TYPE_ROOT     = 1,    // 根结点
NODE_TYPE_INTERNAL = 2,    // 内部结点
NODE_TYPE_LEAF     = 3,    // 叶子结点
};

#define FLAG_LEFT 1
#define FLAG_RIGHT 2

struct nodeData
{
    int data;
    int rows;
};

typedef nodeData KEY_TYPE; 

/* 结点数据结构，为内部结点和叶子结点的父类 */
class CNode
{
public:

    CNode();
    virtual ~CNode();
    NODE_TYPE GetType() { return m_Type; }
    void SetType(NODE_TYPE type) {m_Type = type;}
    int GetCount() { return m_Count;}
    void SetCount(int i) { m_Count = i; }
    virtual KEY_TYPE GetElement(int i)  = 0;
    virtual void SetElement(int i, KEY_TYPE value) { }
    virtual CNode* GetPointer(int i) {return NULL;}
    virtual void SetPointer(int i, CNode* pointer) { }
    CNode* GetFather() { return m_pFather;}
    void SetFather(CNode* father) { m_pFather = father; }
    CNode* GetBrother(int& flag);
    void DeleteChildren();

protected:
    NODE_TYPE m_Type;    // 结点类型，取值为NODE_TYPE类型
    int m_Count;    // 有效数据个数，对中间结点指键个数，对叶子结点指数据个数
    CNode* m_pFather;     // 指向父结点的指针，标准B+树中并没有该指针，加上是为了更快地实现结点分裂和旋转等操作
};

/* 内部结点数据结构 */
class CInternalNode : public CNode
{
public:

    CInternalNode();
    virtual ~CInternalNode();

    // 获取和设置键值，对用户来说，数字从1开始，实际在结点中是从0开始的
    KEY_TYPE GetElement(int i)
    {
        if ((i > 0 ) && (i <= MAXNUM_KEY))
        {
            return m_Keys[i - 1];
        }
    }

    void SetElement(int i, KEY_TYPE key)
    {
        if ((i > 0 ) && (i <= MAXNUM_KEY))
        {
            m_Keys[i - 1] = key;
        }
    }

    // 获取和设置指针，对用户来说，数字从1开始
    CNode* GetPointer(int i)
    {
        if ((i > 0 ) && (i <= MAXNUM_POINTER))
        {
            return m_Pointers[i - 1];
        }
        else
        {
            return NULL;
        }
    }

    void SetPointer(int i, CNode* pointer)
    {
        if ((i > 0 ) && (i <= MAXNUM_POINTER))
        {
            m_Pointers[i - 1] = pointer;
        }
    }

    bool Insert(KEY_TYPE value, CNode* pNode);
    bool Delete(KEY_TYPE value);
    KEY_TYPE Split(CInternalNode* pNode, KEY_TYPE key);
    bool Combine(CNode* pNode);
    bool MoveOneElement(CNode* pNode);

protected:

    KEY_TYPE m_Keys[MAXNUM_KEY];           // 键数组
    CNode* m_Pointers[MAXNUM_POINTER];     // 指针数组

};

/* 叶子结点数据结构 */
class CLeafNode : public CNode
{
public:

    CLeafNode();
    virtual ~CLeafNode();

    // 获取和设置数据
    KEY_TYPE GetElement(int i)
    {
        if ((i > 0 ) && (i <= MAXNUM_DATA))
        {
            return m_Datas[i - 1];
        }
    }

    void SetElement(int i, KEY_TYPE data)
    {
        if ((i > 0 ) && (i <= MAXNUM_DATA))
        {
            m_Datas[i - 1] = data;
        }
    }

    // 获取和设置指针，对叶子结点无意义，只是实行父类的虚函数
    CNode* GetPointer(int i)
    {
        return NULL;
    }
    bool Insert(KEY_TYPE value);
    bool Delete(KEY_TYPE value);
    KEY_TYPE Split(CNode* pNode);
    bool Combine(CNode* pNode);

public:
    CLeafNode* m_pPrevNode;                 // 前一个结点
    CLeafNode* m_pNextNode;                 // 后一个结点
   
protected:
    KEY_TYPE m_Datas[MAXNUM_DATA];    // 数据数组

};

/* B+树数据结构 */
class BPlusTree
{
public:
  
    BPlusTree();
    virtual ~BPlusTree();
    int Search(int data, char* sPath);
    int SearchInterval(int lowerl, int upperl,int *a);
    bool Insert(KEY_TYPE data);
    bool Delete(KEY_TYPE data);
    void ClearTree();
    void PrintTree();
    BPlusTree* RotateTree();
    bool CheckTree();
    void PrintNode(CNode* pNode);
    bool CheckNode(CNode* pNode);
    CNode* GetRoot()
    {
        return m_Root;
    }
    void SetRoot(CNode* root)
    {
        m_Root = root;
    }
    int GetDepth()
    {
        return m_Depth;
    }
    void SetDepth(int depth)
    {
        m_Depth = depth;
    }
    void IncDepth()
    {
        m_Depth = m_Depth + 1;
    }
    void DecDepth()
    {
        if (m_Depth > 0)
        {
            m_Depth = m_Depth - 1;
        }
    }

public:
    CLeafNode* m_pLeafHead;                 // 头结点
    CLeafNode* m_pLeafTail;                   // 尾结点

protected:
    CLeafNode* SearchLeafNode(KEY_TYPE data);
    bool InsertInternalNode(CInternalNode* pNode, KEY_TYPE key, CNode* pRightSon);
    bool DeleteInternalNode(CInternalNode* pNode, KEY_TYPE key);
    CNode* m_Root;    // 根结点
    int m_Depth;      // 树的深度
};

CNode::CNode()
{
    m_Type = NODE_TYPE_LEAF;
    m_Count = 0;
    m_pFather = NULL;
}
CNode::~CNode()
{
    DeleteChildren();
}

// 获取一个最近的兄弟结点
CNode* CNode::GetBrother(int& flag)
{
    CNode* pFather = GetFather();   //获取其父结点指针
    if (NULL == pFather)
    {
        return NULL;
    }

    CNode* pBrother = NULL;

    for (int i = 1; i <= pFather->GetCount() + 1; i++)   //GetCount()表示获取数据或关键字数，要比指针数小1。
    {
        // 找到本结点的位置
        if (pFather->GetPointer(i) == this)
        {
            if (i == (pFather->GetCount() + 1))   //表示其为父结点的最右边孩子。
            {
                pBrother = pFather->GetPointer(i - 1);    // 本身是最后一个指针，只能找前一个指针
                flag = FLAG_LEFT;
            }
            else
            {
                pBrother = pFather->GetPointer(i + 1);    // 优先找后一个指针
                flag = FLAG_RIGHT;
            }
        }
    }

    return pBrother;
}

// 递归删除子结点
void CNode::DeleteChildren()   // 疑问：这里的指针下标是否需要从0开始
{
    for (int i = 1; i <= GetCount(); i++)   //GetCount()为返回结点中关键字即数据的个数
    {
        CNode * pNode = GetPointer(i);
        if (NULL != pNode)    // 叶子结点没有指针
        {
            pNode->DeleteChildren();
        }

        delete pNode;
    }
}

//将内部节点的关键字和指针分别初始化为0和空
CInternalNode::CInternalNode()    
{
    m_Type = NODE_TYPE_INTERNAL;

    int i = 0;
    for (i = 0; i < MAXNUM_KEY; i++)
    {
        m_Keys[i].data = 0;
        m_Keys[i].rows = 0;
    }

    for (i = 0; i < MAXNUM_POINTER; i++)
    {
        m_Pointers[i] = NULL;
    }
}
CInternalNode::~CInternalNode()
{
    for (int i = 0; i < MAXNUM_POINTER; i++)
    {
        m_Pointers[i] = NULL;
    }
}

bool CInternalNode::Insert(KEY_TYPE value, CNode* pNode)
{
    int i;
    // 如果中间结点已满，直接返回失败
    if (GetCount() >= MAXNUM_KEY)
    {
        return false;
    }

    int j = 0;

    // 找到要插入键的位置
    for (i = 0; (value.data > m_Keys[i].data) && ( i < m_Count); i++)
    {
    }

    // 当前位置及其后面的键依次后移，空出当前位置
    for (j = m_Count; j > i; j--)
    {
        m_Keys[j] = m_Keys[j - 1];
    }

    // 当前位置及其后面的指针依次后移
    for (j = m_Count + 1; j > i + 1; j--)
    {
        m_Pointers[j] = m_Pointers[j - 1];
    }
   
    // 把键和指针存入当前位置
    m_Keys[i] = value;
    m_Pointers[i + 1] = pNode;    // 注意是第i+1个指针而不是第i个指针
    pNode->SetFather(this);      // 非常重要  该函数的意思是插入关键字value及其所指向子树

    m_Count++;

    // 返回成功
    return true;
}

// 在中间结点中删除键，以及该键后的指针
bool CInternalNode::Delete(KEY_TYPE key)
{
    int i,j,k;
    for (i = 0; (key.data >= m_Keys[i].data) && (i < m_Count); i++)
    {
    }

    for (j = i - 1; j < m_Count - 1; j++)
    {
        m_Keys[j] = m_Keys[j + 1];
    }
    m_Keys[j].data = 0;
    m_Keys[j].rows = 0;

    for (k = i; k < m_Count; k++)
    {
        m_Pointers[k] = m_Pointers[k + 1];
    }
    m_Pointers[k] = NULL;

    m_Count--;

    return true;
}


KEY_TYPE CInternalNode::Split(CInternalNode* pNode, KEY_TYPE key)  //key是新插入的值，pNode是分裂结点
{
    int i = 0, j = 0;
    nodeData temp ={0,0};
   
    // 如果要插入的键值在第V和V+1个键值中间，需要翻转一下，因此先处理此情况
    if ((key.data > this->GetElement(ORDER_V).data) && (key.data < this->GetElement(ORDER_V + 1).data))
    {
        // 把第V+1 -- 2V个键移到指定的结点中
        for (i = ORDER_V + 1; i <= MAXNUM_KEY; i++)
        {
            j++;
            pNode->SetElement(j, this->GetElement(i));
            this->SetElement(i, temp);
        }

        // 把第V+2 -- 2V+1个指针移到指定的结点中
        j = 0;
        for (i = ORDER_V + 2; i <= MAXNUM_POINTER; i++)
        {
            j++;
            this->GetPointer(i)->SetFather(pNode);    // 重新设置子结点的父亲
            pNode->SetPointer(j, this->GetPointer(i));
            this->SetPointer(i, NULL);
        }

        // 设置好Count个数
        this->SetCount(ORDER_V);
        pNode->SetCount(ORDER_V);

        // 把原键值返回
        return key;
    }

    // 判断是提取第V还是V+1个键
    int position = 0;
    if (key.data < this->GetElement(ORDER_V).data)
    {
        position = ORDER_V;
    }
    else
    {
        position = ORDER_V + 1;
    }

    // 把第position个键提出来，作为新的键值返回
    KEY_TYPE RetKey = this->GetElement(position);

    // 把第position+1 -- 2V个键移到指定的结点中
    j = 0;
    for (i = position + 1; i <= MAXNUM_KEY; i++)
    {
        j++;
        pNode->SetElement(j, this->GetElement(i));
        this->SetElement(i, temp);
    }

    // 把第position+1 -- 2V+1个指针移到指定的结点中(注意指针比键多一个)
    j = 0;
    for (i = position + 1; i <= MAXNUM_POINTER; i++)
    {
        j++;
        this->GetPointer(i)->SetFather(pNode);    // 重新设置子结点的父亲
        pNode->SetPointer(j, this->GetPointer(i));
        this->SetPointer(i, NULL);
    }

    // 清除提取出的位置
    this->SetElement(position, temp);

    // 设置好Count个数
    this->SetCount(position - 1);
    pNode->SetCount(MAXNUM_KEY - position);


    return RetKey;
}

//结合结点，把指定中间结点的数据全部剪切到本中间结点
bool CInternalNode::Combine(CNode* pNode)
{
    // 参数检查
    if (this->GetCount() + pNode->GetCount() + 1> MAXNUM_DATA)    // 预留一个新键的位置
    {
        return false;
    }
   
    // 取待合并结点的第一个孩子的第一个元素作为新键值
    KEY_TYPE NewKey = pNode->GetPointer(1)->GetElement(1); 

    m_Keys[m_Count] = NewKey;
    m_Count++;
    m_Pointers[m_Count] = pNode->GetPointer(1);   

    for (int i = 1; i <= pNode->GetCount(); i++)
    {
        m_Keys[m_Count] = pNode->GetElement(i);
        m_Count++;
        m_Pointers[m_Count] = pNode->GetPointer(i+1);
    }

    return true;
}

// 从另一结点移一个元素到本结点
bool CInternalNode::MoveOneElement(CNode* pNode)
{
    // 参数检查
    if (this->GetCount() >= MAXNUM_DATA)
    {
        return false;
    }

    int i,j;
    nodeData temp = {0,0};

    // 兄弟结点在本结点左边
    if (pNode->GetElement(1).data < this->GetElement(1).data)
    {
        // 先腾出位置
        for (i = m_Count; i > 0; i--)
        {
            m_Keys[i] = m_Keys[i -1];
        }
        for (j = m_Count + 1; j > 0; j--)
        {
            m_Pointers[j] = m_Pointers[j -1];
        }
        // 第一个键值不是兄弟结点的最后一个键值，而是本结点第一个子结点的第一个元素的值
        m_Keys[0] = GetPointer(1)->GetElement(1);
        // 第一个子结点为兄弟结点的最后一个子结点
        m_Pointers[0] = pNode->GetPointer(pNode->GetCount() + 1);
        pNode->SetElement(pNode->GetCount(), temp);
        pNode->SetPointer(pNode->GetCount() + 1, NULL);
    }
    else    // 兄弟结点在本结点右边
    {
        // 最后一个键值不是兄弟结点的第一个键值，而是兄弟结点第一个子结点的第一个元素的值
        m_Keys[m_Count] = pNode->GetPointer(1)->GetElement(1);
        // 最后一个子结点为兄弟结点的第一个子结点
        m_Pointers[m_Count + 1] = pNode->GetPointer(1);
       
        // 修改兄弟结点
        for (i = 1; i < pNode->GetCount() - 1; i++)
        {
            pNode->SetElement(i, pNode->GetElement(i + 1));
        }
        for (j = 1; j < pNode->GetCount(); j++)
        {
            pNode->SetPointer(j, pNode->GetPointer(j + 1));
        }
    }

    // 设置数目
    this->SetCount(this->GetCount() + 1);
    pNode->SetCount(pNode->GetCount() - 1);

    return true;
}

// 清除叶子结点中的数据
CLeafNode::CLeafNode()
{
    m_Type = NODE_TYPE_LEAF;
    nodeData temp = {0,0};
    for (int i = 0; i < MAXNUM_DATA; i++)
    {
        m_Datas[i] = temp;
    }

    m_pPrevNode = NULL;
    m_pNextNode = NULL;
}
CLeafNode::~CLeafNode()
{

}

// 在叶子结点中插入数据
bool CLeafNode::Insert(KEY_TYPE value)
{
    int i,j;
    // 如果叶子结点已满，直接返回失败
    if (GetCount() >= MAXNUM_DATA)
    {
        return false;
    }

    // 找到要插入数据的位置
    for (i = 0; (value.data > m_Datas[i].data) && ( i < m_Count); i++)
    {
    }

    // 当前位置及其后面的数据依次后移，空出当前位置
    for (j = m_Count; j > i; j--)
    {
        m_Datas[j] = m_Datas[j - 1];
    }

    // 把数据存入当前位置
    m_Datas[i] = value;

    m_Count++;

    // 返回成功
    return true;
}

bool CLeafNode::Delete(KEY_TYPE value)
{
    int i,j;
    bool found = false;
    nodeData temp = {0,0};
    for (i = 0; i < m_Count; i++)
    {
        if (value.data == m_Datas[i].data)
        {
            found = true;
            break;
        }
    }
    // 如果没有找到，返回失败
    if (false == found)
    {
        return false;
    }

    // 后面的数据依次前移
    for (j = i; j < m_Count - 1; j++)
    {
        m_Datas[j] = m_Datas[j + 1];
    }

    m_Datas[j] = temp;
    m_Count--;

    // 返回成功
    return true;

}

// 分裂叶子结点，把本叶子结点的后一半数据剪切到指定的叶子结点中
KEY_TYPE CLeafNode::Split(CNode* pNode)    
{
    // 把本叶子结点的后一半数据移到指定的结点中
    int j = 0;
    nodeData temp = {0,0};
    for (int i = ORDER_V + 1; i <= MAXNUM_DATA; i++)
    {
        j++;
        pNode->SetElement(j, this->GetElement(i));
        this->SetElement(i, temp);
    }
    // 设置好Count个数
    this->SetCount(this->GetCount() - j);
    pNode->SetCount(pNode->GetCount() + j);

    // 返回新结点的第一个元素作为键
    return pNode->GetElement(1);
}

// 结合结点，把指定叶子结点的数据全部剪切到本叶子结点
bool CLeafNode::Combine(CNode* pNode)
{
    // 参数检查
    if (this->GetCount() + pNode->GetCount() > MAXNUM_DATA)
    {
        return false;
    }
   
    for (int i = 1; i <= pNode->GetCount(); i++)
    {
        this->Insert(pNode->GetElement(i));
    }

    return true;
}
BPlusTree::BPlusTree()
{
    m_Depth = 0;
    m_Root = NULL;
    m_pLeafHead = NULL;
    m_pLeafTail = NULL;
}
BPlusTree::~BPlusTree()
{
    ClearTree();
}

// 在树中查找数据
int BPlusTree::Search(int data, char* sPath)
{
    int i = 0;
    int offset = 0;
    if (NULL != sPath)
    {
        sprintf(sPath+offset, "The search path is:");
        offset+=19;
    }

    CNode * pNode = GetRoot();
    // 循环查找对应的叶子结点
    while (NULL != pNode)
    {        
        // 结点为叶子结点，循环结束
        if (NODE_TYPE_LEAF == pNode->GetType())
        {
            break;
        }

        // 找到第一个键值大于等于key的位置
        for (i = 1; (data >= pNode->GetElement(i).data )&& (i <= pNode->GetCount()); i++)
        {
        }

        if (NULL != sPath)
        {
            (void)sprintf(sPath+offset, " %3d -->", pNode->GetElement(1).data);
            offset+=8;
        }

        pNode = pNode->GetPointer(i);
    }

    // 没找到
    if (NULL == pNode)
    {
        return -1;
    }

    if (NULL != sPath)
    {
        (void)sprintf(sPath+offset, "%3d", pNode->GetElement(1).data);
        offset+=3;
    }

    // 在叶子结点中继续找
    bool found = false;
    for (i = 1; (i <= pNode->GetCount()); i++)
    {
        if (data == pNode->GetElement(i).data)
        {
            found = true;
        }
    }

    if (NULL != sPath)
    {
        if (true == found)
        {
            (void)sprintf(sPath+offset, " ,successed.");
            return pNode->GetElement(i).rows;
        }
        else
        {
            (void)sprintf(sPath+offset, " ,failed.");
            return -1;
        }
    }
}

int BPlusTree::SearchInterval(int lowerl,int upperl,int *a)
{
    int i = 0,dcount=0;
    for (int i = 0; i < 10; i++)//最多存10个
    {
        a[i] = -1;
    }
    CNode * pNode = GetRoot();
    // 循环查找对应的叶子结点
    while (NULL != pNode)
    {        
        // 结点为叶子结点，循环结束
        if (NODE_TYPE_LEAF == pNode->GetType())
        {
            break;
        }

        // 找到第一个键值大于等于key的位置
        for (i = 1; (lowerl >= pNode->GetElement(i).data )&& (i <= pNode->GetCount()); i++)
        {}

        pNode = pNode->GetPointer(i);
    }
    
    if (pNode == NULL)//b+树性质决定
    {
        return -1;
    }
    CLeafNode *leaf = (CLeafNode*)pNode;
    // 在叶子结点中继续找
    while (leaf != NULL)
    {
        for (i = 1; i <= leaf->GetCount(); i++)
        {
            if ((lowerl <= leaf->GetElement(i).data)&&(upperl >= leaf->GetElement(i).data))
            {
                a[dcount++]=leaf->GetElement(i).rows;
            }
            else if (upperl <= leaf->GetElement(i).data)
            {
                return 0;
            }
        }
        leaf = leaf->m_pNextNode;
    }
}


bool BPlusTree::Insert(KEY_TYPE data)  //
{
    // 检查是否重复插入
    nodeData temp = {0,0};
    char a[1000];
    int found = Search(data.data, a);
    if (-1 != found)
    {
        return false;
    }

    // 查找理想的叶子结点
    CLeafNode* pOldNode = SearchLeafNode(data);
    // 如果没有找到，说明整个树是空的，生成根结点
    if (NULL == pOldNode)
    {
        pOldNode = new CLeafNode;
        m_pLeafHead = pOldNode;   
        m_pLeafTail = pOldNode;
        SetRoot(pOldNode);
    }

    // 叶子结点未满，对应情况1，直接插入
    if (pOldNode->GetCount() < MAXNUM_DATA)
    {
        return pOldNode->Insert(data);
    }

    // 原叶子结点已满，新建叶子结点，并把原结点后一半数据剪切到新结点
    CLeafNode* pNewNode = new CLeafNode;
    KEY_TYPE key = temp;
    key = pOldNode->Split(pNewNode);   

    // 在双向链表中插入结点
    CLeafNode* pOldNext = pOldNode->m_pNextNode;
    pOldNode->m_pNextNode = pNewNode;
    pNewNode->m_pNextNode = pOldNext;
    pNewNode->m_pPrevNode = pOldNode;
    if (NULL == pOldNext)
    {
        m_pLeafTail = pNewNode;
    }
    else
    {
        pOldNext->m_pPrevNode = pNewNode;
    }


    // 判断是插入到原结点还是新结点中，确保是按数据值排序的
    if (data.data < key.data)
    {
        pOldNode->Insert(data);    // 插入原结点
    }
    else
    {
        pNewNode->Insert(data);    // 插入新结点
    }

    // 父结点
    CInternalNode* pFather = (CInternalNode*)(pOldNode->GetFather());

    // 如果原结点是根节点，对应情况2
    if (NULL == pFather)
    {
        CNode* pNode1 = new CInternalNode;
        pNode1->SetPointer(1, pOldNode);                           // 指针1指向原结点
        pNode1->SetElement(1, key);                                // 设置键
        pNode1->SetPointer(2, pNewNode);                           // 指针2指向新结点
        pOldNode->SetFather(pNode1);                               // 指定父结点
        pNewNode->SetFather(pNode1);                               // 指定父结点
        pNode1->SetCount(1);

        SetRoot(pNode1);                                           // 指定新的根结点
        return true;
    }

    // 情况3和情况4在这里实现
    bool ret = InsertInternalNode(pFather, key, pNewNode);
    return ret;
}


bool BPlusTree::Delete(KEY_TYPE data)
{
    // 查找理想的叶子结点
    CLeafNode* pOldNode = SearchLeafNode(data);
    // 如果没有找到，返回失败
    if (NULL == pOldNode)
    {
        return false;
    }

    // 删除数据，如果失败一定是没有找到，直接返回失败
    bool success = pOldNode->Delete(data);
    if (false == success)
    {
        return false;
    }

    // 获取父结点
    CInternalNode* pFather = (CInternalNode*)(pOldNode->GetFather());
    if (NULL == pFather)
    {
        // 如果一个数据都没有了，删除根结点(只有根节点可能出现此情况)
        if (0 == pOldNode->GetCount())
        {
            delete pOldNode;
            m_pLeafHead = NULL;
            m_pLeafTail = NULL;
            SetRoot(NULL);
        }

        return true;
    }

   
    // 删除后叶子结点填充度仍>=50%，对应情况1
    if (pOldNode->GetCount() >= ORDER_V)
    {
        for (int i = 1; (data.data >= pFather->GetElement(i).data) && (i <= pFather->GetCount()); i++)
        {
            // 如果删除的是父结点的键值，需要更改该键
            if (pFather->GetElement(i).data == data.data)
            {
                pFather->SetElement(i, pOldNode->GetElement(1));    // 更改为叶子结点新的第一个元素
            }
        }

        return true;
    }

    // 找到一个最近的兄弟结点(根据B+树的定义，除了叶子结点，总是能找到的)
    int flag = FLAG_LEFT;
    CLeafNode* pBrother = (CLeafNode*)(pOldNode->GetBrother(flag));

    // 兄弟结点填充度>50%，对应情况2A
    nodeData temp = {0,0};
    KEY_TYPE NewData = temp;
    if (pBrother->GetCount() > ORDER_V)
    {
        if (FLAG_LEFT == flag)    // 兄弟在左边，移最后一个数据过来
        {
            NewData = pBrother->GetElement(pBrother->GetCount());
        }
        else    // 兄弟在右边，移第一个数据过来
        {
            NewData = pBrother->GetElement(1);
        }

        pOldNode->Insert(NewData);
        pBrother->Delete(NewData);

        // 修改父结点的键值
        if (FLAG_LEFT == flag)
        {
            for (int i = 1; i <= pFather->GetCount() + 1; i++)
            {
                if (pFather->GetPointer(i) == pOldNode && i > 1)
                {
                    pFather->SetElement(i - 1 , pOldNode->GetElement(1));    // 更改本结点对应的键
                }
            }
        }
        else
        {
            for (int i = 1; i <= pFather->GetCount() + 1; i++)
            {
                if (pFather->GetPointer(i) == pOldNode && i > 1)
                {
                    pFather->SetElement(i - 1, pOldNode->GetElement(1));    // 更改本结点对应的键
                }
                if (pFather->GetPointer(i) == pBrother && i > 1)
                {
                    pFather->SetElement(i - 1 , pBrother->GetElement(1));    // 更改兄弟结点对应的键
                }
            }
        }


        return true;
    }

    // 父结点中要删除的键
    KEY_TYPE NewKey = temp;

    // 把本结点与兄弟结点合并，无论如何合并到数据较小的结点，这样父结点就无需修改指针
   
    if (FLAG_LEFT == flag)
    {
        (void)pBrother->Combine(pOldNode);
        NewKey = pOldNode->GetElement(1);

        CLeafNode* pOldNext = pOldNode->m_pNextNode;
        pBrother->m_pNextNode = pOldNext;
        // 在双向链表中删除结点
        if (NULL == pOldNext)
        {
            m_pLeafTail = pBrother;
        }
        else
        {
            pOldNext->m_pPrevNode = pBrother;
        }
        // 删除本结点
        delete pOldNode;
    }
    else
    {
        (void)pOldNode->Combine(pBrother);
        NewKey = pBrother->GetElement(1);

        CLeafNode* pOldNext = pBrother->m_pNextNode;
        pOldNode->m_pNextNode = pOldNext;
        // 在双向链表中删除结点
        if (NULL == pOldNext)
        {
           m_pLeafTail = pOldNode;
        }
        else
        {
            pOldNext->m_pPrevNode = pOldNode;
        }
         // 删除本结点
        delete pBrother;
    }

    return DeleteInternalNode(pFather, NewKey);
}

// 清除整个树，删除所有结点
void BPlusTree::ClearTree()
{
    CNode* pNode = GetRoot();
    if (NULL != pNode)
    {
        pNode->DeleteChildren();
   
        delete pNode;
    }

    m_pLeafHead = NULL;
    m_pLeafTail = NULL;
    SetRoot(NULL);
}

// 旋转以重新平衡，实际上是把整个树重构一下,结果不理想，待重新考虑
BPlusTree* BPlusTree::RotateTree()
{
    BPlusTree* pNewTree = new BPlusTree;
    int i = 0;
    CLeafNode * pNode = m_pLeafHead;
    while (NULL != pNode)
    {
        for (int i = 1; i <= pNode->GetCount(); i ++)
        {
            (void)pNewTree->Insert(pNode->GetElement(i));
        }

        pNode = pNode->m_pNextNode;
    }

    return pNewTree;
   
}
// 检查树是否满足B+树的定义
bool BPlusTree::CheckTree()
{
    CLeafNode * pThisNode = m_pLeafHead;
    CLeafNode * pNextNode = NULL;
    while (NULL != pThisNode)
    {
        pNextNode = pThisNode->m_pNextNode;
        if (NULL != pNextNode)
        {
           if (pThisNode->GetElement(pThisNode->GetCount()).data > pNextNode->GetElement(1).data)
           {
               return false;
           }
        }
        pThisNode = pNextNode;
    }
       
    return CheckNode(GetRoot());
}

// 递归检查结点及其子树是否满足B+树的定义
bool BPlusTree::CheckNode(CNode* pNode)
{
    if (NULL == pNode)
    {
        return true;
    }
   
    int i = 0;
    bool ret = false;
   
    // 检查是否满足50%的填充度
    if ((pNode->GetCount() < ORDER_V) && (pNode != GetRoot()))
   {
        return false;
    }

    // 检查键或数据是否按大小排序
    for (i = 1; i < pNode->GetCount(); i++)
    {
        if (pNode->GetElement(i).data > pNode->GetElement(i + 1).data)
        {
            return false;
        }
    }

    if (NODE_TYPE_LEAF == pNode->GetType())
    {
        return true;
    }

    // 对中间结点，递归检查子树
    for (i = 1; i <= pNode->GetCount() + 1; i++)
    {
        ret = CheckNode(pNode->GetPointer(i));
     // 只要有一个不合法就返回不合法
        if (false == ret)
        {
            return false;
        }
    }

    return true;

}

// 打印整个树
void BPlusTree::PrintTree()//只显示四层，用于简单的测试
{
    CNode* pRoot = GetRoot();
    if (NULL == pRoot) return;

    CNode* p1, *p2, *p3;
    int i, j, k;
    int total = 0;

    printf("\n第一层\n | ");
    PrintNode(pRoot);
    total = 0;
    printf("\n第二层\n | ");
    for (i = 1; i <= MAXNUM_POINTER; i++)
    {
        p1 = pRoot->GetPointer(i);
        if (NULL == p1) continue;
        PrintNode(p1);
        total++;
        if (total%4 == 0) printf("\n | ");
    }
    total = 0;
    printf("\n第三层\n | ");
    for (i = 1; i <= MAXNUM_POINTER; i++)
    {
        p1 = pRoot->GetPointer(i);
        if (NULL == p1) continue;
        for (j = 1; j <= MAXNUM_POINTER; j++)
        {
            p2 = p1->GetPointer(j);
            if (NULL == p2) continue;
            PrintNode(p2);
            total++;
            if (total%4 == 0) printf("\n | ");
        }
    }
    total = 0;
    printf("\n第四层\n | ");
    for (i = 1; i <= MAXNUM_POINTER; i++)
    {
        p1 = pRoot->GetPointer(i);
        if (NULL == p1) continue;
        for (j = 1; j <= MAXNUM_POINTER; j++)
        {
            p2 = p1->GetPointer(j);
            if (NULL == p2) continue;
            for (k = 1; k <= MAXNUM_POINTER; k++)
            {
                p3 = p2->GetPointer(k);
                if (NULL == p3) continue;
                PrintNode(p3);
                total++;
                if (total%4 == 0) printf("\n | ");
            }
        }
    }
}

// 打印某结点
void BPlusTree::PrintNode(CNode* pNode)
{
    if (NULL == pNode)
    {
        return;
    }
   
    for (int i = 1; i <= MAXNUM_KEY; i++)
    {
        printf("%3d ", pNode->GetElement(i).data);
        if (i >= MAXNUM_KEY)
        {
            printf(" | ");
        }
    }
}

// 查找对应的叶子结点
CLeafNode* BPlusTree::SearchLeafNode(KEY_TYPE data)
{
    int i = 0;

    CNode * pNode = GetRoot();
    // 循环查找对应的叶子结点
    while (NULL != pNode)
    {        
        // 结点为叶子结点，循环结束
        if (NODE_TYPE_LEAF == pNode->GetType())
        {
            break;
        }

        // 找到第一个键值大于等于key的位置
        for (i = 1; i <= pNode->GetCount(); i++)
        {
            if (data.data < pNode->GetElement(i).data)
            {
                break;
            }
        }

        pNode = pNode->GetPointer(i);
    }

    return (CLeafNode*)pNode;
}

//递归函数：插入键到中间结点
bool BPlusTree::InsertInternalNode(CInternalNode* pNode, KEY_TYPE key, CNode* pRightSon)
{
    nodeData temp = {0,0};
    if (NULL == pNode || NODE_TYPE_LEAF ==pNode->GetType())
    {
        return false;
    }

    // 结点未满，直接插入
    if (pNode->GetCount() < MAXNUM_KEY)
    {
        return pNode->Insert(key, pRightSon);
    }

    CInternalNode* pBrother = new CInternalNode;  //C++中new 类名表示分配一个类需要的内存空间，并返回其首地址；
    KEY_TYPE NewKey = temp;
    // 分裂本结点
    NewKey = pNode->Split(pBrother, key);   

    if (pNode->GetCount() < pBrother->GetCount())
    {
        pNode->Insert(key, pRightSon);
    }
    else if (pNode->GetCount() > pBrother->GetCount())
    {
         pBrother->Insert(key, pRightSon);
    }
    else    // 两者相等，即键值在第V和V+1个键值中间的情况，把字节点挂到新结点的第一个指针上
    {
        pBrother->SetPointer(1,pRightSon);
        pRightSon->SetFather(pBrother);
    }

    CInternalNode* pFather = (CInternalNode*)(pNode->GetFather());
    // 直到根结点都满了，新生成根结点
    if (NULL == pFather)
    {
        pFather = new CInternalNode;
        pFather->SetPointer(1, pNode);                           // 指针1指向原结点
        pFather->SetElement(1, NewKey);                          // 设置键
        pFather->SetPointer(2, pBrother);                        // 指针2指向新结点
        pNode->SetFather(pFather);                               // 指定父结点
        pBrother->SetFather(pFather);                            // 指定父结点
        pFather->SetCount(1);

        SetRoot(pFather);                                        // 指定新的根结点
        return true;
    }

    // 递归
    return InsertInternalNode(pFather, NewKey, pBrother);
}

// 递归函数：在中间结点中删除键
bool BPlusTree::DeleteInternalNode(CInternalNode* pNode, KEY_TYPE key)
{
    // 删除键，如果失败一定是没有找到，直接返回失败
    bool success = pNode->Delete(key);
    if (false == success)
    {
        return false;
    }

    // 获取父结点
    CInternalNode* pFather = (CInternalNode*)(pNode->GetFather());
    if (NULL == pFather)
    {
        // 如果一个数据都没有了，把根结点的第一个结点作为根结点
        if (0 == pNode->GetCount())
        {
            SetRoot(pNode->GetPointer(1));
            delete pNode;
        }

        return true;
    }
   
    // 删除后结点填充度仍>=50%
    if (pNode->GetCount() >= ORDER_V)
    {
        for (int i = 1; (key.data >= pFather->GetElement(i).data) && (i <= pFather->GetCount()); i++)
        {
            // 如果删除的是父结点的键值，需要更改该键
            if (pFather->GetElement(i).data == key.data)
            {
                pFather->SetElement(i, pNode->GetElement(1));    // 更改为叶子结点新的第一个元素
            }
        }

        return true;
    }

    //找到一个最近的兄弟结点(根据B+树的定义，除了根结点，总是能找到的)
    int flag = FLAG_LEFT;
    CInternalNode* pBrother = (CInternalNode*)(pNode->GetBrother(flag));
    nodeData temp = {0,0};

    // 兄弟结点填充度>50%
    KEY_TYPE NewData = temp;
    if (pBrother->GetCount() > ORDER_V)
    {
        pNode->MoveOneElement(pBrother);

        // 修改父结点的键值
        if (FLAG_LEFT == flag)
        {
            for (int i = 1; i <= pFather->GetCount() + 1; i++)
            {
                if (pFather->GetPointer(i) == pNode && i > 1)
                {
                    pFather->SetElement(i - 1 , pNode->GetElement(1));    // 更改本结点对应的键
                }
            }
        }
        else
        {
            for (int i = 1; i <= pFather->GetCount() + 1; i++)
            {
                if (pFather->GetPointer(i) == pNode && i > 1)
                {
                    pFather->SetElement(i - 1, pNode->GetElement(1));    // 更改本结点对应的键
                }
                if (pFather->GetPointer(i) == pBrother && i > 1)
                {
                    pFather->SetElement(i - 1 , pBrother->GetElement(1));    // 更改兄弟结点对应的键
                }
            }
        }

        return true;
    }
   
    // 父结点中要删除的键：兄弟结点都不大于50，则需要合并结点，此时父结点需要删除键
    KEY_TYPE NewKey = temp;

    // 把本结点与兄弟结点合并，无论如何合并到数据较小的结点，这样父结点就无需修改指针
    if (FLAG_LEFT == flag)
    {
        (void)pBrother->Combine(pNode);
        NewKey = pNode->GetElement(1);
        delete pNode;
    }
    else
    {
        (void)pNode->Combine(pBrother);
        NewKey = pBrother->GetElement(1);
        delete pBrother;
    }

    // 递归
    return DeleteInternalNode(pFather, NewKey);
}
#endif