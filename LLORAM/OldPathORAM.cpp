#include "OldLLORAM.h"



OldLLORAM::OldLLORAM()
{
}


OldLLORAM::~OldLLORAM()
{
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// Path-ORAM2
LLORAM2::LLORAM2(int m_L)
{
	L = m_L;
	leafs = pow(2, L - 1);//Ҷ�Ӹ��������ĸ߶�ΪL��root L=1 L<=32;
	if (m_L > 1)
	{
		pos_map = new LLORAM2(m_L - 1);
	}
	else
	{
		pos_map = NULL;
	}
}

int LLORAM2::read(int a, bool bFirst)
{
	DataBlock data;
	if (L == 1)
	{
		data = ReadDataFromStash(a, bFirst);
		return data.v;
	}
	access("read", a, bFirst, data);
	return data.v;
}

void LLORAM2::write(int a, bool bFirst, int v)
{
	DataBlock data;
	if (L == 1)
	{
		int new_x = 0;
		WriteStash(a, bFirst, data, new_x); //L==1 ֱ��д�뻺����
		return;
	}
	data.v = v;
	access("write", a, bFirst, data);
}

string LLORAM2::P(int x, int l)//������BUG
{
	if (l == 0) return "";
	bitset<64> s = x;
	string v = s.to_string();
	return v.substr((size_t)(64 - L + 1), (size_t)(l - 1));
}

Bucket LLORAM2::ReadBucket(int x, int l)
{
	string p = P(x, l);
	Bucket b = oram[p];
	return b;
}

void LLORAM2::WriteBucket(int x, int l, Bucket bkt)
{
	//
	oram[P(x, l)] = bkt;
}

DataBlock LLORAM2::ReadDataFromStash(int a, bool bFirst)
{
	DataBlock b = { 0 };
	ORAMBlock ob = stash[a];
	if (bFirst)
	{
		b.v = ob.data;
	}
	else
	{
		b.v = ob.data1;
	}
	return b;
}
void LLORAM2::WriteStash(int a, bool bFirst, DataBlock& data, int pos)
{
	ORAMBlock b = stash[a];
	if (bFirst)
	{
		b.data = data.v;
	}
	else
	{
		b.data1 = data.v;
	}
	//b.pos = pos;
	CPath64 p32;
	p32.LoadLeaf(L, pos);
	b.node = p32.value;
	stash[a] = b;
};

void LLORAM2::access(string op, int a, bool bFirst, DataBlock& data1)
{
	int x = pos_map->read(a / 2, a % 2 == 0);// ��С��ORAM �ж�ȡ
	int new_x = rand32() % (leafs);// ע�⣬���ﲻ��һ��Ĵ�С
	pos_map->write(a / 2, a % 2 == 0, new_x); //д��С��ORAM֮��
	for (int l = 0; l < L; l++)
	{
		Bucket temp = ReadBucket(x, l);
		for (auto b : temp.blocks)
		{
			stash[b.a] = b;
		}
	};
	DataBlock data = ReadDataFromStash(a, bFirst);
	if (op == "write")
	{
		WriteStash(a, bFirst, data1, new_x); //�����ݻ����ڿͻ���
	}
	for (int l = L; l >= 1; l--)
	{
		int i = 0;
		Bucket bkt;
		for (auto v : stash)
		{
			CPath64 p32(v.second.node);
			int leaf = p32.GetLeaf();
			if (P(x, l) == P(leaf, l))
			{
				bkt.blocks.push_back(v.second);
				i++;
				if (i >= Z) break;
			}
		}
		for (auto v : bkt.blocks)
		{
			stash.erase(v.a);//ɾ��ԭ����
		}
		for (; i < Z; i++)
		{
			ORAMBlock empty = { 0 };
			bkt.blocks.push_back(empty);
		}
		WriteBucket(x, l, bkt);
	}
	data1 = data;
}
