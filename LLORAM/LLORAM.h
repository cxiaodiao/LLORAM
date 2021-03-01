#pragma once
#include "common.h"
#include "unordered_map"
#include "bitset"
#include "CPath64.h"
#include "myhash.h"
#include "myAES.h"


#define Z_block 2


using namespace std;

struct Mem20
{
	unsigned char bytes[20];
};

//struct Mem24
//{
//	unsigned char bytes[24];
//};
struct Mem4
{
	unsigned char bytes[4];
};
struct Mem8
{
	unsigned char bytes[8];
};

struct hash_func  
{    
	size_t operator()(const Mem20&addr) const
	{         
		return *(size_t*)addr.bytes;    
	}
};
struct cmp_fun //�ȽϺ��� ==
{    
	bool operator()(const Mem20&a1, const Mem20&a2) const
	{         
		return memcmp(a1.bytes, a2.bytes, sizeof(a1)) == 0 ? true:false;    
	}
};

//static size_t operator()(const Mem20& m1)
//{
//	return memcmp(m1.bytes, m2.bytes, 20) == 0;
//}

struct PointerMapEnc
{
	Mem20 k1;
	Mem8 v1;
	Mem20 k2;
	Mem8 v2; //Ҷ�ӽڵ�
};

struct PointerMap
{
	Mem20 k1;
	unsigned long long node1;

	Mem20 k2;
	unsigned long long node2;
};

struct RawBlock 
{
//	Mem20 S_leaf0;//seed, ���ڷ��ʴ�leaf������·��
//	Mem20 S_leaf1;//seed, a secret key
	//Mem20 M;//used to rebuild the index
	unsigned long long a;
	unsigned long long data;
	unsigned long long data1;
	unsigned long long node;
};

template<class T>
struct RawBlock_L
{
	unsigned long long a;
	T data;
	T data1;
	unsigned long long node;
};

//���ڵ�Ľṹ���£�
// ���ڵ�BucketORAM = (Header,Tail)
//Header=(Sf,mmf,(PointerMap),(PointerMap),(PointerMap),(PointerMap))//���ýڵ�Seed����
//Tail= (S,M, DataBlock,DataBlock,DataBlock,DataBlock) //�˽ڵ����˽Կ��RCPA-secure��������
struct Header
{
//	Mem20 Sf;//pointer to the father node;
	Mem20 mmf;//M XOR Mf
	//PointerMapEnc maps[Z_block];//Z=4Ĭ�ϵ�Ԫ�ظ���
	vector< PointerMapEnc> maps;
	Header()
	{
		maps = vector< PointerMapEnc>(Z_block);
	}
};

struct Tail
{
	//Mem20 S;//seed, a secret key
	Mem20 M;//used to rebuild the index
	//RawBlock blocks[Z_block]; //Z=4 4��
	vector<RawBlock> blocks;
	Tail()
	{
		blocks = vector< RawBlock>(Z_block);
		int j = 1;
		for (auto& v : blocks)
		{
			v.a = -j;
			j++;
		}
	}
};

template<class T>
struct Tail_L
{
	//Mem20 S;//seed, a secret key
	Mem20 M;//used to rebuild the index
	//RawBlock blocks[Z_block]; //Z=4 4��
	vector<RawBlock_L<T>> blocks;
	Tail_L()
	{
		blocks = vector<RawBlock_L<T>>(Z_block);
		int j = 1;
		for (auto& v : blocks)
		{
			v.a = -j;
			j++;
		}
	}
};

template<class T>
struct BucketORAM_L
{
	Header head;//ͷ������ ����ɾ������Ϊ�˴�û��ʹ��
	Tail_L<T> tail;
};


struct BucketORAM
{
	Header head;
	Tail tail;
};

struct E_BucketORAM
{
	Header head;
	vector<byte> etail;//���ܵ�ָ��
};

struct ORAMRootNode
{
	int tree_height;//���ĸ߶�
//	Mem20 Seed;//S
	Mem20 Mask;//M
};

struct LevelLToken
{
	int tree_height;//��ǰ���ĸ߶�
	Mem20 TokenForThisTree;
	//int new_x;//��ӳ��Ľڵ㣿����д����
};

struct ORAMRequest
{
	vector<LevelLToken> tokens;//����L��token ÿ��һ�������һ��L=1 Ҷ����Ϊ1�� ֱ��д����stash�У���д������	
	unsigned long long tree1_pos;//��һ�����Ĳ�ѯλ�ú����ƴ洢�ڿͻ��ˣ���Ҫ���͹���
//	Mem20 tree1_seed;
};


struct ORAMNodeWithPath2
{
	int tree_height;//��ǰ���ĸ߶�
	string path;//��ǰ�ڵ��·��
	vector<byte> etail;
	bool bEmpty=false;
};
struct ORAMResponse
{
	vector<ORAMNodeWithPath2> accessed;//���ʹ��Ľڵ� ͷ���Ѿ����� β��δ����
	vector<ORAMNodeWithPath2> childs;//��Щ����δ���ܵĽڵ�
	vector<unsigned long long> leaf_accessed;//����L�����ʹ���Ҷ�ӣ�ÿ����һ��
};


struct ORAMBucketWithPath2
{
	int tree_height;//��ǰ���ĸ߶�
	string path;//��ǰ�ڵ��·��
	E_BucketORAM bkt;
};
struct ORAMShuffleToken
{
	vector<ORAMBucketWithPath2> rebuild;
	vector<ORAMBucketWithPath2> rebuild_sibling;
};


struct DataBlock
{
	unsigned long long v;
};



template<class T>
class LLORAM
{
private:
	void initClient(int height);
	unsigned long long leafs;
	ORAMResponse res_last;//����ר��
	vector<int> last_accessed_nodes;//����ר��
	ORAMShuffleToken client_side_shuffle_token_for_single_rounnd;

public:
	LLORAM<T>();
	LLORAM<T>(int m_L);
	~LLORAM<T>();
	void DumpDebugLastAccessedLeaves();
	void SetL(int m_L);
	T read(unsigned long long a, bool bSecond);
	void write(unsigned long long a, bool bSecond, T v);
	void write(unsigned long long a, T v);
	T read(unsigned long long a);

	bool ReadBucket(int tree_height, unsigned long long x, int l, E_BucketORAM & b);
	bool ReadBucketByPath(int tree_height, string path, E_BucketORAM & b);


	void single_round_access(string op, unsigned long long a, T& data1);
	void single_round_process_request(ORAMRequest & request, ORAMShuffleToken & lastShuffle, ORAMResponse & response);
	void single_round_process_request_complete(ORAMShuffleToken & shuffle);
	void single_round_access_complete();
	DataBlock ReadDataFromStash(int i_tree, unsigned long long a, bool bSecond);
	T ReadDataFromStash_L(int i_tree, unsigned long long a, bool bSecond);
	void access(string op, unsigned long long a, T& data1);
	void dump();
	void dumpTree();
	void dumpStashOverflow();
	void dumpStash();
	void dumpfile(string filename);
	void dump(ORAMBucketWithPath2 & bk);
	void dump(ORAMShuffleToken & t);
	void dump(ORAMNodeWithPath2 & nd);
	void dump(ORAMResponse & res);
	void dumpData();
	E_BucketORAM GetFather(int i_tree, string path);

	void dumpM();

	Tail ToTail(vector<byte>& b);

	Tail_L<T> ToTail_L(vector<byte>& b);

	vector<byte> ToETail(Tail& t);
	vector<byte> ToETail_L(Tail_L<T>& t);


	bool DebugCheckContainItem(int i_tree, unsigned long long addr, int b, unsigned long long & v, unsigned long long & node);
	bool DebugCheckORAMIntegrity(unsigned long long addr);
	void DebugCheckNodes();
	//���Ĳ����˶����ˣ����ж��ٸ���ʧ
	bool bDebug = false;
	bool bDebugMask = false;
	bool bDebugAccess = false;
	bool bDebugShuffle = false;
	bool bDebugDumpHeader = false;
	string SecretKey = "123456";
	unsigned long long capacity = 0;


	int L=0;//root L=1; the height of the tree.
	vector<ORAMRootNode> roots;
	ORAMNodeWithPath2 ReadNode(int level, unordered_map<string, unsigned long long>& accessed, string currentPath);
	string EncodeAddress(unsigned long long a, bool bSecond);
	void ClientBuildSearchKey(unsigned long long a, bool b, Mem20 M, Mem20 & searchKey);
	bool ReadNodePosition(unordered_map<Mem20, Mem8, hash_func, cmp_fun>& tb, Mem20 & searchKey, unsigned long long & node);
	bool WriteNodePosition(unordered_map<Mem20, Mem8, hash_func, cmp_fun>& tb, Mem20 searchKey, unsigned long long node);
	string GetSibling(string path);
	void Search(ORAMRequest& req, ORAMResponse& res);
	E_BucketORAM BuildBucket(int tree_i, string path, Tail t, Mem20 Mf, Mem20 M);
	E_BucketORAM BuildBucket_L(int tree_i, string path, Tail_L<T> t, Mem20 Mf, Mem20 M);

	E_BucketORAM BuildSiblingBucket(ORAMNodeWithPath2 & old, Mem20 Mf);
	E_BucketORAM BuildSiblingBucket_L(ORAMNodeWithPath2 & old, Mem20 Mf);

	void InitSeeds(unsigned long long x, unordered_map<string, Tail>& seed_init, int i_tree);
	void WriteResponseToStash(ORAMResponse & res);
	void AddPathNodes(int i_tree, int level, unsigned long long leaf_x, Tail& tail, unordered_map<string, Tail>& seed_map, ORAMResponse & res, ORAMShuffleToken& t);
	void AddPathNodes_L(int i_tree, int level, unsigned long long leaf_x, Tail_L<T>& tail, unordered_map<string, Tail_L<T>>& seed_map, ORAMResponse & res, ORAMShuffleToken& t);
	void InitNewPositions(vector<unsigned long long>& new_x_array);

	ORAMShuffleToken BuildShuffleToken(string op, unsigned long long a, T & data1, ORAMResponse & res);
	void Shuffle(ORAMShuffleToken& sf);
	
