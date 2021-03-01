#pragma once
#include <string>
using namespace std;
class CPath64
{
public:
	CPath64();
	CPath64(long long node_value);
	CPath64 LeftChild();
	CPath64 RightChild();
	CPath64 Sibiling();
	CPath64 Father();
	string ToString();
	string P(long long x, int l, int height);
	string AtLevelPath(int l);
	void LoadPath(string path);
	void LoadLeaf(int level,long long leaf);
	unsigned long long MapToRandomLeaf(int height);
	//ӳ�䵽һ�������Ҷ��
	unsigned long long GetLeaf();//��ȡ����߿�ʼ����ǰλ�õ�λ�ã�����ߵĽڵ�leaf=0
	unsigned long long value; //���ڵ�value=0 ����� 2i+1���ұ�2i+2
	~CPath64();
};

//Number��0��ʼ��� ��ߺ���Ϊ2*i+1���ұߺ���Ϊ2*i+2
unsigned long long GetFatherNumber(unsigned long long node);
unsigned long long GetLeftChildNumber(unsigned long long node);//2*I+1
unsigned long long GetRightChildNumber(unsigned long long node);//2*I+2
string NumberToPath(unsigned long long node);
unsigned long long PathToNumber(string path);

