#pragma once
#include "LLORAM.h"


//std::Function<int(string)> fun;
struct Char20
{
	unsigned char c[20];
	byte len;
};

inline Char20 StrToChar20(string s)
{
	Char20 obj = { 0 };
	int len = s.length();
	if (len > 20) len = 20;
	memcpy(obj.c, s.c_str(), len);
	obj.len = len;
	return obj;
};

inline Mem20 fun(Char20 s)
{
	Mem20 m;
	myhash::Blake2b((void*)s.c, s.len, m.bytes);
	return m;
}

#define Items_Size 4
struct R
{
	unsigned char c1[20* Items_Size];
	Char20 v1[Items_Size];
};

class OHashTable
{
	int m_L=1;
	LLORAM<R> pathoram;
public:
	OHashTable(int L);
	Char20 read(Char20 k);
	void write(Char20 k, Char20 v);
	bool deleteByKey(Char20 k);
	void writestring(string k, string v);
	string readstring(string k);
	int GetBandWidthBytes();
	bool deleteByString(string k);
	void resize(int newL);
	~OHashTable();
	int DebugSearchLength = 0;
	int DebugReadWriteRound = 0;
};


inline OHashTable::OHashTable(int L)
{
	pathoram.SetL(L);
	m_L = L;
}

inline Char20 OHashTable::read(Char20 k)
{
	Mem20 m= fun(k);
	unsigned long long a = *(unsigned long long*)&m.bytes;//�ĳ�8ֱ�ӵķ��ʵ�ַ �����ܻ���������룿��ʹ���룬��ϣֵ�Ѿ������ˣ�Ҳ����ν��
	for (int j = 0; j < pathoram.capacity; j++)//��ೢ��10�Σ�����ʧ��
	{
		a = a % pathoram.capacity;
		R r = pathoram.read(a);
		DebugReadWriteRound++;
		char* p1 = (char*)r.c1;
		Char20* pv = (Char20*)&r.v1;
		for (int i = 0; i < Items_Size; i++)
		{
			DebugSearchLength++;
			if (memcmp(p1, m.bytes, sizeof(m)) == 0)//�ҵ���ͬ��
			{
				return *pv;
			}
			
			p1 += sizeof(m);
			pv++;
		}
		a = a + j * j*j;//��ת����һ����ַ ����ʽѰַ ƽ������̽��
	}
	//�������û�ҵ�
	Char20 ret = { 0 };
	return ret;
}

Mem20 memEmpty = { 0 };
Mem20 memDeleted = { -1 };

inline void OHashTable::write(Char20 k,Char20 v)
{
	Mem20 m = fun(k);
	unsigned long long a = *(unsigned long long*)&m.bytes;
	for (int j = 0; j < pathoram.capacity; j++)//��ೢ��10�Σ�����ʧ��
	{
		a = a % pathoram.capacity;
		R r = pathoram.read(a);
		DebugReadWriteRound++;
		char* p1 = (char*)r.c1;
		Char20* pv = (Char20*)&r.v1;
		for (int i = 0; i < Items_Size; i++)
		{
			DebugSearchLength++;
			if ((memcmp(p1, memEmpty.bytes, sizeof(memEmpty)) == 0)|| (memcmp(p1, memDeleted.bytes, sizeof(memDeleted)) == 0))//�ҵ��˿�λ
			{
				memcpy(p1, m.bytes, sizeof(m));
				*pv = v;
				pathoram.write(a, r);
				return;
			}
			
			p1 += sizeof(m);
			pv++;
		}
		a = a+j*j*j;//��ת����һ����ַ ����ʽѰַ ƽ������̽��
	}
	//ʧ�� ���費����ʧ��
}

inline bool OHashTable::deleteByKey(Char20 k)
{
	Mem20 m = fun(k);
	unsigned long long a = *(unsigned long long*)&m.bytes;
	for (int j = 0; j < pathoram.capacity; j++)//��ೢ��10�Σ�����ʧ��
	{
		a = a % pathoram.capacity;
		R r = pathoram.read(a);
		DebugReadWriteRound++;
		char* p1 = (char*)r.c1;
		Char20* pv = (Char20*)&r.v1;
		for (int i = 0; i < Items_Size; i++)
		{
			DebugSearchLength++;
			if ((memcmp(p1, m.bytes, sizeof(m)) == 0) )//�ҵ���
			{
				memcpy(p1, memDeleted.bytes, sizeof(memDeleted));//����ΪĹ��
				pathoram.write(a, r);
				return true;
			}
			p1 += sizeof(m);
			pv++;
		}
		a = a + j * j*j;//��ת����һ����ַ ����ʽѰַ ƽ������̽��
	}
	//ʧ�� ���費����ʧ��
	return false;
}

inline void OHashTable::writestring(string k, string v)
{
	Char20 obj1 = StrToChar20(k);
	Char20 obj2 = StrToChar20(v);
	write(obj1, obj2);
}

inline string OHashTable::readstring(string k)
{
	Char20 obj = StrToChar20(k);
	Char20 ret = read(obj);
	return string((char*)ret.c,ret.len);
}

inline bool OHashTable::deleteByString(string k)
{
	auto v = StrToChar20(k);
	return deleteByKey(v);
}


inline void OHashTable::resize(int newL)
{
	pathoram.SetL(newL);
	m_L = newL;	
	throw exception("not implemented!");
}

inline OHashTable::~OHashTable()
{
}
inline int OHashTable::GetBandWidthBytes()
{
	return pathoram.request_bytes;
}