	//i_tree ���ĸ߶� a Ҷ�ӵ�λ�� bSecond �Ƿ��һ�����˺������ص���
	void WriteStashWithNode(int i_tree, unsigned long long a, bool bSecond, DataBlock & data, unsigned long long node);
	void WriteStashWithNode_L(int i_tree, unsigned long long a, bool bSecond, T & data, unsigned long long node);
	//void WriteStash(int i_tree,unsigned long long a, bool bSecond, DataBlock & data, unsigned long long pos);
	void EncBucketHeader(BucketORAM & bucket, Mem20 key, bool bEncrypt);
	//Mem20 buildToken(unsigned long long addr, int bSecond);
	//unordered_map<Mem20, ORAMNode, hash_func, cmp_fun> oram;
	ORAMRequest BuildORAMRequest(unsigned long long addr);
	//unordered_map<string, ORAMNode> oram;
	//vector<unordered_map<int, RawBlock>> stashs;//�ܹ���L��stash ÿ����һ��
	vector<unordered_map<unsigned long long, RawBlock>> stashs;//�ܹ���L��stash ÿ����һ��
	unordered_map<unsigned long long, RawBlock_L<T>> stash_L;//��L��ORAM

	vector<unordered_map<string, E_BucketORAM>> oram;

	string P(unsigned long long x, int l, int height);

	unsigned long long item_count = 0;
	void testdump(int L);
	int response_bytes=0;
	int request_bytes = 0;
	//�ܹ���L����

};

//-----------------------------------------------------------------------------------------------------------------

struct ORAMBlock
{
	unsigned long long a;
	unsigned long long data;
	unsigned long long data1;
	unsigned long long node;
};

struct Bucket
{
	vector<ORAMBlock> blocks; //Z=4 4��
};



inline void Random(Mem20& mem)
{
	for (int i = 0; i < 20; i++)
	{
		mem.bytes[i] = rand() % 256;
	}
};

//��������KEY
inline void H1(string buf, Mem20& mem)
{
	myhash::Blake2b((void*)buf.c_str(), buf.length(), mem.bytes);
}



////���������Կ
//void H2(char* buf, Mem20& mem)
//{
//	char buf21[21];
//	memcpy(buf21, buf, 20);
//	buf21[20] = '!';
//	myhash::Blake2b((void*)buf21, 21, mem.bytes);
//}
//
////����MAP k��v
//void H3(Mem20& input, Mem20& k,Mem20& v)
//{
//	char buf21[21];
//	unsigned char buf22[40];
//	memcpy(buf21, input.bytes, 20);
//	buf21[20] = '#';
//	myhash::Blake2b_40((void*)buf21, 21,buf22);
//	memcpy(k.bytes, buf22,20);
//	memcpy(v.bytes, buf22+20,20);
//}
//
//void H31(string str, Mem20& mem)
//{
//	string s = str;
//	s += "!";
//	myhash::Blake2b((void*)s.c_str(), s.length(), mem.bytes);
//}

inline void H28(Mem20& input, Mem20& outputKey, Mem8& outputValue)
{
	byte temp[28] = { 0 };
	myhash::Blake2b_28((void*)input.bytes, 20, temp);
	memcpy(outputKey.bytes, temp, 20);
	memcpy(outputValue.bytes, temp + 20, 8);
}


inline void XOR(Mem20& mem1, Mem20& mem2)
{
	for (int i = 0; i < 20; i++)
	{
		mem1.bytes[i] = mem1.bytes[i] ^ mem2.bytes[i];
		//*(BYTE*)(mem1.bytes + i ) =*(BYTE*)(mem1.bytes+i) ^ *(BYTE*)(mem2.bytes + i);
	}
}
inline void XOR(Mem8& mem1, Mem8& mem2)
{
	for (int i = 0; i < 8; i++)
	{
		mem1.bytes[i] = mem1.bytes[i] ^ mem2.bytes[i];
		//*(BYTE*)(mem1.bytes + i ) =*(BYTE*)(mem1.bytes+i) ^ *(BYTE*)(mem2.bytes + i);
	}
}
inline void XOR(byte* p1, byte* p2, int len)
{
	for (int i = 0; i < len; i++)
	{
		p1[i] = p1[i] ^ p2[i];
	}
}

inline void show(Mem20 m)
{
	for (int i = 0; i < 20; i++)
	{
		printf("%.2x", m.bytes[i]);
	}
	printf(" ");
}
inline void show(Mem8 m)
{
	for (int i = 0; i < 8; i++)
	{
		printf("%.2x", m.bytes[i]);
	}
	printf(" ");
}
inline void show(unsigned long long a)
{
	printf("%ld", a);
}
inline void show(string s)
{
	printf("%s", s.c_str());
}
//void F_K(string str, Mem20& mem, string key = "123456")
//{
//	string s = str;
//	s += "$" + key;
//	myhash::Blake2b((void*)s.c_str(), s.length(), mem.bytes);
//}





template<class T>
inline LLORAM<T>::LLORAM()
{
	L = 1;
	myAES::SetPrivateKey((char*)"password");
}

template<class T>
LLORAM<T>::LLORAM(int m_L)
{
	if (m_L > 64)
	{
		printf("error!m_L=%d>64", m_L);
		return;
	}
	L = m_L;
	leafs = pow(2, L - 1);//Ҷ�Ӹ��������ĸ߶�ΪL��root L=1 L<=64;
	initClient(m_L);
	myAES::SetPrivateKey((char*)"password");
}

template<class T>
LLORAM<T>::~LLORAM<T>()
{
}



template<class T>
void LLORAM<T> ::SetL(int m_L)
{
	if (L > m_L)
	{
		printf("error!\r\n Old height %d > New height %d", L, m_L);
		return;
	}
	initClient(m_L);

	L = m_L;
	leafs = pow(2, L - 1);//Ҷ�Ӹ��������ĸ߶�ΪL��root L=1 L<=64;
//	roots.clear();
//	stashs.clear();
//	oram.clear();

}

template<class T> inline void LLORAM<T>::initClient(int height)
{
	//������pow(2, height) -1 ���ڵ㣬ÿһ���ڵ�����洢1�����ݣ�
	capacity = (pow(2, height) - 1);//����ռ���������Ļ� ��ᵼ�µ�һ�����ڵ����ɳ���2��Ԫ�أ�����(a,b)= 0,0 ; 0,1; 1,0; 1,1
	// create L roots	
	if (roots.size() == 0)
	{
		for (int i = 1; i <= height; i++)
		{
			ORAMRootNode node = { 0 };
			Random(node.Mask);
			//Random(node.Seed);
			node.tree_height = i;
			roots.push_back(node);//��ʼ���ͻ��˵Ľڵ�

			unordered_map<unsigned long long, RawBlock> map;
			stashs.push_back(map);//��ʼ���ͻ��˵�mapӳ��

			unordered_map<string, E_BucketORAM> eomap;//�Ȼ���������			Ĭ����128MB����
			oram.push_back(eomap);
		}
	}
	else
	{
		//���µ����߶ȣ�
		if (height < L)
		{
			printf("not suport! height=%d<%d", height, L);
			return;
		}
		else
		{
			//

			for (int i = L + 1; i <= height; i++)
			{
				ORAMRootNode node = { 0 };
				Random(node.Mask);
				//Random(node.Seed);
				node.tree_height = i;
				//roots.push_back(node);//��ʼ���ͻ��˵Ľڵ�
				roots.insert(roots.begin(), node);

				unordered_map<unsigned long long, RawBlock> map;
				//stashs.push_back(map);//��ʼ���ͻ��˵�stashs
				stashs.insert(stashs.begin(), map);
				unordered_map<string, E_BucketORAM> eomap;//

				Mem20 M = roots[0].Mask;

				E_BucketORAM bkt;

				Mem20 Mf = { 0 };//�Ӵ˽ڵ��ȡ������������ʱû����	
				//S = old.tail.S;
				Tail t;



				//bkt.head.mmf = M; //�˴�����

				//�˴�û���ܣ�����Ҫ��һ������Ͱ

				t.blocks[0].a = 0;
				t.blocks[0].data = 0;
				t.blocks[0].data1 = 0;
				t.blocks[0].node = 0;//����һ��ָ����ڵ��Ԫ��
				for (int j = 1; j < Z_block; j++)//���ú���Ϊ�գ�ֻ������һ����
				{
					t.blocks[j].a = -j;
				}
				bkt = BuildBucket(1, "", t, Mf, M);//����ͬ���ڵ��Ѿ����� Tail����					
				eomap[""] = bkt;
				//oram.push_back(eomap);
				oram.insert(oram.begin(), eomap);


			}
			//������С

			//�������
			for (int i = 1; i <= height; i++)
			{
				roots[i - 1].tree_height = i;//���±��				
			}
		}
	}
}




template<class T>
ORAMRequest LLORAM<T>::BuildORAMRequest(unsigned long long addr)
{
	ORAMRequest r;
	unsigned long long a = addr;
	bool b = false;
	for (int i = L; i >= 1; i--)
	{
		b = ((a % 2) == 1);
		a = a / 2;

		LevelLToken tk = { 0 };
		tk.tree_height = i;
		//	Mem20 fk = buildToken(a, b);
		//	XOR(fk, roots[i - 1].Mask); //Level i,�洢��i-1λ�õ�roots������
		if (i < L)//��L�㲻��Ҫ
		{
			Mem20 searchToken;
			//ST= M_root XOR f_k(a||b)
			ClientBuildSearchKey(a, b, roots[i - 1].Mask, searchToken);//ע�⣬�����ǲ��õ�root�ڵ��Mask��Ϊ����

			if (bDebugMask)
			{
				printf("search token i_tree=%d,a=%d,b=%d ST=", i, a, b); show(searchToken); printf(", M="); show(roots[i - 1].Mask);
				printf("\r\n");
			}

			tk.TokenForThisTree = searchToken;
			//tk.new_x= rand32() % (((int)pow(2, i - 1)) / 2);// һ��Ĵ�С Ϊÿ�����һ��λ�ã����ڴ�Сд
		}
		r.tokens.push_back(tk);

		//
		if (i == 1)//��һ����
		{
			r.tree1_pos = 0;//��Ϊֻ��1���ڵ�
			//r.tree1_seed = roots[0].Seed;
		}
	}
	//��Ҫ�����Ʒ�������1��L����
	ORAMRequest r2;
	r2.tree1_pos = r.tree1_pos;
	//r2.tree1_seed = r.tree1_seed;
	for (int i = 1; i <= L; i++)
	{
		r2.tokens.push_back(r.tokens[L - i]);
	}
	return r2;
}

//int LLORAM::ReadPosition(ORAMRequest& req,Mem20 &seed_in_leaf)
//{
//	//
//	return 0;
//}

