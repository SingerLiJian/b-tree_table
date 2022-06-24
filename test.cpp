#include <iostream>
#include "Table.h"
#include "Thread.h"
using namespace std;

int main()
{
    table t;
    int order,Attr,lowerl,upperl;
    cout<<"1:加行 2:线性搜索 3:显示整个表 4:加速搜索 5:显示树 6:并行加行与加速搜索 7:并行加速搜索 0:退出"<<endl;
    cin>>order;
    while (order!=0)
    {
        if (order==1)
        {
            Thread addTh;
            addTh.ADD(&t);//无需等待，多次按1也可并行
        }
        else if (order==2)
        {
            cout<<"输入属性 下限 上限"<<endl;
            cin>>Attr>>lowerl>>upperl;
            int r=t.searchAttr(Attr,lowerl,upperl);
        }
        else if (order==3)
        {
            int r=t.readTable();
        }
        else if (order==4)
        {
            Thread searchTh;
            searchTh.Search(&t);
            searchTh.WaitForDeath();//需要等待，否则选择栏与输入搜索信息顺序冲突
        }
        else if (order==5)
        {
           int r = t.showTree();
           if (r==-1)
           {
               cout<<"暂时没树"<<endl;
           }
        }
        else if (order==6)
        {
            Thread addTh;
            Thread searchTh;
            addTh.ADD(&t);
            searchTh.Search(&t);
            searchTh.WaitForDeath();
        }
        else if (order==7)
        {
            Thread searchTh1;
            Thread searchTh2;
            searchTh1.Search(&t);
            searchTh2.Search(&t);
            searchTh1.WaitForDeath();
            searchTh2.WaitForDeath();
        }
        cout<<"1:加行 2:线性搜索 3:显示整个表 4:加速搜索 5:显示树 6:并行加行与加速搜索 7：并行加速搜索 0:退出"<<endl;
        cin>>order;
    }
    return 0;
}
