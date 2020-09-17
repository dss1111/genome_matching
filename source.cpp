/*2014112037 윤재문 */
/*알고리즘 프로젝트*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//메인에서 Genome길이, shortread 길이 갯수, Reference Genome을 만들때 mismatch갯수를 입력하여 프로그램 동작//
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
	int shortread_Length;	//shortread 길이
	float t;
public:
	MyGenome();	//생성자. 해시테이블 생성및 초기화
	void makeMyGenome(int); //MyGenome을 만들어 txt파일에 저장
	void readMyGenome();	//txt파일을 읽어 MG벡터에 저장
	void makeReferenceGenome(int);	//MG벡터를 복사한 RG의 랜덤한 위치 미스매치생성
	void makeShortRead(int, int);	//shortread 을 원하는 길이, 원하는 수만큼 생성
	void makeHashTable(string);	// shortread를 해싱해서 해시테이블에 연결
	void matchingBF();	//BruteForce로 해싱테이블의 shortread들 매칭
	void matchingBM();	//BoyerMoore로 해싱테이블의 shortread들 매칭
	void matchingKMP();	//KMP로 해싱테이블의 shortread들 매칭
	void bruteForce(string);//Bruteforce 알고리즘 사용
	void boyerMoore(string);//BoyerMoore 알고리즘 사용
	void kmp(string);	//kmp알고리즘 사용
	void makeBCT(string, int *);	//Bad character 테이블 생성
	void makeGST(string, int *, int *);	//Good suffix 테이블 생성
	int getBCT(char);	//문자의 BCT테이블의 위치반환
	void makePT(string, int *);	//kmp에 사용하는 PatternTable 생성
	void result();	//결과출력
	void writeVec(string Name);
	void makeNG(string);
};
MyGenome::MyGenome()
{
	/*생성자*/
	/*해시 테이블 생성및 초기화*/
	/*AAA를 0 TTT를 63으로 해시 테이블 생성*/
	HashTable = new hash_element *[64];	//4*4*4
	for (int i = 0; i < 64; i++)
	{
		HashTable[i] = NULL;
	}
}
void MyGenome::makeMyGenome(int length)
{
	/*input: int length */
	/*파라미터로 받은 길이만큼 메르센 트위스터를 사용하여 랜덤으로 시퀸스생성*/
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
	/*텍스트 파일로부터 Genome을 읽어온다*/

	FILE * read = fopen("MG.txt", "r");
	char buf[1025]; //한줄만큼 읽어온 문자열을 저장할 버퍼
	char *bufp;
	string str;
	while (!feof(read))
	{
		bufp = fgets(buf, sizeof(buf), read);
		str = str + buf;
		MG.push_back(str);	//MG벡터에 한줄 저장
		str = "";
	}
	fclose(read);
}
void MyGenome::makeReferenceGenome(int misnum)
{
	/*input: misnum*/
	/*misnum만큼 mismatch생성*/
	/*위치 중복이 가능해서 실제 mismatch수는 misnum보다 작음*/

	/*MG를 복사해서 RG를 만든다*/
	RG.resize(MG.size());
	copy(MG.begin(), MG.end(), RG.begin());

	/*난수생성하여 ACGT중 랜덤하게 랜덤한 줄 랜덤한 행의 값을 바꾼다*/
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
	/*RG를 복사한 NG생성*/
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
	/*length의 길이를지닌 num개의 shortread를 만든다*/


	shortread_Length = length;	//length = ShortRead 길이  num = ShortRead 수 
	int line, col;


	/*난수생성을 이용 랜덤한 줄의 열부터 length만큼 잘라 shortread를 만든다*/
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
		makeHashTable(shortread);	//만든 shortread는 해시테이블에 저장
		file << shortread << endl;
	}
	file.close();

}
void MyGenome::makeHashTable(string shortread)
{
	/*input: string shortread*/
	/*shortread를 받아 해시테이블에 저장*/
	int hash=0;
	int digit=1;

	/*AAA=0 ~ TTT=63까지 해시테이블 위치 계산*/
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

	/*shortread를 담은 구조체 생성*/
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
		/*링크드 리스트와 동일한구조*/
		hash_element * tail = HashTable[hash];
		while (tail->next != NULL)//끝까지 이동	
		{
			if (tail->shortread == shortread)	//이미 존재하는 shortread이면
			{
				exist = true;	//존재한다 체크
				break;
			}
			tail = tail->next;
		}
		if (exist == false)	//존재하지 않으면
		{
			tail->next = element; 	//끝에 연결
		}
		else//존재하면
		{
			delete element; //연결하지 않고 제거	
		}
	}
}
void MyGenome::matchingBF()
{
	/*Brute force알고리즘 사용*/
	/*해시 테이블에 들어있는 shortread를 차례로 알고리즘에 대입*/
	/*수행시간 체크*/
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
	std::cout << "BF수행시간: " << t / CLOCKS_PER_SEC << endl;
	makeNG("BF");
	writeVec("bf");
}
void MyGenome::matchingBM()
{
	/*Boyer moore 알고리즘 사용*/
	/*해시 테이블에 들어있는 shortread를 차례로 알고리즘에 대입*/
	/*수행시간 체크*/
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
	std::cout << "BM수행시간: " << t / CLOCKS_PER_SEC << endl;
	makeNG("BM");
	writeVec("bm");
}
void MyGenome::matchingKMP()
{
	/*kmp 알고리즘 사용*/
	/*해시 테이블에 들어있는 shortread를 차례로 알고리즘에 대입*/
	/*수행시간 체크*/
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
	std::cout << "KMP수행시간: " << t / CLOCKS_PER_SEC << endl;
	makeNG("KMP");
	writeVec("kmp");
}
void MyGenome::kmp(string shortread)
{
	/*input: string shortread*/
	int * PT = new int[shortread_Length];	//패턴테이블 동적할당
	int i, j,k;
	string temp;
	int count = 0;
	int mis = 0;
	makePT(shortread,PT);	//패턴테이블을 만든다
	for (k = 0; k < RG.size(); k++)
	{
		temp = RG.at(k);
		for(i=0;i < temp.length()-shortread_Length;)
		{
			j = 0;
			mis = 0;
			while (j < shortread_Length)
			{
				if (shortread[j] == temp[i + j])	//해당 문자가 일치
				{
					j++;	//다음은 앞의문자를 비교
				}
				else if (mis == 0)	//일치하지 않는데 mismatch가 지금까지 없는경우
				{
					j++;
					mis++;
				}
				else //mismatch가 2개이상
					break;
			}
			/*
			while (j >= 0 && temp[i] != shortread[j])
			{
				if (mis == 0 )	// 미스매치가 0개면
				{
					mis++;
					i++;
					j++;
					if (j == shortread_Length)
					{
						j = shortread_Length-1;
					}
				}
				else if(mis==1)//미스매치가 1개
				{
					j = PT[j];	//패턴테이블 사용 인덱스이동
				}
				else
				{
					break;
				}
				j = PT[j];
			}
			*/
			if (j ==shortread_Length)	//미스매치1개이내 일치
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
						NG.at(k) = temp.replace(i, shortread_Length, shortread);	//NG문자열 수정
						location = location + "를 " + shortread + "로 바꿈" + std::to_string(k + 1) + "줄" + std::to_string(i+1) + "번째";
						kmp_result.push_back(location);
						i = i + shortread_Length-1;//수정된 다음인덱스부터 매칭을이어감
					}
				}
				if (mis == 2)
				{
					j = PT[j];
				}
				i++;
			}
			else//불일치
			{
					i++;
			}
		}
	}
}
void MyGenome::makePT(string shortread, int * PT)
{
	/*input: string shortread   int * PT */
	/*shortread를 받아서 패턴테이블 생성*/
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
	/*bruteforce알고리즘*/
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
				if (temp[j+k] == shortread[k])//일치
				{
					count++;
				}
				else //불일치 
				{
					mis++;
					count++;
				}
				if (mis > 1)//미스매치가 2개이상
				{
					break;
				}
				if (count == shortread_Length && (j+shortread_Length<NG.at(i).length())) //미스매치1개이내 일치
				{
					string location;
					for (int g = 0; g < shortread_Length; g++)
					{
						location += NG.at(i)[j + g];
					}
					if (location != shortread)
					{
						NG.at(i) = temp.replace(j, shortread_Length, shortread); //NG문자열 수정
						location = location + "를 " + shortread + "로 바꿈" + std::to_string(i + 1) + "줄" + std::to_string(j+1) + "번째";
						bf_result.push_back(location);
						j = j + shortread_Length - 1;//수정된 다음인덱스부터 다시매칭
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
	/*boyermoore알고리즘*/
	int BCT[4];		//ACGT 4가지종류이므로 [4]
	int *suffix = new int[shortread_Length+1];
	int *GST = new int[shortread_Length+1];
	int i, j,k;
	/*테이블 초기화*/
	for (k = 0; k < shortread_Length+1; k++)
	{
		GST[k] = 0;
		suffix[k] = 0;
	}
	int mis;
	int max;
	makeBCT(shortread,BCT);	//나쁜 문자테이블 생성
	makeGST(shortread,suffix,GST);//착한 접미부 테이블 생성
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
				if (shortread[j] == temp[i + j])	//해당 문자가 일치
				{
					j--;	//다음은 앞의문자를 비교
				}
				else if (mis == 0)	//일치하지 않는데 mismatch가 지금까지 없는경우
				{
					j--;
					mis++;
				}
				else //mismatch가 2개이상
					break;
					
			}
			
			if (j < 0)	//미스매치1개이내 일치
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
						NG.at(k) = temp.replace(i, shortread_Length, shortread);	//NG문자열 수정
						location = location + "를 " + shortread + "로 바꿈" + std::to_string(k + 1) + "줄" + std::to_string(i+1) + "번째";
						bm_result.push_back(location);
						i = i + shortread_Length-1;//수정된 다음인덱스부터 매칭을이어감
					}
				}
				i++;
			}
			else//불일치
			{
				/*나쁜문자테이블과 착한접미부테이블중 더 멀리점프하는것만큼 이동*/
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
	/*나쁜 문자 테이블*/
	int i, j;
	for (i = 0; i < 4; i++)
	{
		BCT[i] = -1;	//테이블 -1로 초기화
	}
	/*BCT[0~3]은 각각 A,C,G,T를 뜻한다 */
	/*테이블에 입력된 j값은 불일치할때 이동거리를 나타낸다*/
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
	/*착한 접미부 테이블 생성*/
	int i = shortread_Length;
	int j = shortread_Length + 1;
	suffix[i] = j;	//테이블의 끝에 패턴(shortread)길이 + 1 넣음
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
		if (GST[i] == 0)	//테이블 값이 0이면
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
	/*문자를 받아 해당 문자에 해당하는 BCT테이블 위치를 반환*/
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
	/*결과 출력*/
	/*Reference genome과 New Genome을 비교해서 일치율을 출력*/
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
	cout <<"일치율" <<same*100 / (same + notsame) <<"%\n" ;
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