//�����˽ڵ㣬��������
template<class T>
ORAMNodeWithPath2 LLORAM<T>::ReadNode(int level, unordered_map<string, unsigned long long>& accessed, string currentPath)
{
	ORAMNodeWithPath2 nd;

	nd.tree_height = level;
	nd.path = currentPath;
	if (oram[level - 1].find(currentPath) != oram[level - 1].end())
	{

		nd.etail = oram[level - 1][currentPath].etail;

	}
	else
	{
		//nd.bEmpty = true;//�սڵ��־����Ҫ�ڿͻ����ع��˽ڵ�

	}

	return nd;
}

template<class T>
string LLORAM<T>::EncodeAddress(unsigned long long a, bool bSecond)
{
	string s;
	if (bSecond == 0)
	{
		s = IntToStr(a) + "," + SecretKey + "0";//ģ��F_K(a||b) ������Կ
	}
	else
	{
		s = IntToStr(a) + "," + SecretKey + "1";
	}
	return s;
}

template<class T>
void LLORAM<T>::ClientBuildSearchKey(unsigned long long a, bool bSecond, Mem20 M, Mem20& searchKey)
{
	PointerMap pm;
	Mem20 vkey0;
	Mem20 vkey1;
	string s = EncodeAddress(a, bSecond);
	//printf("build search key:(a=%d,b=%d,M)=>searchkey ", a, b); show(M); show(searchKey);
	H1(s, searchKey);//������Կ��α�������
//	show("M="); show(M); show(",Ab="); show(searchKey); printf(" %s,", s.c_str());
	XOR(searchKey, M); // searchKey = M XOR F_K(a||b)
//	show("searchKey="); show(searchKey); show("\r\n");
	//printf("search key:"); show(searchKey); printf("\r\n");
}

//�����������ƣ�����һ�������õ�λ����Ϣ��λ������
template<class T>
bool LLORAM<T>::ReadNodePosition(unordered_map<Mem20, Mem8, hash_func, cmp_fun>& tb, Mem20& searchKey, unsigned long long & node)
{
	Mem20 k;
	Mem8 v;
	H28(searchKey, k, v);/////////////////////////////////////////////////////////////////////////////////////////////////����������������������������/////��Ҫ�޸�
	if (tb.find(k) == tb.end())
	{
		node = -1;
		//	show(searchKey); printf("=> 0\r\n");
		//	printf("not found:");
			//for (auto v : tb)
			//{
			//	show("(k,v) "); show(v.first); printf(", "); show(v.second); printf("\r\n");
			//}
		return false;//�����쳣 ������������
	};
	Mem8 v2 = tb[k]; //Ӧ���ǿ����ҵ���
	XOR(v2, v);
	//memcpy(seed_in_leaf.bytes, v2.bytes, 20); //��һ���������ƣ�����4���ֽ���λ��
	node = *(unsigned long long*)(v2.bytes);
	//	printf("read:" ); show(searchKey); printf("=>("); show(seed_in_leaf); printf(",%d)\r\n", pos);
	return true;
}
//�����������ƣ�����һ�������õ�λ����Ϣ��λ������
template<class T>
bool LLORAM<T>::WriteNodePosition(unordered_map<Mem20, Mem8, hash_func, cmp_fun>& tb, Mem20 searchKey, unsigned long long node)
{
	Mem20 k;
	Mem8 v;
	H28(searchKey, k, v);/////////////////////////////////////////////////////////////////�޸ģ�
	Mem8 m2;
	memcpy(m2.bytes, &node, sizeof(node));
	XOR(v, m2);
	tb[k] = v;
	//	printf("write:"); printf("pos=%d ", pos); show(searchKey); printf("("); show(k); printf("<="); show(v);printf(")\r\n");
	return true;
}

template<class T>
string LLORAM<T>::GetSibling(string path)
{
	string ret = path;
	if (ret.length() == 0) return "";
	int lastChar = ret.length() - 1;
	if (ret[lastChar] == '0')
	{
		ret[lastChar] = '1';
	}
	else
	{
		ret[lastChar] = '0';
	}
	return ret;
}

template<class T>
void LLORAM<T>::Search(ORAMRequest& req, ORAMResponse& res)
{
	unsigned long long node = 0;

	//	Mem20 seed_in_leaf=req.tree1_seed;//��ʼ����
	//	Mem20 seed= seed_in_leaf;
	unsigned long long x = 0;// req.tree1_pos;//��ʼλ��
	if (bDebugAccess)
	{
		last_accessed_nodes.push_back(0);
	}
	for (int i_tree = 1; i_tree <= L; i_tree++) //�ӵ�1������������L����
	{
		//seed = req.tokens[i_tree-1].tree_height;
		unordered_map<string, unsigned long long> accessed;

		Mem20 searchKey = { 0 };
		if (bDebug)
		{
			CPath64 p64;
			p64.LoadLeaf(i_tree, x);
			printf("read path:[tree=%d,leaf=%ld, path=%s, node=%ld]\r\n", i_tree, x, p64.ToString().c_str(), p64.value);
		}
		//vector < Mem20> searchKeysALL;//�� level L �� level 1����
		//vector < Mem20> mmfAll;//�� level L �� level 1����
		vector<Header> headers;
		//���ܴ�leaf-to-root��ȫ���ڵ�
		for (int l = i_tree; l >= 1; l--)
		{
			E_BucketORAM temp;
			//InitBucketORAM(temp);
			bool bRet = ReadBucket(i_tree, x, l, temp);

			if (bRet == false) continue;//���û�ж����������
			ORAMNodeWithPath2 nd;

			nd.tree_height = i_tree;
			nd.path = P(x, l, i_tree);

			nd.etail = temp.etail;

			res.accessed.push_back(nd);//��ȡ�Ѿ����ʹ��Ľڵ�
			if (l > 1)
			{
				string sibling_path = GetSibling(nd.path);

				ORAMNodeWithPath2 child = ReadNode(nd.tree_height, accessed, sibling_path);//�˴������Ľڵ㲢û�н���
				res.childs.push_back(child);//��ȡͬ���ڵ�
			}
			//�Ƚ���temp.head ��Կ�� seed

			//mmfAll.push_back(temp.head.mmf);
			headers.push_back(temp.head);//����˴�head�Ѿ�����
			//XOR(seed, temp.head.Sf);//�ݹ��ȡ,�õ���һ���ڵ�Ľ�����Կ�����׽ڵ㣩
		};
		//����ڵ������
		//if (headers.size() == i_tree) //�п��ܳ��� Ҷ�ӽڵ�Ϊ�գ������м�ڵ㵽root·��ȫ�����Ѿ���������

		res.leaf_accessed.push_back(x);//����ÿһ�������ʹ���Ҷ��λ��
		if (i_tree == L) break;//��L�㲻��Ҫ����λ��
		int count_headers = headers.size();
		bool bSuccess = false;
		for (int c = 1; c <= count_headers; c++)
		{
			//ROOT =1 Ҫ��Ҫ���ܣ�
			if (c == 1)
			{
				searchKey = req.tokens[i_tree - 1].TokenForThisTree;
			}
			else
			{
				XOR(searchKey, headers[count_headers - c].mmf);
			}

			unordered_map<Mem20, Mem8, hash_func, cmp_fun> computation;
			for (int i = 0; i < headers[count_headers - c].maps.size(); i++)
			{
				PointerMapEnc mp = headers[count_headers - c].maps[i];
				computation[mp.k1] = mp.v1;
				computation[mp.k2] = mp.v2;
			};
			//
		//	printf("Test read level tree=%d,level=%d\r\n",i_tree,l);
			node = -1;
			//	seed_in_leaf = { 0 };
			bSuccess = ReadNodePosition(computation, searchKey, node);//���ݽڵ������������� ���ҵ���һ��λ������
			if (node == -1)
			{
				continue;//�յ�ֵ λ��û��-1
				//printf("read -1; tree=%d c=%d",i_tree,c);
				//getchar();
			}
			if (bSuccess)
			{
				//printf("read leaf-to-root path:%d\r\n", pos);
				break;
			}
		}

		//printf("read paths:(tree=%d,leaf=%d)\r\n", i_tree,pos);
		if (bDebugAccess)
		{
			last_accessed_nodes.push_back(node);
		}
		if (bSuccess)
		{
			if (node != -1)
			{
				CPath64 p64(node);
				//x = p32.GetLeaf();// pos;	
				unsigned long long y = p64.MapToRandomLeaf(i_tree + 1);//ӳ�䵽һ�������Ҷ�ӽڵ㣨��һ����������·����
				x = y;
				//if (x != y)
				//{
				//	printf("x=%d y=%d itree=%d node=%d\r\n", x, y, i_tree, node);
				//	getchar();
				//}
			}
			else
			{
				x = rand64() % ((unsigned long long)pow(2, i_tree + 1 - 1));//���û�ҵ��� �����ȡһ��λ�� +1ָ������һ����
			}
		}
		else
		{
			if (count_headers == 1)//ֻ��һ�����ģ�����������ܲ�ѯ������϶����Ա����ܣ������ǽ��ܴ���
			{
				printf("decode error!i_tree=%d node=%ld\r\n", i_tree, node);//��������еĽڵ㣬���ǽ��ܴ���
				getchar();
			}
			x = rand64() % ((unsigned long long)pow(2, i_tree + 1 - 1));//���û�ҵ��� �����ȡһ��λ�� +1ָ������һ����
		//	x = pos;
		}
		//seed = seed_in_leaf;
	}
}

