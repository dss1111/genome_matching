/*2014112037 ���繮 */
/*�˰��� ������Ʈ*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//���ο��� Genome����, shortread ���� ����, Reference Genome�� ���鶧 mismatch������ �Է��Ͽ� ���α׷� ����//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <Windows.h>
#include <vector>
#include <string>
#include <random>
#include <ctime>

using namespace std;


struct hash_element
{
	string shortread;
	hash_element * next;
};

class MyGenome
{
private:
	vector <string> MG;	//My Genome
	vector <string> RG;	//Reference Genome
	vector <string> NG; //New Genome
	vector <string> bf_result;
	vector <string> kmp_result;
	vector <string> bm_result;
	hash_element ** HashTable;	//Hashed ShortRead Table
	int shortread_Length;	//shortread ����
	float t;
public:
	MyGenome();	//������. �ؽ����̺� ������ �ʱ�ȭ
	void makeMyGenome(int); //MyGenome�� ����� txt���Ͽ� ����
	void readMyGenome();	//txt������ �о� MG���Ϳ� ����
	void makeReferenceGenome(int);	//MG���͸� ������ RG�� ������ ��ġ �̽���ġ����
	void makeShortRead(int, int);	//shortread �� ���ϴ� ����, ���ϴ� ����ŭ ����
	void makeHashTable(string);	// shortread�� �ؽ��ؼ� �ؽ����̺� ����
	void matchingBF();	//BruteForce�� �ؽ����̺��� shortread�� ��Ī
	void matchingBM();	//BoyerMoore�� �ؽ����̺��� shortread�� ��Ī
	void matchingKMP();	//KMP�� �ؽ����̺��� shortread�� ��Ī
	void bruteForce(string);//Bruteforce �˰��� ���
	void boyerMoore(string);//BoyerMoore �˰��� ���
	void kmp(string);	//kmp�˰��� ���
	void makeBCT(string, int *);	//Bad character ���̺� ����
	void makeGST(string, int *, int *);	//Good suffix ���̺� ����
	int getBCT(char);	//������ BCT���̺��� ��ġ��ȯ
	void makePT(string, int *);	//kmp�� ����ϴ� PatternTable ����
	void result();	//������
	void writeVec(string Name);
	void makeNG(string);
};
MyGenome::MyGenome()
{
	/*������*/
	/*�ؽ� ���̺� ������ �ʱ�ȭ*/
	/*AAA�� 0 TTT�� 63���� �ؽ� ���̺� ����*/
	HashTable = new hash_element *[64];	//4*4*4
	for (int i = 0; i < 64; i++)
	{
		HashTable[i] = NULL;
	}
}
void MyGenome::makeMyGenome(int length)
{
	/*input: int length */
	/*�Ķ���ͷ� ���� ���̸�ŭ �޸��� Ʈ�����͸� ����Ͽ� �������� ����������*/
	ofstream file("MG.txt");
	int k;
	random_device rn;
	mt19937_64 rnd(rn());
	uniform_int_distribution<int> range(0, 3);
	for (int i = 0; i < length; i++)
	{
		k = range(rnd);
		if (k == 0)
		{
			file << 'A';
		}
		else if (k == 1)
		{
			file << 'C';
		}
		else if (k == 2)
		{
			file << 'G';
		}
		else
			file << 'T';
	}
	file.close();
}
void MyGenome::readMyGenome()
{
	/*�ؽ�Ʈ ���Ϸκ��� Genome�� �о�´�*/

	FILE * read = fopen("MG.txt", "r");
	char buf[1025]; //���ٸ�ŭ �о�� ���ڿ��� ������ ����
	char *bufp;
	string str;
	while (!feof(read))
	{
		bufp = fgets(buf, sizeof(buf), read);
		str = str + buf;
		MG.push_back(str);	//MG���Ϳ� ���� ����
		str = "";
	}
	fclose(read);
}
void MyGenome::makeReferenceGenome(int misnum)
{
	/*input: misnum*/
	/*misnum��ŭ mismatch����*/
	/*��ġ �ߺ��� �����ؼ� ���� mismatch���� misnum���� ����*/

	/*MG�� �����ؼ� RG�� �����*/
	RG.resize(MG.size());
	copy(MG.begin(), MG.end(), RG.begin());

	/*���������Ͽ� ACGT�� �����ϰ� ������ �� ������ ���� ���� �ٲ۴�*/
	int line, col, k;
	random_device rn;
	mt19937_64 rnd(rn());
	uniform_int_distribution<int> a(0, 3);
	uniform_int_distribution<int> l(0, RG.size()-1);
	uniform_int_distribution<int> c(0, 1023);
	int linecount = RG.size() - 1;
	for (int i = 0; i < misnum; i++)
	{
		k = a(rnd);
		line = l(rnd);
		col = c(rnd);
		if (line == linecount && col >RG.at(linecount).length())
		{
			i--;
		}
		else
		{
			if (k == 0)
			{
				RG.at(line)[col] = 'A';
			}
			else if (k == 1)
			{
				RG.at(line)[col] = 'C';
			}
			else if (k == 2)
			{
				RG.at(line)[col] = 'G';
			}
			else
			{
				RG.at(line)[col] = 'T';
			}
		}
	}
	/*RG�� ������ NG����*/
	NG.resize(RG.size());
	copy(RG.begin(), RG.end(),NG.begin());
	ofstream file("RG.txt");
	for (int i = 0; i < RG.size(); i++)
	{
		for(int j = 0; j < RG.at(i).length(); j++)
		{
			file << RG.at(i)[j];
		}
		file << endl;
	}
	file.close();
}
void MyGenome::makeShortRead(int length, int num)
{
	/*input: int length , int num*/
	/*length�� ���̸����� num���� shortread�� �����*/


	shortread_Length = length;	//length = ShortRead ����  num = ShortRead �� 
	int line, col;


	/*���������� �̿� ������ ���� ������ length��ŭ �߶� shortread�� �����*/
	random_device rn;
	mt19937_64 rnd(rn());
	uniform_int_distribution<int> l(0, RG.size() - 1);
	uniform_int_distribution<int> c(0, 1023);
	string shortread;
	ofstream file("SR.txt");
	for (int i = 0; i < num; i++)
	{
		line = l(rnd);
		col = c(rnd);
		if (line == RG.size() - 1 && (col+length)>RG.at(RG.size()-1).length())
		{
			i--;
		}
		else
		{
			if ((col + length) > RG.at(line).length())
			{
				shortread = RG.at(line).substr(col);
				shortread = shortread + RG.at(line + 1).substr(0, length - shortread.length());
			}
			else 
				shortread = RG.at(line).substr(col, length);
		}
		makeHashTable(shortread);	//���� shortread�� �ؽ����̺� ����
		file << shortread << endl;
	}
	file.close();

}
void MyGenome::makeHashTable(string shortread)
{
	/*input: string shortread*/
	/*shortread�� �޾� �ؽ����̺� ����*/
	int hash=0;
	int digit=1;

	/*AAA=0 ~ TTT=63���� �ؽ����̺� ��ġ ���*/
	for (int i = 2; i >=0; i--)
	{
		if (shortread[i] == 'C')
		{
			hash = hash + 1 * digit;
		}
		else if (shortread[i] == 'G')
		{
			hash = hash + 2 * digit;
		}
		else if (shortread[i] == 'T')
		{
			hash = hash + 3 * digit;
		}
		digit = digit * 4;
	}
	bool exist=false;

	/*shortread�� ���� ����ü ����*/
	hash_element * element;
	element = new hash_element;
	element->shortread = shortread;
	element->next = NULL;
	if (HashTable[hash]==NULL)
	{
		HashTable[hash] = element;
	}
	else
	{
		/*��ũ�� ����Ʈ�� �����ѱ���*/
		hash_element * tail = HashTable[hash];
		while (tail->next != NULL)//������ �̵�	
		{
			if (tail->shortread == shortread)	//�̹� �����ϴ� shortread�̸�
			{
				exist = true;	//�����Ѵ� üũ
				break;
			}
			tail = tail->next;
		}
		if (exist == false)	//�������� ������
		{
			tail->next = element; 	//���� ����
		}
		else//�����ϸ�
		{
			delete element; //�������� �ʰ� ����	
		}
	}
}
void MyGenome::matchingBF()
{
	/*Brute force�˰��� ���*/
	/*�ؽ� ���̺� ����ִ� shortread�� ���ʷ� �˰��� ����*/
	/*����ð� üũ*/
	t = clock();
	hash_element * temp;
	for (int i = 0; i < 64; i++)
	{
		temp = HashTable[i];
		while (temp != NULL)
		{
			bruteForce(temp->shortread);
			temp = temp->next;
		}
	}
	t = clock() - t;
	std::cout << "BF����ð�: " << t / CLOCKS_PER_SEC << endl;
	makeNG("BF");
	writeVec("bf");
}
void MyGenome::matchingBM()
{
	/*Boyer moore �˰��� ���*/
	/*�ؽ� ���̺� ����ִ� shortread�� ���ʷ� �˰��� ����*/
	/*����ð� üũ*/
	t = clock();
	hash_element * temp;
	for (int i = 0; i < 64; i++)
	{
		temp = HashTable[i];
		while (temp != NULL)
		{
			boyerMoore(temp->shortread);
			temp = temp->next;
		}
	}
	t = clock() - t;
	std::cout << "BM����ð�: " << t / CLOCKS_PER_SEC << endl;
	makeNG("BM");
	writeVec("bm");
}
void MyGenome::matchingKMP()
{
	/*kmp �˰��� ���*/
	/*�ؽ� ���̺� ����ִ� shortread�� ���ʷ� �˰��� ����*/
	/*����ð� üũ*/
	t = clock();
	hash_element * temp;
	for (int i = 0; i < 64; i++)
	{
		temp = HashTable[i];
		while (temp != NULL)
		{
			kmp(temp->shortread);
			temp = temp->next;
		}
	}
	t = clock() - t;
	std::cout << "KMP����ð�: " << t / CLOCKS_PER_SEC << endl;
	makeNG("KMP");
	writeVec("kmp");
}
void MyGenome::kmp(string shortread)
{
	/*input: string shortread*/
	int * PT = new int[shortread_Length];	//�������̺� �����Ҵ�
	int i, j,k;
	string temp;
	int count = 0;
	int mis = 0;
	makePT(shortread,PT);	//�������̺��� �����
	for (k = 0; k < RG.size(); k++)
	{
		temp = RG.at(k);
		for(i=0;i < temp.length()-shortread_Length;)
		{
			j = 0;
			mis = 0;
			while (j < shortread_Length)
			{
				if (shortread[j] == temp[i + j])	//�ش� ���ڰ� ��ġ
				{
					j++;	//������ ���ǹ��ڸ� ��
				}
				else if (mis == 0)	//��ġ���� �ʴµ� mismatch�� ���ݱ��� ���°��
				{
					j++;
					mis++;
				}
				else //mismatch�� 2���̻�
					break;
			}
			/*
			while (j >= 0 && temp[i] != shortread[j])
			{
				if (mis == 0 )	// �̽���ġ�� 0����
				{
					mis++;
					i++;
					j++;
					if (j == shortread_Length)
					{
						j = shortread_Length-1;
					}
				}
				else if(mis==1)//�̽���ġ�� 1��
				{
					j = PT[j];	//�������̺� ��� �ε����̵�
				}
				else
				{
					break;
				}
				j = PT[j];
			}
			*/
			if (j ==shortread_Length)	//�̽���ġ1���̳� ��ġ
			{
				if ((i + shortread_Length < NG.at(k).length()))
				{
					string location;
					for (int g = 0; g < shortread_Length; g++)
					{
						location+=NG.at(k)[i + g];
					}
					if (location != shortread)
					{
						NG.at(k) = temp.replace(i, shortread_Length, shortread);	//NG���ڿ� ����
						location = location + "�� " + shortread + "�� �ٲ�" + std::to_string(k + 1) + "��" + std::to_string(i+1) + "��°";
						kmp_result.push_back(location);
						i = i + shortread_Length-1;//������ �����ε������� ��Ī���̾
					}
				}
				if (mis == 2)
				{
					j = PT[j];
				}
				i++;
			}
			else//����ġ
			{
					i++;
			}
		}
	}
}
void MyGenome::makePT(string shortread, int * PT)
{
	/*input: string shortread   int * PT */
	/*shortread�� �޾Ƽ� �������̺� ����*/
	int i = 0, j = -1;
	PT[0] = -1;
	/*make pattern table*/
	while (i<shortread_Length)
	{
		while (j > -1 && shortread[i] != shortread[j])
		{
			j = PT[j];
		}
		i++;
		j++;
		PT[i] = j;
	}
}
void MyGenome::bruteForce(string shortread)
{
	/*input: string shortread*/
	/*bruteforce�˰���*/
	int count = 0;
	int flag = 0;
	int i, j, k;
	int mis;
	long index= -1;
	string temp;
	for (i = 0; i < RG.size();i++)
	{
		temp = RG.at(i);
		for (j = 0; j < temp.length()-shortread.size(); j++)
		{
			index++;
			mis = 0;
			count = 0;
			for (k = 0; k < shortread_Length; k++)
			{
				if (temp[j+k] == shortread[k])//��ġ
				{
					count++;
				}
				else //����ġ 
				{
					mis++;
					count++;
				}
				if (mis > 1)//�̽���ġ�� 2���̻�
				{
					break;
				}
				if (count == shortread_Length && (j+shortread_Length<NG.at(i).length())) //�̽���ġ1���̳� ��ġ
				{
					string location;
					for (int g = 0; g < shortread_Length; g++)
					{
						location += NG.at(i)[j + g];
					}
					if (location != shortread)
					{
						NG.at(i) = temp.replace(j, shortread_Length, shortread); //NG���ڿ� ����
						location = location + "�� " + shortread + "�� �ٲ�" + std::to_string(i + 1) + "��" + std::to_string(j+1) + "��°";
						bf_result.push_back(location);
						j = j + shortread_Length - 1;//������ �����ε������� �ٽø�Ī
					}
					j++;
				}
			}
		}
		index += shortread_Length;
	}
	return;
}
void MyGenome::boyerMoore(string shortread)
{
	/*input: string shortread*/
	/*boyermoore�˰���*/
	int BCT[4];		//ACGT 4���������̹Ƿ� [4]
	int *suffix = new int[shortread_Length+1];
	int *GST = new int[shortread_Length+1];
	int i, j,k;
	/*���̺� �ʱ�ȭ*/
	for (k = 0; k < shortread_Length+1; k++)
	{
		GST[k] = 0;
		suffix[k] = 0;
	}
	int mis;
	int max;
	makeBCT(shortread,BCT);	//���� �������̺� ����
	makeGST(shortread,suffix,GST);//���� ���̺� ���̺� ����
	string temp;
	for (k = 0; k<RG.size(); k++)
	{
		temp = RG.at(k);
		for (i=0;i < temp.length() - shortread_Length;)
		{
			j = shortread_Length - 1;
			mis = 0;
			/*
			while (j >= 0 && shortread[j] == temp[i + j])
			{
				j--;
			}
			*/
			
			while (j >= 0)
			{
				if (shortread[j] == temp[i + j])	//�ش� ���ڰ� ��ġ
				{
					j--;	//������ ���ǹ��ڸ� ��
				}
				else if (mis == 0)	//��ġ���� �ʴµ� mismatch�� ���ݱ��� ���°��
				{
					j--;
					mis++;
				}
				else //mismatch�� 2���̻�
					break;
					
			}
			
			if (j < 0)	//�̽���ġ1���̳� ��ġ
			{	
				string location;
				if ((i + shortread_Length < NG.at(k).length()))
				{
					for (int g = 0; g < shortread_Length; g++)
					{
						location+=NG.at(k)[i + g];
					}
					if (location != shortread)
					{
						NG.at(k) = temp.replace(i, shortread_Length, shortread);	//NG���ڿ� ����
						location = location + "�� " + shortread + "�� �ٲ�" + std::to_string(k + 1) + "��" + std::to_string(i+1) + "��°";
						bm_result.push_back(location);
						i = i + shortread_Length-1;//������ �����ε������� ��Ī���̾
					}
				}
				i++;
			}
			else//����ġ
			{
				/*���۹������̺�� �������̺����̺��� �� �ָ������ϴ°͸�ŭ �̵�*/
				if (GST[j + 1] > j - BCT[ getBCT(temp[i + j])])
				{
					max = GST[j + 1];
				}
				else 
				{
					max = j - BCT[getBCT(temp[i + j])];
				}
				i += max;
			}
		}
	}
	delete[] suffix;
	delete[] GST;
}
void MyGenome::makeBCT(string shortread, int * BCT)
{
	/*input: string shortread  int * BCT*/
	/*���� ���� ���̺�*/
	int i, j;
	for (i = 0; i < 4; i++)
	{
		BCT[i] = -1;	//���̺� -1�� �ʱ�ȭ
	}
	/*BCT[0~3]�� ���� A,C,G,T�� ���Ѵ� */
	/*���̺� �Էµ� j���� ����ġ�Ҷ� �̵��Ÿ��� ��Ÿ����*/
	for (j = 0; j < shortread_Length; j++)
	{
		if (shortread[j] == 'A')
		{
			BCT[0] = j;
		}
		else if (shortread[j] == 'C')
		{
			BCT[1] = j;
		}
		else if (shortread[j] == 'G')
		{
			BCT[2] = j;
		}
		else
		{
			BCT[3] = j;
		}
	}
}
void MyGenome::makeGST(string shortread, int * suffix, int * GST)
{
	/*input: string shortread int *suffix int * GST */
	/*���� ���̺� ���̺� ����*/
	int i = shortread_Length;
	int j = shortread_Length + 1;
	suffix[i] = j;	//���̺��� ���� ����(shortread)���� + 1 ����
	while (i > 0)
	{
		while (j <= shortread_Length && shortread[i - 1] != shortread[j - 1])
		{
			if (GST[j] == 0)
			{
				GST[j] = j - 1;
			}
			j = suffix[j];
		}
		i--;
		j--;
		suffix[i] = j;
	}

	j = suffix[0];
	for (i = 0; i <= shortread_Length; i++)
	{
		if (GST[i] == 0)	//���̺� ���� 0�̸�
		{
			GST[i] = j;
		}
		if (i == j)
		{
			j = suffix[j];
		}
	}
}
int MyGenome::getBCT(char a)
{
	/*input: char a*/
	/*���ڸ� �޾� �ش� ���ڿ� �ش��ϴ� BCT���̺� ��ġ�� ��ȯ*/
	if (a == 'A')
	{
		return 0;
	}
	else if (a == 'C')
	{
		return 1;
	}
	else if (a == 'G')
	{
		return 2;
	}
	else
	{
		return 3;
	}
}
void MyGenome::result()
{
	/*��� ���*/
	/*Reference genome�� New Genome�� ���ؼ� ��ġ���� ���*/
	int i, j;
	float same=0;
	float notsame=0;
	float percent;
	for (i = 0; i < MG.size(); i++)
	{
		for (j = 0; j < MG.at(i).length(); j++)
		{
			if (MG.at(i)[j] == NG.at(i)[j])
			{
				same++;
			}
			else
				notsame++;
		}
	}
	cout << "mismatch:" << notsame << endl;
	cout <<"��ġ��" <<same*100 / (same + notsame) <<"%\n" ;
	copy(RG.begin(), RG.end(), NG.begin());
}
void MyGenome::writeVec(string name)
{
	int i, j;
	if (name == "kmp")
	{
		ofstream file("kmp_loc.txt");
		for (i = 0; i < kmp_result.size(); i++)
		{
			file << kmp_result.at(i);
			file << endl;
		}
		file.close();
	}
	else if(name=="bm")
	{
		ofstream file("bm_loc.txt");
		for (i = 0; i < bm_result.size(); i++)
		{
			file << bm_result.at(i);
			file << endl;
		}
		file.close();
	}
	else
	{
		ofstream file("bf_loc.txt");
		for (i = 0; i < bf_result.size(); i++)
		{
			file << bf_result.at(i);
			file << endl;
		}
		file.close();
	}

}
void MyGenome::makeNG(string name)
{
	ofstream file(name+"_NG.txt");
	int i, j;
	for (i = 0; i < NG.size(); i++)
	{
		for (j = 0; j < NG.at(i).length(); j++)
		{
			file << NG.at(i)[j];
		}
		file << endl;
	}
	file.close();
}
int main()
{
	class MyGenome A;
	A.makeMyGenome(10000);			// genoem length
	A.readMyGenome();
	A.makeReferenceGenome(10);		// how many diffrent value be made
	A.makeShortRead(20, 1000);		// shortread length and number of shortread
	A.result();
	A.matchingBF();
	A.result();
	A.matchingKMP();
	A.result();
	A.matchingBM();
	A.result();
}