template<class T>
E_BucketORAM LLORAM<T>::BuildBucket(int tree_i, string path, Tail t, Mem20 Mf, Mem20 M)
{

	E_BucketORAM ebk;
	t.M = M;
	//bk.etail = ToETail(t);	
	Mem20 mmf = M;
	XOR(mmf, Mf);
	ebk.head.mmf = mmf;


	//------------------------------------------------- 
	//�ؼ���������map
	for (int i = 0; i < t.blocks.size(); i++) //����β���� ����ͷ��
	{
		PointerMap pm;
		unordered_map<Mem20, Mem8, hash_func, cmp_fun> tb;

		Mem20 searchKey;
		ClientBuildSearchKey(t.blocks[i].a, 0, t.M, searchKey);
		WriteNodePosition(tb, searchKey, t.blocks[i].data);

		if (bDebugMask)
			if ((long long)t.blocks[i].a >= 0)
			{
				printf("write:tree=%d,path=%s,M=", tree_i, path.c_str()); show(t.M); printf(",a=%d, b=%d; ", t.blocks[i].a, 0);
				printf(" SK="); show(searchKey); printf(",data=%d)\r\n", t.blocks[i].data); //SK= M XOR F_K(data||0)
			}

		Mem20 searchKey1;
		ClientBuildSearchKey(t.blocks[i].a, 1, t.M, searchKey1);
		WriteNodePosition(tb, searchKey1, t.blocks[i].data1);

		if (bDebugMask)
			if ((long long)t.blocks[i].a >= 0)
			{
				printf("write:tree=%d,path=%s,M=", tree_i, path.c_str()); show(t.M); printf(",a=%d, b=%d; ", t.blocks[i].a, 1);
				printf(" SK="); show(searchKey1); printf(",data=%d)\r\n", t.blocks[i].data1);//SK= M XOR F_K(data1||1)
			}

		int j = 0;
		for (auto v : tb)
		{

			if (ebk.head.maps.size() <= i)
			{
				PointerMapEnc enc;
				ebk.head.maps.push_back(enc);
			}
			if (j == 0)
			{
				ebk.head.maps[i].k1 = v.first;
				memcpy(ebk.head.maps[i].v1.bytes, v.second.bytes, sizeof(Mem8));
			}
			if (j == 1)
			{
				ebk.head.maps[i].k2 = v.first;
				memcpy(ebk.head.maps[i].v2.bytes, v.second.bytes, sizeof(Mem8));
				break;
			}


			j++;
		}
	}
	//---------


	ebk.etail = ToETail(t);
	//Tail t2 = ToTail(ebk.etail);
	return ebk;




}


template<class T>
E_BucketORAM LLORAM<T>::BuildBucket_L(int tree_i, string path, Tail_L<T> t, Mem20 Mf, Mem20 M)
{
	E_BucketORAM ebk;
	t.M = M;
	//bk.etail = ToETail(t);	
	Mem20 mmf = M;
	XOR(mmf, Mf);
	ebk.head.mmf = mmf;




	//------------------------------------------------- 
	//��L����������Ҫ����map��
//	for (int i = 0; i < t.blocks.size(); i++) //����β���� ����ͷ��
//	{
//		PointerMap pm;
//		unordered_map<Mem20, Mem8, hash_func, cmp_fun> tb;
//		//bool LLORAM::WritePosition(unordered_map<Mem20, Mem28, hash_func, cmp_fun>& tb, Mem20& searchKey, unsigned long& pos, Mem20 &seed_in_leaf);
//		Mem20 searchKey;
//		ClientBuildSearchKey(t.blocks[i].a, 0, t.M, searchKey);
//		WriteNodePosition(tb, searchKey, t.blocks[i].data);
//
//		if (bDebugMask)
//			if ((int)t.blocks[i].a >= 0)
//			{
//				printf("write:tree=%d,path=%s,M=", tree_i, path.c_str()); show(t.M); printf(",a=%d, b=%d; ", t.blocks[i].a, 0);
//				printf(" SK="); show(searchKey); printf(",data=%d)\r\n", t.blocks[i].data); //SK= M XOR F_K(data||0)
//			}
//
//		Mem20 searchKey1;
//		ClientBuildSearchKey(t.blocks[i].a, 1, t.M, searchKey1);
//		WriteNodePosition(tb, searchKey1, t.blocks[i].data1);
//
//		if (bDebugMask)
//			if ((int)t.blocks[i].a >= 0)
//			{
//				printf("write:tree=%d,path=%s,M=", tree_i, path.c_str()); show(t.M); printf(",a=%d, b=%d; ", t.blocks[i].a, 1);
//				printf(" SK="); show(searchKey1); printf(",data=%d)\r\n", t.blocks[i].data1);//SK= M XOR F_K(data1||1)
//			}
//
//		int j = 0;
//		for (auto v : tb)
//		{
//			if (ebk.head.maps.size() <= i)
//			{
//				PointerMapEnc enc;
//				ebk.head.maps.push_back(enc);
//			}
//			if (j == 0)
//			{
//				ebk.head.maps[i].k1 = v.first;
//				memcpy(ebk.head.maps[i].v1.bytes, v.second.bytes, sizeof(Mem8));
//			}
//			if (j == 1)
//			{
//				ebk.head.maps[i].k2 = v.first;
//				memcpy(ebk.head.maps[i].v2.bytes, v.second.bytes, sizeof(Mem8));
//				break;
//			}
//
//			j++;
//		}
//	}
	//---------


	ebk.etail = ToETail_L(t);
	//Tail t2 = ToTail(ebk.etail);
	return ebk;




}

template<class T>
//��������ָ�룬��Ҫ����˽Կ�Ƚ���tail�ڵ㣬���ýڵ���Կ���ܵ�ǰheader����

E_BucketORAM LLORAM<T>::BuildSiblingBucket(ORAMNodeWithPath2 &old, Mem20 Mf)//Sf���ڵ���Կ��S��ǰ�ڵ���Կ��Mf�ڵ���Կ

{
	Mem20 M;
	//S = old.tail.S;

	Tail t_decrypt;
	if (!old.bEmpty)
	{
		t_decrypt = ToTail(old.etail);//����
		M = t_decrypt.M;
	}
	else
	{
		Random(t_decrypt.M);
		M = t_decrypt.M;
	}

	E_BucketORAM ebkt;
	ebkt = BuildBucket(old.tree_height, old.path, t_decrypt, Mf, M);//����ͬ���ڵ��Ѿ����� Tail����	
	return ebkt;


	//���˽Կ����
}

template<class T>
//��������ָ�룬��Ҫ����˽Կ�Ƚ���tail�ڵ㣬���ýڵ���Կ���ܵ�ǰheader����

E_BucketORAM LLORAM<T>::BuildSiblingBucket_L(ORAMNodeWithPath2 &old, Mem20 Mf)//Sf���ڵ���Կ��S��ǰ�ڵ���Կ��Mf�ڵ���Կ
{
	Mem20 M;
	//S = old.tail.S;
	Tail_L<T> t_decrypt;
	if (!old.bEmpty)
	{
		t_decrypt = ToTail_L(old.etail);//����
		M = t_decrypt.M;
	}
	else
	{
		Random(t_decrypt.M);
		M = t_decrypt.M;
	}

	E_BucketORAM ebkt;
	ebkt = BuildBucket_L(old.tree_height, old.path, t_decrypt, Mf, M);//����ͬ���ڵ��Ѿ����� Tail����	
	return ebkt;

	//���˽Կ����
}

template<class T>
//��ʼ������Ҷ��������·���ϵ�ȫ��seed��mask
void LLORAM<T>::InitSeeds(unsigned long long x, unordered_map<string, Tail>& seed_init, int i_tree)
{
	seed_init.clear();
	for (int l = L; l >= 1; l--)
	{
		string p = P(x, l, i_tree);
		Tail t;
		Random(t.M);
		seed_init[p] = t;
	}
}

template<class T>
void LLORAM<T>::WriteResponseToStash(ORAMResponse& res)
{
	for (int i_access = 0; i_access < res.accessed.size(); i_access++) //�洢ȫ�����ص�����
	{
		//��Ҫ���Ƚ��ܴ˽ڵ�

		if (res.accessed[i_access].tree_height < L)
		{
			Tail t = ToTail(res.accessed[i_access].etail);
			for (int i_block = 0; i_block < t.blocks.size(); i_block++)
			{
				RawBlock bc = t.blocks[i_block];
				if ((long long)bc.a < 0)
				{
					//cout << "zero,";
					continue;//��Ч��ַ������
				}
				unsigned long long d0 = bc.data;
				unsigned long long d1 = t.blocks[i_block].data1;
				DataBlock vdata;
				vdata.v = bc.data;

				int j_tree = res.accessed[i_access].tree_height;
				//bc.pos = new_x_array[j_tree - 1];//�����µ�λ��
				//CPath32 p(bc.node);
				//int leaf = p.GetLeaf();
				WriteStashWithNode(j_tree, bc.a, 0, vdata, bc.node);
				vdata.v = bc.data1;
				WriteStashWithNode(j_tree, bc.a, 1, vdata, bc.node);
			}
		}
		else
		{
			Tail_L<T> t = ToTail_L(res.accessed[i_access].etail);
			for (int i_block = 0; i_block < t.blocks.size(); i_block++)
			{
				RawBlock_L<T> bc = t.blocks[i_block];
				if ((long long)bc.a < 0)
				{
					//cout << "zero,";
					continue;//��Ч��ַ������
				}
				T d0 = bc.data;
				T d1 = t.blocks[i_block].data1;

				int j_tree = res.accessed[i_access].tree_height;
				//bc.pos = new_x_array[j_tree - 1];//�����µ�λ��
				//CPath32 p(bc.node);
				//int leaf = p.GetLeaf();
				WriteStashWithNode_L(j_tree, bc.a, 0, d0, bc.node);
				WriteStashWithNode_L(j_tree, bc.a, 1, d1, bc.node);
			}
		}

	}
}

template<class T>
void LLORAM<T>::AddPathNodes(int i_tree, int level, unsigned long long leaf_x, Tail& tail, unordered_map<string, Tail>& seed_map, ORAMResponse& res, ORAMShuffleToken& t)
{

	ORAMBucketWithPath2 e;

	e.tree_height = i_tree;
	e.path = P(leaf_x, level, i_tree);
	for (int i = 0; i < tail.blocks.size(); i++)
	{
		if ((long long)tail.blocks[i].a >= 0)
		{
			if (bDebug) printf("set path node tree=%d path=%s ,oldLeaf=%d, node=%d\r\n", i_tree, e.path.c_str(), leaf_x, tail.blocks[i].node);
		}
	}
	if (level > 1)
	{
		unsigned long long father = level - 1;
		//Mem20 Sf = seed_map[P(leaf_x, father,i_tree)].S;
		Mem20 Mf = seed_map[P(leaf_x, father, i_tree)].M;
		string currentPath = P(leaf_x, level, i_tree);
		//Mem20 S = seed_map[currentPath].S;
		Mem20 M = seed_map[currentPath].M;
		e.bkt = BuildBucket(i_tree, currentPath, tail, Mf, M);
		;//�˴�û���ܣ�����Ҫ��һ������Ͱ


		ORAMBucketWithPath2 sibling_node;//�ع��ֵܽڵ�

		//for (int i = 0; i < sibling_node.bkt.tail.blocks.size(); i++)
		//{
		//	sibling_node.bkt.tail.blocks[i].a = -1;
		//}
		//string father_path = P(leaf_x, father, i_tree);
		string sibling_path = GetSibling(currentPath);
		sibling_node.tree_height = i_tree;
		sibling_node.path = sibling_path;
		bool bFound = false;
		for (auto old : res.childs)
		{
			if (old.tree_height == i_tree)//����ǵ�ǰ��
			{
				if (old.path == sibling_node.path)//·����ͬ
				{
					bFound = true;
					//�����ͬ���ڵ㣬�����¹���˽ڵ�
					sibling_node.bkt = BuildSiblingBucket(old, Mf);
					break;
				}
			}
		}
		if (!bFound)
		{
			//����ֵܽڵ�û�г�ʼ��
			Tail t0;
			Random(M);
			t0.M = M;
			sibling_node.bkt = BuildBucket(i_tree, sibling_path, t0, Mf, M);//����ͬ���ڵ��Ѿ����� Tail����	
			//Tail test = ToTail(sibling_node.bkt.etail);
		}
		t.rebuild_sibling.push_back(sibling_node);
	}
	else
	{
		//���ڵ�
		Mem20 empty = { 0 };
		//Mem20 S = seed_map[""].S;
		Mem20 M = seed_map[""].M;
		//	printf("build tree %d root:\r\n",i_tree);
		e.bkt = BuildBucket(i_tree, "", tail, empty, M);
		//dump(e);
	}
	t.rebuild.push_back(e);
}


template<class T>
void LLORAM<T>::AddPathNodes_L(int i_tree, int level, unsigned long long leaf_x, Tail_L<T>& tail, unordered_map<string, Tail_L<T>>& seed_map, ORAMResponse& res, ORAMShuffleToken& t)
{
	ORAMBucketWithPath2 e;

	e.tree_height = i_tree;
	e.path = P(leaf_x, level, i_tree);
	for (int i = 0; i < tail.blocks.size(); i++)
	{
		if ((long long)tail.blocks[i].a >= 0)
		{
			if (bDebug) printf("set path node tree=%d path=%s ,oldLeaf=%ld, node=%ld\r\n", i_tree, e.path.c_str(), leaf_x, tail.blocks[i].node);
		}
	}
	if (level > 1)
	{
		unsigned long long father = level - 1;
		//Mem20 Sf = seed_map[P(leaf_x, father,i_tree)].S;
		Mem20 Mf = seed_map[P(leaf_x, father, i_tree)].M;
		string currentPath = P(leaf_x, level, i_tree);
		//Mem20 S = seed_map[currentPath].S;
		Mem20 M = seed_map[currentPath].M;
		e.bkt = BuildBucket_L(i_tree, currentPath, tail, Mf, M);
		;//�˴�û���ܣ�����Ҫ��һ������Ͱ

		ORAMBucketWithPath2 sibling_node;//�ع��ֵܽڵ�
		//for (int i = 0; i < sibling_node.bkt.tail.blocks.size(); i++)
		//{
		//	sibling_node.bkt.tail.blocks[i].a = -1;
		//}
		//string father_path = P(leaf_x, father, i_tree);
		string sibling_path = GetSibling(currentPath);
		sibling_node.tree_height = i_tree;
		sibling_node.path = sibling_path;
		bool bFound = false;
		for (auto old : res.childs)
		{
			if (old.tree_height == i_tree)//����ǵ�ǰ��
			{
				if (old.path == sibling_node.path)//·����ͬ
				{
					bFound = true;
					//�����ͬ���ڵ㣬�����¹���˽ڵ�
					sibling_node.bkt = BuildSiblingBucket_L(old, Mf);
					break;
				}
			}
		}
		if (!bFound)
		{
			//����ֵܽڵ�û�г�ʼ��
			Tail_L<T> t0;
			Random(M);
			t0.M = M;
			sibling_node.bkt = BuildBucket_L(i_tree, sibling_path, t0, Mf, M);//����ͬ���ڵ��Ѿ����� Tail����	
			//Tail test = ToTail(sibling_node.bkt.etail);

		}
		t.rebuild_sibling.push_back(sibling_node);
	}
	else
	{
		//���ڵ�
		Mem20 empty = { 0 };
		//Mem20 S = seed_map[""].S;
		Mem20 M = seed_map[""].M;
		//	printf("build tree %d root:\r\n",i_tree);
		e.bkt = BuildBucket_L(i_tree, "", tail, empty, M);
		//dump(e);
	}
	t.rebuild.push_back(e);
}

template<class T>
void LLORAM<T>::InitNewPositions(vector<unsigned long long>& new_x_array)
{
	new_x_array.clear();
	for (int i_tree = 1; i_tree <= L; i_tree++)
	{
		unsigned long long new_x = rand64() % ((unsigned long long)pow(2, i_tree - 1));// ����һ�������λ��
		new_x_array.push_back(new_x);
	}
}


template<class T>
ORAMShuffleToken LLORAM<T>::BuildShuffleToken(string op, unsigned long long a, T& data1, ORAMResponse& res)
{
	ORAMShuffleToken t;

	unsigned long long aa = a;
	//bool b = false;
	unsigned long long new_x;
	vector<unsigned long long> new_x_array;
	unsigned long long x = 0;
	unordered_map<string, Tail> seed_map;

	unsigned long long s1, s2, s3;
	//s1 = stashs[L - 1].size();//debug
	InitNewPositions(new_x_array);
	WriteResponseToStash(res);

	int b = 0;
	unsigned long long last_x = 0;


	//s2 = stashs[L - 1].size();//debug
	for (int i_tree = L; i_tree >= 1; i_tree--)
	{
		b = ((aa % 2) == 1);
		aa = aa / 2;

		x = res.leaf_accessed[i_tree - 1];//��ȡ���ʹ���Ҷ��
		InitSeeds(x, seed_map, i_tree);//��ʼ����x��root��ȫ��·����seed�������ؽ�
		//����SEED���ͻ���
		//roots[i_tree - 1].Seed = seed_map[""].S;
		roots[i_tree - 1].Mask = seed_map[""].M;
		new_x = new_x_array[i_tree - 1];// rand32() % ((int)pow(2, i_tree - 1));// ����һ�������λ��

		//int pos1=0;

		DataBlock data;
		T data_L;
		if (i_tree < L)
		{
			data = ReadDataFromStash(i_tree, aa, b);//pos��ԭ����λ�ã������µ�λ��
		}
		else
		{
			data_L = ReadDataFromStash_L(i_tree, aa, b);
		}
		if (op == "write")
		{
			if (i_tree == L)
			{
				//WriteStash(i_tree, aa, b, data1, new_x); //�����ݻ����ڿͻ���,����һ����λ��
				CPath64 p;
				p.LoadLeaf(i_tree, new_x);
				WriteStashWithNode_L(i_tree, aa, b, data1, p.value);
			}
		}
		if (op == "read")
		{
			if (i_tree == L)
			{
				data1 = data_L;//������ȡ������ֵ
				CPath64 p;
				p.LoadLeaf(i_tree, new_x);
				WriteStashWithNode_L(i_tree, aa, b, data1, p.value);
			}
		}
		if (i_tree < L) //����L�����������µ�λ��
		{
			DataBlock data2;
			CPath64 p;
			p.LoadLeaf(i_tree + 1, last_x);//��һ���ڵ��Ҷ�� ��Ӧ�Ľڵ���
			data2.v = p.value;//��Ҷ��ת��Ϊ���  ������ʵ�����ӵ���һ���������Ľڵ���
			CPath64 pp;
			pp.LoadLeaf(i_tree, new_x);
			WriteStashWithNode(i_tree, aa, b, data2, pp.value);//�����ϴε�λ����Ϊ����,λ��Ϊ���䵽һ���µĵط�
		}

		//	Mem20 Sf, Mf;
		for (int l = i_tree; l >= 1; l--)// ���� Log(n)��
		{
			int i = 0;
			Tail tail;


			string p1 = P(x, l, i_tree);

			if (i_tree < L)
			{
				for (auto v : stashs[i_tree - 1])
				{
					CPath64 p64(v.second.node);
					//int leaf=p32.MapToRandomLeaf(i_tree);
					//string p2 = P(leaf, l, i_tree);
					string p2 = p64.AtLevelPath(l);
					if ((p1 == p2))
					{
						//tail.blocks.push_back(v.second);
						if (i < Z_block)
						{
							tail.blocks[i] = v.second;
						}
						else
						{
							tail.blocks.push_back(v.second);
						}
						//tail.blocks[i].pos = new_x;//����Ҫ����λ��
						if (bDebug)	printf("itree=%d,level=%d,path=\"%s\",a=%ld, (x=%ld,node=%ld) \r\n", i_tree, l, p1.c_str(), v.first, x, v.second.node);
						i++;
						if ((l == i_tree) || ((l != i_tree) && (p1 != "")))//������������� ������������������Ҳ��Ǹ��ڵ�
						{
							if (i >= Z_block) break;//�����Ƹ���
						};
						//���ڵ���Բ������޵Ľڵ㣨ʵ����ֻ�к��ٵĽڵ㣩
					}
				}
				for (int j = 0; j < i; j++)
				{
					if ((long long)tail.blocks[j].a >= 0)
					{
						stashs[i_tree - 1].erase(tail.blocks[j].a);//
					}
				}
				AddPathNodes(i_tree, l, x, tail, seed_map, res, t);
			}
			else
			{
				Tail_L<T> tail_L;
				for (std::pair<unsigned long long,RawBlock_L<T>> v: stash_L)
				{
					RawBlock_L<T> r = v.second;
					CPath64 p64(v.second.node);
					string p2 = p64.AtLevelPath(l);
					if ((p1 == p2))
					{
						//tail.blocks.push_back(v.second);
						if (i < Z_block)
						{
							tail_L.blocks[i] = v.second;
						}
						else
						{
							tail_L.blocks.push_back(v.second);
						}
						//tail.blocks[i].pos = new_x;//����Ҫ����λ��
						if (bDebug)	printf("itree=%d,level=%d,path=\"%s\",a=%ld, (x=%ld,node=%ld) \r\n", i_tree, l, p1.c_str(), v.first, x, v.second.node);
						i++;
						if ((l == i_tree) || ((l != i_tree) && (p1 != "")))//������������� ������������������Ҳ��Ǹ��ڵ�
						{
							if (i >= Z_block) break;//�����Ƹ���
						};
						//���ڵ���Բ������޵Ľڵ㣨ʵ����ֻ�к��ٵĽڵ㣩
					}
				}
				for (int j = 0; j < i; j++)
				{
					if ((long long)tail_L.blocks[j].a >= 0)
					{
						stash_L.erase(tail_L.blocks[j].a);//
					}
				}
				unordered_map<string, Tail_L<T>> seed_map_L;//�˴��Ѿ�û���� ��������
				AddPathNodes_L(i_tree, l, x, tail_L, seed_map_L, res, t);
			}


			//for (; i < Z_block; i++)
			//{
			//	RawBlock empty = {0 };
			//	empty.a = -1;//����-1Ϊ��ֵ
			//	empty.data = -1;
			//	empty.data1 = -1;
			//	empty.node = -1;
			//	tail.blocks[i] = empty;
			//}			

		}


		last_x = new_x;

	}
	//s3 = stashs[L - 1].size();//debug
	//printf("stash L size:(%d=>%d=>%d)\r\n", s1, s2, s3);
	if (bDebugShuffle)
	{
		dump(t);
	}
	return t;
}

template<class T>
void LLORAM<T>::Shuffle(ORAMShuffleToken& sf)
{
	request_bytes = 0;
	for (auto v : sf.rebuild)
	{
		//	for (int i = 0; i < Z_block; i++)
		//	{
		////		if (v.bkt.tail.blocks[i].a == 0)
		////		{
		//			//printf(" tree:%d, path:%s, a=%d\r\n", v.tree_height, v.path.c_str(), v.bkt.tail.blocks[i].a);
		////		}
		//	}
		oram[v.tree_height - 1][v.path] = v.bkt;//����ԭ���Ľڵ�

		int headsize=v.bkt.head.maps.size()*sizeof(PointerMapEnc);
		headsize += sizeof(v.bkt.head.mmf);
		int tailsize = v.bkt.etail.size();
		request_bytes += headsize+ tailsize;//������;
	}
	for (auto v : sf.rebuild_sibling)
	{
		//for (int i = 0; i < Z_block; i++)
		//{
		//	//		if (v.bkt.tail.blocks[i].a == 0)
		//	//		{
		//	//printf(" tree:%d, path:%s, a=%d\r\n", v.tree_height, v.path.c_str(), v.bkt.tail.blocks[i].a);
		//	//		}
		//}
		oram[v.tree_height - 1][v.path] = v.bkt;//����ԭ���Ľڵ�

		int headsize = v.bkt.head.maps.size() * sizeof(PointerMapEnc);
		int tailsize = v.bkt.etail.size();
		request_bytes += headsize + tailsize;//������;
	}
}

template<class T>
void LLORAM<T>::access(string op, unsigned long long a, T& data1)
{
	static int count = 0;
	//ÿһ���ڵ�����������2������ ���� 2^L-1���ڵ�
	if ((unsigned long long)a > capacity)//��ַ��0��ʼ���
	{
		printf(" exceed the range, a=%ld", a);
		getchar();
		return;
	}
	//if (bDebug) printf("%s a=%d,data=(obj) (count=%d)\r\n", op.c_str(), a, count++);
	if (bDebug) dump();
	ORAMRequest req = BuildORAMRequest(a);
	ORAMResponse res;
	if (bDebugAccess)
	{
		last_accessed_nodes.clear();
	};
	Search(req, res);
	if (bDebugAccess)
	{

		res_last = res;
	};
	/*dump();*/
	ORAMShuffleToken sf = BuildShuffleToken(op, a, data1, res);
	Shuffle(sf);
	//if (bDebug) dump();
}

template<class T>
void LLORAM<T>::single_round_access(string op, unsigned long long a, T & data1)
{
	//
	static int count = 0;
	//ÿһ���ڵ�����������2������ ���� 2^L-1���ڵ�
	if ((unsigned long long)a > capacity)//��ַ��0��ʼ���
	{
		printf(" exceed the range, a=%ld", a);
		getchar();
		return;
	}
	//if (bDebug) printf("%s a=%d,data=(obj) (count=%d)\r\n", op.c_str(), a, count++);
	if (bDebug) dump();
	ORAMRequest req = BuildORAMRequest(a);
	ORAMResponse response;
	single_round_process_request(req, client_side_shuffle_token_for_single_rounnd, response);
	client_side_shuffle_token_for_single_rounnd = BuildShuffleToken(op, a, data1, response);//���浽�ͻ���
}

template<class T>
void LLORAM<T>::single_round_process_request(ORAMRequest& request, ORAMShuffleToken& shuffle, ORAMResponse& response)
{
	Shuffle(shuffle);//�ȸ����ϴ�һ�ε�
	if (bDebugAccess)
	{
		last_accessed_nodes.clear();
	};
	Search(request, response);
	if (bDebugAccess)
	{
		res_last = response;
	};
}
template<class T>
void LLORAM<T>::single_round_process_request_complete(ORAMShuffleToken& shuffle)
{
	Shuffle(shuffle);//�ȸ�����һ�ε�
}

template<class T>
void LLORAM<T>::single_round_access_complete()//�����һ�εķ���·�����¹��죬������
{
	single_round_process_request_complete(client_side_shuffle_token_for_single_rounnd);
	client_side_shuffle_token_for_single_rounnd.rebuild.clear();
	client_side_shuffle_token_for_single_rounnd.rebuild_sibling.clear();
}
template<class T>
DataBlock LLORAM<T>::ReadDataFromStash(int i_tree, unsigned long long a, bool bSecond)
{
	DataBlock b = { 0 };
	//int aa = a * 2 + bSecond;
	if (stashs[i_tree - 1].find(a) == stashs[i_tree - 1].end())
	{
		b.v = -1;//û�ҵ�
		return b;
	}
	RawBlock ob = stashs[i_tree - 1][a];//i_tree ��1��ʼ���
	//DataBlock ob = stashs[i_tree - 1][aa];//i_tree ��1��ʼ���
	if (bSecond)
	{
		b.v = ob.data1;
	}
	else
	{
		b.v = ob.data;
	}
	return b;
}

template<class T>
T LLORAM<T>::ReadDataFromStash_L(int i_tree, unsigned long long a, bool bSecond)
{
	T b = { 0 };
	//int aa = a * 2 + bSecond;
	if (stash_L.find(a) == stash_L.end())
	{
		return b;
	}
	RawBlock_L<T> ob = stash_L[a];//i_tree ��1��ʼ���
	//DataBlock ob = stashs[i_tree - 1][aa];//i_tree ��1��ʼ���
	if (bSecond)
	{
		b = ob.data1;
	}
	else
	{
		b = ob.data;
	}
	return b;
}

template<class T>
void LLORAM<T>::WriteStashWithNode(int i_tree, unsigned long long a, bool bSecond, DataBlock& data, unsigned long long node)
{
	//int aa = a * 2 + bSecond;
	//DataBlock d;
	//RawBlock b = stashs[i_tree - 1][a];//i_tree ��1��ʼ���
	if (a < 0)
	{
		printf("not support! a==-1\r\n");
		return;
	}
	if (stashs[i_tree - 1].find(a) == stashs[i_tree - 1].end())
	{
		RawBlock block = { 0 };
		block.a = a;
		block.data = -1;
		block.data1 = -1;
		stashs[i_tree - 1][a] = block;
	}
	if (bSecond)
	{
		//b.data = data.v;
		stashs[i_tree - 1][a].data1 = data.v;
	}
	else
	{
		//	b.data1 = data.v;
		stashs[i_tree - 1][a].data = data.v;
	}
	//	CPath32 p;
	//	p.LoadLeaf(i_tree, pos);
	stashs[i_tree - 1][a].node = node;// p.value;//��Ҷ��ת��Ϊ�ڵ���
};

template<class T>
void LLORAM<T>::WriteStashWithNode_L(int i_tree, unsigned long long a, bool bSecond, T& data, unsigned long long node)
{
	//int aa = a * 2 + bSecond;
	//DataBlock d;
	//RawBlock b = stashs[i_tree - 1][a];//i_tree ��1��ʼ���
	if (a < 0)
	{
		printf("not support! a==-1\r\n");
		return;
	}
	if (stash_L.find(a) == stash_L.end())
	{
		RawBlock_L<T> block = { 0 };
		block.a = a;
		stash_L[a] = block;
	}
	if (bSecond)
	{
		//b.data = data.v;
		stash_L[a].data1 = data;
	}
	else
	{
		//	b.data1 = data.v;
		stash_L[a].data = data;
	}
	//	CPath32 p;
	//	p.LoadLeaf(i_tree, pos);
	stash_L[a].node = node;// p.value;//��Ҷ��ת��Ϊ�ڵ���
};

template<class T>
T LLORAM<T>::read(unsigned long long a, bool bSecond)
{
	T data;
	//if (L == 1)//L=1 �����ڿͻ���
	//{
	//	data = ReadDataFromStash(a, bSecond);
	//	return data.v;
	//}	
	access("read", a, data);
	return data;
}

template<class T>
void LLORAM<T>::write(unsigned long long a, bool bSecond, T data)
{
	access("write", a, data);
}

template<class T>
T LLORAM<T>::read(unsigned long long a)
{
	return read(a, false);
}

template<class T>
void LLORAM<T>::write(unsigned long long a, T v)
{
	write(a, false, v);
}

template<class T>
bool LLORAM<T>::ReadBucket(int tree_height, unsigned long long x, int l, E_BucketORAM& b)
{

	//��ʱ��д���ܽ��ܵĴ���

	string p = P(x, l, tree_height);
	if (oram.size() == 0) return false;
	if (oram[tree_height - 1].find(p) == oram[tree_height - 1].end())
	{
		return false;
	}
	b = oram[tree_height - 1][p];
	return true;
}

template<class T>
bool LLORAM<T>::ReadBucketByPath(int tree_height, string path, E_BucketORAM& b)
{
	//��ʱ��д���ܽ��ܵĴ���	
	if (oram.size() == 0) return false;
	if (oram[tree_height - 1].find(path) == oram[tree_height - 1].end())
	{
		return false;
	}
	b = oram[tree_height - 1][path];
	return true;
}

template<class T>
void LLORAM<T>::EncBucketHeader(BucketORAM& bucket, Mem20 key, bool bEncrypt) //
{
	char key1[21] = { 0 };
	memcpy(key1, key.bytes, 20);
	key1[20] = '@';//����һ���������
	if (bEncrypt)//��������һ��
	{
		Header hd;
		hashbytes(key1, 21, sizeof(bucket.head), (void*)&hd);
		XOR((byte*)(&bucket.head), (byte*)&hd, sizeof(hd));
	}
	else
	{//���ܽ��ܶ�һ�����������
		Header hd;
		hashbytes(key1, 21, sizeof(bucket.head), (void*)&hd);
		XOR((byte*)(&bucket.head), (byte*)&hd, sizeof(hd));
	}
}

template<class T>
string LLORAM<T>::P(unsigned long long x, int l, int height)
{
	if ((l > L) || (l < 1))
	{
		if (bDebug) printf("error input!\r\n"); return "";
	};
	bitset<64> s = x;
	string v = s.to_string();
	return v.substr((size_t)(64 - height + 1), (size_t)(l - 1));
}

template<class T>
void LLORAM<T>::dump()
{
	unsigned long long size = 0;
	for (int i = 1; i <= L; i++)
	{
		if (oram[i - 1].size() == 0) continue;
		printf("Tree %d: ", i);
		for (auto v : oram[i - 1])
		{
			printf("\"%s\" ", v.first.c_str());
			Tail t = ToTail(v.second.etail);
			for (int j = 0; j < t.blocks.size(); j++)
			{
				if ((long long)t.blocks[j].a < 0) {
					printf("()"); continue;//����ӡ�սڵ�
				}
				printf("(a=%ld d0=%ld d1=%ld node=%ld)", (long long)t.blocks[j].a, (long long)t.blocks[j].data, (long long)t.blocks[j].data1, t.blocks[j].node);
			};
			printf("; ");
		}
		printf("\r\n");
	}
	for (int i = 1; i <= L; i++)
	{
		if (stashs[i - 1].size() == 0) continue;
		printf("Stash %d: ", i);
		for (auto v : stashs[i - 1])
		{
			printf("L=%ld [a=%ld (%ld,%ld) node=%ld] ", i, v.first, v.second.data, v.second.data1, v.second.node);
		}
		size += stashs[i - 1].size();
		printf("\r\n");
	}
	for (int i = 1; i <= L; i++)
	{
		int tree_item = 0;
		int tree_empty = 0;
		int total = 0;
		for (auto v : oram[i - 1])
		{
			auto v2 = v.second;
			Tail t = ToTail(v2.etail);
			for (int j = 0; j < t.blocks.size(); j++)
			{
				total++;
				if ((long long)t.blocks[j].a >= 0)
				{
					tree_item++;
				}
				else
				{
					tree_empty++;
				}
			}

		}

		printf("tree=%d, item=%ld, empty=%ld,total=%ld,stash=%ld \r\n", i, tree_item, tree_empty, total, stashs[i - 1].size());
	}
	printf("-------------------(stash size=%d)-------------------------------\r\n", size);
}

template<class T>
void LLORAM<T>::dumpTree()
{
	unsigned long long  used = 0;
	unsigned long long emptyblock = 0;

	for (int i = 1; i <= L; i++)
	{
		if (oram[i - 1].size() == 0) continue;
		printf("Tree %d: ", i);
		std::map<string, string> mymap;
		for (auto v : oram[i - 1])
		{
			string kk = "", vv = "";
			kk = string("\"") + v.first + "\"";
			Tail t = ToTail(v.second.etail);
			for (int j = 0; j < t.blocks.size(); j++)
			{
				if ((long long)t.blocks[j].a < 0) {
					emptyblock++;
					vv += "()"; continue;//����ӡ�սڵ�
				}
				char buf[256] = { 0 };
				sprintf(buf, "(a=%ld d0=%d d1=%ld node=%ld)", (long long)t.blocks[j].a, (long long)t.blocks[j].data, (long long)t.blocks[j].data1, t.blocks[j].node);
				vv += buf;
				used += 1;
			};

			mymap[kk] = vv;
			vv += ";";
		}
		for (auto var : mymap)
		{
			printf(" %s%s", var.first.c_str(), var.second.c_str());
		}
		printf("\r\n-------------------(used blocks =%d,empty blocks=%d)-------------------------------\r\n", used, emptyblock);
	}
}

template<class T>
void LLORAM<T>::dumpStashOverflow()
{
	unsigned long long  size = 0;
	for (int i = 1; i <= L; i++)
	{
		if (stashs[i - 1].size() == 0) continue;
		printf("Local Stash Tree=%d: ", i);
		for (auto v : stashs[i - 1])
		{
			printf("L=%d [a=%ld (%ld,%ld) node=%ld] ", i, v.first, v.second.data, v.second.data1, v.second.node);
		}
		size += stashs[i - 1].size();
		printf("\r\n");
	}
	for (int i = 1; i <= L; i++)
	{
		if (oram[i - 1][""].head.maps.size() > Z_block)
		{
			printf("StashOverflow: L=%d [root node size=%ld] ", i, oram[i - 1][""].head.maps.size());
			printf("\r\n");
		}
	}
	printf("-------------------(stash size=%d)-------------------------------\r\n", size);
}

template<class T>
void LLORAM<T>::dumpStash()
{
	unsigned long long  size = 0;
	for (int i = 1; i <= L; i++)
	{
		if (stashs[i - 1].size() == 0) continue;
		printf("Stash %d: ", i);
		for (auto v : stashs[i - 1])
		{
			printf("L=%d [a=%ld (%ld,%ld) node=%ld] ", i, v.first, v.second.data, v.second.data1, v.second.node);
		}
		size += stashs[i - 1].size();
		printf("\r\n");
	}
	for (int i = 1; i <= L; i++)
	{
		if (oram[i - 1][""].head.maps.size() > Z_block)
		{
			printf("L=%d [root node size=%ld] ", i, oram[i - 1][""].head.maps.size());
			printf("\r\n");
		}
	}
	for (int i = 1; i <= L; i++)
	{
		int tree_item = 0;
		int tree_empty = 0;
		int total = 0;
		for (auto v : oram[i - 1])
		{
			auto v2 = v.second;
			Tail t = ToTail(v2.etail);
			for (int j = 0; j < t.blocks.size(); j++)
			{
				total++;
				if ((long long)t.blocks[j].a >= 0)
				{
					tree_item++;
				}
				else
				{
					tree_empty++;
				}
			}

		}
		printf("tree=%d, item=%ld, empty=%d,total=%ld,stash=%d \r\n", i, tree_item, tree_empty, total, stashs[i - 1].size());
	}
	printf("-------------------(stash size=%d)-------------------------------\r\n", size);
}

template<class T>
void LLORAM<T>::dumpfile(string filename)
{
	FILE* fp = fopen(filename.c_str(), "ab+");
	unsigned long long size = 0;
	for (int i = 1; i <= L; i++)
	{
		if (oram[i - 1].size() == 0) continue;
		fprintf(fp, "Tree %d: ", i);
		for (auto v : oram[i - 1])
		{
			fprintf(fp, "\"%s\" ", v.first.c_str());


			Tail t = ToTail(v.second.etail);

			for (int j = 0; j < t.blocks.size(); j++)
			{
				if ((long long)t.blocks[j].a < 0) {
					fprintf(fp, "()"); continue;//����ӡ�սڵ�
				}
				fprintf(fp, "(a=%ld d0=%ld d1=%ld node=%ld)", t.blocks[j].a, (unsigned long long)t.blocks[j].data, (unsigned long long)t.blocks[j].data1, t.blocks[j].node);
			};
			fprintf(fp, "; ");
		}
		fprintf(fp, "\r\n");
	}
	for (int i = 1; i <= L; i++)
	{
		if (stashs[i - 1].size() == 0) continue;
		fprintf(fp, "Stash %d: ", i);
		for (auto v : stashs[i - 1])
		{
			fprintf(fp, "L=%d [a=%ld (%ld,%ld) node=%ld] ", i, v.first, v.second.data, v.second.data1, v.second.node);
		}
		size += stashs[i - 1].size();
		fprintf(fp, "\r\n");
	}
	for (int i = 1; i <= L; i++)
	{
		int tree_item = 0;
		int tree_empty = 0;
		int total = 0;
		for (auto v : oram[i - 1])
		{
			auto v2 = v.second;
			Tail t = ToTail(v2.etail);

			for (int j = 0; j < t.blocks.size(); j++)
			{
				total++;
				if ((long long)t.blocks[j].a >= 0)
				{
					tree_item++;
				}
				else
				{
					tree_empty++;
				}
			}
		}

		fprintf(fp, "tree=%d, item=%ld, empty=%d,total=%d,stash=%d \r\n", i, tree_item, tree_empty, total, stashs[i - 1].size());
	}
	fprintf(fp, "-------------------(stash size=%d)-------------------------------\r\n", size);
	fclose(fp);
}

template<class T>
void LLORAM<T>::dump(ORAMBucketWithPath2& bk)
{
	int c = 0;
	printf("tree=%d path=\"%s\"--------\r\n", bk.tree_height, bk.path.c_str());
	if (bDebugDumpHeader)
	{
		printf("--------header------\r\n");
		//printf("Sf:"); show(bk.bkt.head.Sf); 
		printf(" mmf:"); show(bk.bkt.head.mmf); printf("\r\n");

		for (int i = 0; i < bk.bkt.head.maps.size(); i++)
		{
			show("k1 v1:"); show(bk.bkt.head.maps[i].k1); printf("=>"); show(bk.bkt.head.maps[i].v1); show(",");  show("\r\n");
			show("k2 v2:"); show(bk.bkt.head.maps[i].k2); printf("=>"); show(bk.bkt.head.maps[i].v2); show(",");  show("\r\n");
		}
	}
	printf("-------tail------\r\n");
	Tail t = ToTail(bk.bkt.etail);
	show("M:"); show(t.M); show("\r\n");

	for (int i = 0; i < t.blocks.size(); i++)
	{
		show("a="); show(t.blocks[i].a); show(" data="); show(t.blocks[i].data); show(" data1="); show(t.blocks[i].data1); show(" node="); show(t.blocks[i].node); show("\r\n");
	}
	printf("-------(write-back blocks=%d)------\r\n", t.blocks.size());
}

template<class T>
void LLORAM<T>::dump(ORAMShuffleToken& t)
{
	printf("--------Shuffle------\r\n");
	for (auto v : t.rebuild)
	{
		dump(v);
	}
	for (auto v : t.rebuild_sibling)
	{
		dump(v);
	}
	printf("--------------------\r\n");
}

template<class T>
void LLORAM<T>::dump(ORAMNodeWithPath2& nd)
{
	Tail t = ToTail(nd.etail);
	printf("tree=%d path=%s--------\r\n", nd.tree_height, nd.path.c_str());
	printf("-------tail------\r\n");
	show("M:"); show(t.M); show("\r\n");
	for (int i = 0; i < t.blocks.size(); i++)
	{
		show("a="); show(t.blocks[i].a); show(" data="); show(t.blocks[i].data); show(" data1="); show(t.blocks[i].data1); show(" node="); show(t.blocks[i].node); show("\r\n");
	}
	printf("-----------------\r\n");
}

template<class T>
void LLORAM<T>::dump(ORAMResponse& res)
{
	printf("--------reponse------\r\n");
	printf("accessed leaves (");
	for (auto v : res.leaf_accessed)
	{
		printf("%d, ", v);
	}
	printf(")\r\n");
	printf("accessed nodes:\r\n");
	for (auto nd : res.accessed)
	{
		dump(nd);
	}
	printf("--------------------\r\n");
}

template<class T>
void LLORAM<T>::dumpData()
{
	unordered_map<unsigned long long, unsigned long long> mp;
	for (auto bkt : oram[L - 1])
	{
		Tail t = ToTail(bkt.second.etail);
		for (int i = 0; i < t.blocks.size(); i++)
		{
			if ((long long)t.blocks[i].a < 0) continue;
			unsigned long long d1 = t.blocks[i].data;
			unsigned long long d2 = t.blocks[i].data1;
			if (d1 != -1)
			{
				mp[d1] = 1;
			}
			if (d2 != -1)
			{
				mp[d2] = 1;
			}
		}
	}
	int c = mp.size();
	int size = 0;
	for (int i = 1; i <= L; i++)
	{
		size += stashs[i - 1].size();
	}


	printf("\r\n %d items are inserted!\r\n", c);
	printf("%d items are in the stashs! total stash used:%d\r\n", stashs[L - 1].size(), size);
}

template<class T>
E_BucketORAM LLORAM<T>::GetFather(int i_tree, string path)
{
	E_BucketORAM b;
	if (path == "") return b;
	string path1 = path.substr(0, path.length() - 1);
	for (auto v : oram[i_tree - 1])
	{
		if (v.first == path1)
		{
			return v.second;
		}
	}
}
template<class T>
void LLORAM<T>::dumpM()
{
	int size = 0;
	for (int i = 1; i <= L; i++)
	{
		if (oram[i - 1].size() == 0) continue;
		printf("\r\n Tree %d: \r\n", i);
		for (auto v : oram[i - 1])
		{
			printf("path=\"%s\" \r\n", v.first.c_str());
			printf("MMF: "); show(v.second.head.mmf);
			printf("  M: "); show(ToTail(v.second.etail).M);
			printf("\r\n");
		}
	}
}

template<class T>
vector<byte> LLORAM<T>::ToETail(Tail& t)
{
	vector<byte> b;
	int sizem = sizeof(t.M);
	int insize = sizem + t.blocks.size() * sizeof(RawBlock);
	int outsize;

	char* m = (char*)malloc(insize);

	char* output = (char*)malloc(insize * 2);
	memcpy(m, &t.M, sizem);//����M
	for (int i = 0; i < t.blocks.size(); i++)
	{
		memcpy(m + sizem + i * sizeof(RawBlock), &t.blocks[i], sizeof(RawBlock));
	}
	myAES::Encrypt(m, insize, output, outsize);
	for (int i = 0; i < outsize; i++)
	{
		b.push_back(output[i]);
	}
	free(m);
	free(output);
	return b;
}

template<class T>
vector<byte> LLORAM<T>::ToETail_L(Tail_L<T>& t)
{
	vector<byte> b;
	int sizem = sizeof(t.M);
	int insize = sizem + t.blocks.size() * sizeof(RawBlock_L<T>);
	int outsize;

	char* m = (char*)malloc(insize);

	char* output = (char*)malloc(insize * 2);
	memcpy(m, &t.M, sizem);//����M
	for (int i = 0; i < t.blocks.size(); i++)
	{
		memcpy(m + sizem + i * sizeof(RawBlock_L<T>), &t.blocks[i], sizeof(RawBlock_L<T>));
	}
	myAES::Encrypt(m, insize, output, outsize);
	for (int i = 0; i < outsize; i++)
	{
		b.push_back(output[i]);
	}
	free(m);
	free(output);
	return b;
}

template<class T>
//ת��Ϊβ��
Tail LLORAM<T>::ToTail(vector<byte>& b)
{
	Tail t;

	int insize = b.size();
	char* plaintext = (char*)malloc(b.size());
	int outsize;
	char* m = (char*)malloc(b.size());
	for (int i = 0; i < b.size(); i++)
	{
		m[i] = b[i];
	}
	int sizem = sizeof(t.M);
	myAES::Decrypt(m, insize, plaintext, outsize);
	int outBlock = (outsize - sizem) / sizeof(RawBlock);//
	memcpy(&t.M, plaintext, sizem);
	for (int i = 0; i < outBlock; i++)
	{
		if (t.blocks.size() < outBlock)
		{
			RawBlock r = { 0 };
			t.blocks.push_back(r);//�������
		}
		memcpy(&t.blocks[i], plaintext + sizem + i * sizeof(RawBlock), sizeof(RawBlock));
	}
	free(m);
	free(plaintext);
	return t;
}

template<class T>
//ת��Ϊβ��
Tail_L<T> LLORAM<T>::ToTail_L(vector<byte>& b)
{
	Tail_L<T> t;

	int insize = b.size();
	char* plaintext = (char*)malloc(b.size());
	int outsize;
	char* m = (char*)malloc(b.size());
	for (int i = 0; i < b.size(); i++)
	{
		m[i] = b[i];
	}
	int sizem = sizeof(t.M);
	myAES::Decrypt(m, insize, plaintext, outsize);
	int outBlock = (outsize - sizem) / sizeof(RawBlock_L<T>);//
	memcpy(&t.M, plaintext, sizem);
	for (int i = 0; i < outBlock; i++)
	{
		if (t.blocks.size() < outBlock)
		{
			RawBlock_L<T> r = { 0 };
			t.blocks.push_back(r);//�������
		}
		memcpy(&t.blocks[i], plaintext + sizem + i * sizeof(RawBlock_L<T>), sizeof(RawBlock_L<T>));
	}
	free(m);
	free(plaintext);
	return t;
}

template<class T>
bool LLORAM<T>::DebugCheckContainItem(int i_tree, unsigned long long addr, int b, unsigned long long& value, unsigned long long& node)
{
	for (auto o : oram[i_tree - 1])
	{
		Tail t = ToTail(o.second.etail);
		for (auto v : t.blocks)
		{
			if (v.a == addr)//����û�м���
			{
				node = v.node;
				if (b == 0) value = v.data;
				else value = v.data1;
				return true;
			}
		}
	}
	return false;
}

template<class T>
bool LLORAM<T>::DebugCheckORAMIntegrity(unsigned long long addr)
{
	unsigned long long a;
	int b;
	a = addr;
	unsigned long long node = 0;
	unsigned long long lastNode = 0;
	unsigned long long value = 0;
	bool bError = false;
	for (int i_tree = L; i_tree >= 1; i_tree--)
	{
		//
		b = a % 2;
		a = a / 2;

		bool ret = DebugCheckContainItem(i_tree, a, b, value, node);
		if (ret == false)
		{
			printf("doesn't contain item i_tree=%d addr=%ld (a=%ld,b=%d)\r\n", i_tree, addr, a, b);
			bError = true;
		}
		if (i_tree < L)
		{
			if (value != lastNode)
			{
				printf("value!=lastNode %ld!=%ld i_tree=%d addr=%d (a=%ld,b=%d)\r\n", value, lastNode, i_tree, addr, a, b);
				bError = true;
			}
		}
		lastNode = node;
	}
	if (!bError)
	{
		printf("Check OK! (addr=%ld)\r\n", addr);
		return true;
	}
	else
	{
		printf("errors exist! (addr=%ld)\r\n", addr);
		return false;
	}
}

template<class T>
void LLORAM<T>::DebugCheckNodes()
{
	for (int i = 1; i <= L; i++)
	{
		if (oram[i - 1].size() == 0) continue;
		printf("Check Tree %d: \r\n", i);
		for (auto v : oram[i - 1])
		{
			printf("\"%s\" \r\n", v.first.c_str());
			//for (int j = 0; j < Z_block; j++)
			//{
			if (v.first.length() == 0)
			{
				continue;//�������ڵ㣬����Ҫ���
			}

			//������еĽڵ��Ƿ�������������ѹ��

			Mem20 currentM = ToTail(v.second.etail).M;
			E_BucketORAM father = GetFather(i, v.first);
			Mem20 fatherM = ToTail(father.etail).M;

			Mem20 compute = currentM;
			XOR(compute, fatherM);
			if (memcmp(v.second.head.mmf.bytes, compute.bytes, 20) != 0)
			{
				printf("Error! tree=%d path=%s MMF!=M XOR Mf\r\n", i, v.first.c_str());
				printf("M="); show(currentM);
				printf("Mf="); show(fatherM);
				printf("\r\n");
			}
			else
			{
				printf("check ok!\r\n");
			}
			//printf("(a=%d d0=%d d1=%d node=%d)", v.second.tail.blocks[j].a, (int)v.second.tail.blocks[j].data, (int)v.second.tail.blocks[j].data1, v.second.tail.blocks[j].node);
		//};
		//printf("; ");
		}
		printf("\r\n");
	}
}

template<class T>
void LLORAM<T>::testdump(int L)
{
}

template<class T>
void LLORAM<T>::DumpDebugLastAccessedLeaves()
{
	printf("accessed:[");
	int lastTree = 0;
	for (auto a : res_last.accessed)
	{
		if (lastTree != a.tree_height)
		{
			printf("\r\n tree=%d, ", a.tree_height);
			lastTree = a.tree_height;
		}
		printf("path=\"%s\" ", a.path.c_str());
	}
	printf("\r\n accessed nodes:{");
	for (auto a : last_accessed_nodes)
	{
		printf("%d,", a);
	}
	printf("}\r\n]\r\n");
}


