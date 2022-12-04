#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include <assert.h>
#include <map>

std::wstring GBKToUNICODE(const std::string& str)
{
	std::	wstring strTo;
	int		iTextLen;
	// wide char to multi char

	iTextLen = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), NULL, NULL);
	if (iTextLen > 0)
	{
		strTo.resize(iTextLen);
		::MultiByteToWideChar(CP_ACP,
			0,
			str.c_str(),
			(int)str.size(),
			(wchar_t*)strTo.c_str(),
			(int)strTo.size());
	}
	return strTo;
}

std::string UnicodeToANSI(const std::wstring& str)
{
	char* pElementText;
	int    iTextLen;
	// wide char to multi char
	iTextLen = WideCharToMultiByte(CP_ACP,
		0,
		str.c_str(),
		-1,
		NULL,
		0,
		NULL,
		NULL);
	pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, sizeof(char) * (iTextLen + 1));
	::WideCharToMultiByte(CP_ACP,
		0,
		str.c_str(),
		-1,
		pElementText,
		iTextLen,
		NULL,
		NULL);
	std::string strText;
	strText = pElementText;
	delete[] pElementText;
	return strText;
}

typedef std::vector<std::string> DataVec;
typedef std::vector<std::wstring> WDataVec;
typedef std::map<int, DataVec> DataLenMap;

void UnorderData(std::wstring& wstr)
{
	int a = rand() % wstr.size();
	int b = rand() % wstr.size();
	std::swap(wstr[a], wstr[b]);
}

void RepeatData(std::wstring& wstr)
{
	int a = rand() % wstr.size();
	int b = rand() % wstr.size();
	wstr[a] = wstr[b];
}

void CutData(std::wstring& wstr)
{
	int a = rand() % wstr.size();

	std::wstring result;
	for (int i = 0; i < (int)wstr.size(); i++)
	{
		if (i != a)
			result += wstr[i];
	}
}

void OffsetData(std::wstring& wstr)
{
	int a = rand() % wstr.size();
	wstr[a]++;
}

void Load(DataVec& rDataVec, DataVec& rTestData, DataLenMap& rTestDataMap, int nTestNum)
{
	FILE* pFile = fopen("itemname.txt", "r");
	if (!pFile)
		return;
	char szBuffer[128];
	while (fgets(szBuffer, sizeof(szBuffer), pFile))
	{
		size_t uLen = strlen(szBuffer);
		if (uLen > 0 && szBuffer[uLen - 1] == '\n')
			szBuffer[uLen - 1] = '\0';

		rDataVec.push_back(szBuffer);
	}

	srand(0);
	while (rTestData.size() < nTestNum)
	{
		std::wstring patter = GBKToUNICODE(rDataVec[rand() % rDataVec.size()]);
		size_t uPatterLen = patter.size();

		if (patter.size() > 3)
		{
			patter = patter.substr(rand() % (uPatterLen / 2));
		}

		if (patter.size() > 3)
		{
			size_t uCutLen = rand() % patter.size();
			size_t uLen = 0;
			if (patter.size() > uCutLen)
				uLen = patter.size() - uCutLen;
			if (uLen < 2)
				uLen = 2;
			patter = patter.substr(0, uLen);
		}

		if (patter.size() <= 0)
			continue;

		assert(patter.size() > 0);

		if (rand() % 20 == 0)
			UnorderData(patter);
		if (rand() % 20 == 0)
			RepeatData(patter);
		if (rand() % 20 == 0 && patter.size() >= 2)
		{
			CutData(patter);
		}
		if (rand() % 20 == 0)
			OffsetData(patter);

		rTestData.push_back(UnicodeToANSI(patter));
	}

	for (int len = 1; len <= 32; len++)
	{
		int nCount = 0;
		int nSizeTarget = nTestNum;
		if (len == 1)
			nSizeTarget = 100;
		auto& rTestCase = rTestDataMap[len];
		while (rTestCase.size() < nSizeTarget)
		{
			std::wstring patter = GBKToUNICODE(rDataVec[rand() % rDataVec.size()]);
			size_t uPatterLen = patter.size();
			size_t uStartPos = 0;

			nCount++;
			if (nCount > 1000 && rTestCase.empty())
				break;

			if (patter.size() > 1)
				uStartPos = rand() % (patter.size() - 1);

			if (rand() % 20 == 0 && len >= 2)
			{
				patter = patter.substr(uStartPos, len + 1);
				if (patter.size() >= 2)
					CutData(patter);
			}
			else
				patter = patter.substr(uStartPos, len);

			if (patter.size()< len)
				continue;

			assert(patter.size() > 0);

			if (rand() % 20 == 0)
				UnorderData(patter);
			if (rand() % 20 == 0)
				RepeatData(patter);
			if (rand() % 20 == 0)
				OffsetData(patter);

			rTestCase.push_back(UnicodeToANSI(patter));
		}
	}

	if (pFile)
		fclose(pFile);
}

template<class ALGORITHM>
void Test(const DataVec& crDataVec, const DataVec& crTestVec, ALGORITHM& rAl)
{
	DataVec patterns;
	DWORD dwStart = 0;
	DWORD dwEnd = 0;

	dwStart = GetTickCount();

	rAl.Init(crDataVec);

	dwEnd = GetTickCount();
	printf("%s init cost %d ms\n", rAl.GetName(), dwEnd - dwStart);


	dwStart = GetTickCount();

	for (int i = 0; i < (int)crTestVec.size(); i++)
	{
		rAl.Match(crTestVec[i]);
	}

	dwEnd = GetTickCount();
	printf("%s search %d times cost %d ms\n", rAl.GetName(), (int)crTestVec.size(), dwEnd - dwStart);
}

void PrintProgress()
{
	static char s[] = { '|', '/', '-', '\\' };
	static int i = 0;
	putchar('\b');
	i++;
	i %= _countof(s);
	putchar(s[i]);
}

void PrintProgressEnd()
{
	putchar('\b');
}

class BF
{
public:
	const char* GetName()
	{
		return "BF";
	}

	bool Init(const DataVec& crDataVec)
	{
		for (auto& rString : crDataVec)
			m_wDataVec.push_back(GBKToUNICODE(rString));
		return true;
	}

	bool Match(const std::string& crStrPatter)
	{
		std::wstring wPatter = GBKToUNICODE(crStrPatter);
		for (auto& rStr : m_wDataVec)
		{
			size_t findPos = rStr.find(wPatter, 0);
			if (findPos != std::string::npos)
			{
				PrintProgress();
				//std::wcout << rStr.c_str() << L" -> " << wPatter.c_str() << std::endl;
			}
		}
		PrintProgressEnd();
		return true;
	}
private:
	WDataVec m_wDataVec;
};

class BoyerMoore
{
public:
	const char* GetName()
	{
		return "BoyerMoore";
	}

	bool Init(const DataVec& crDataVec)
	{
		for (auto& rString : crDataVec)
			m_wDataVec.push_back(GBKToUNICODE(rString));
		return true;
	}

	bool Match(const std::string& crStrPatter)
	{
		std::wstring wPatter = GBKToUNICODE(crStrPatter);
		for (auto& rStr : m_wDataVec)
		{
			if (search(rStr, wPatter))
			{
				assert(rStr.find(wPatter) != std::string::npos);
				PrintProgress();
				//std::wcout << rStr.c_str() << L" -> " << wPatter.c_str() << std::endl;
			}
			else
			{
				assert(rStr.find(wPatter) == std::string::npos);
			}
		}
		PrintProgressEnd();
		return true;
	}
private:
	#define CHAR_RANGE 64

	inline int CharMapIndex(wchar_t wc)
	{
		for (int i = 0; i < m_nCharMapCounter; i++)
		{
			if (m_charMap[i] == wc)
				return i;
		}
		m_charMap[m_nCharMapCounter] = wc;
		return m_nCharMapCounter++;
	}

	void badCharHeuristic(const std::wstring& str, int size,
		int badchar[CHAR_RANGE])
	{
		int i;

		// Initialize all occurrences as -1
		for (i = 0; i < CHAR_RANGE; i++)
			badchar[i] = -1;

		// Fill the actual value of last occurrence
		// of a character
		for (i = 0; i < size; i++)
			badchar[(int)CharMapIndex(str[i])] = i;
	}

	/* A pattern searching function that uses Bad
	Character Heuristic of Boyer Moore Algorithm */
	bool search(const std::wstring& txt, const std::wstring& pat)
	{
		int m = (int)pat.size();
		int n = (int)txt.size();

		int badchar[CHAR_RANGE];

		m_nCharMapCounter = 0;

		/* Fill the bad character array by calling
		the preprocessing function badCharHeuristic()
		for given pattern */
		badCharHeuristic(pat, m, badchar);

		int s = 0; // s is shift of the pattern with
					// respect to text
		while (s <= (n - m))
		{
			int j = m - 1;

			/* Keep reducing index j of pattern while
			characters of pattern and text are
			matching at this shift s */
			while (j >= 0 && pat[j] == txt[s + j])
				j--;

			/* If the pattern is present at current
			shift, then index j will become -1 after
			the above loop */
			if (j < 0)
			{
				//cout << "pattern occurs at shift = " << s << endl;

				return true;
				/* Shift the pattern so that the next
				character in text aligns with the last
				occurrence of it in pattern.
				The condition s+m < n is necessary for
				the case when pattern occurs at the end
				of text */
				// s += (s + m < n) ? m - badchar[txt[s + m]] : 1;

			}

			else
				/* Shift the pattern so that the bad character
				in text aligns with the last occurrence of
				it in pattern. The max function is used to
				make sure that we get a positive shift.
				We may get a negative shift if the last
				occurrence of bad character in pattern
				is on the right side of the current
				character. */
				s += max(1, j - badchar[CharMapIndex(txt[s + j])]);
		}
		return false;
	}

private:
	WDataVec m_wDataVec;
	wchar_t m_charMap[CHAR_RANGE];
	int m_nCharMapCounter = 0;
};


class HashMatcher
{
public:
	const char* GetName()
	{
		return "HashMatcher";
	}

	bool Init(const DataVec& crDataVec)
	{
		for (auto& rString : crDataVec)
			m_wDataVec.push_back(GBKToUNICODE(rString));

		m_uCofs[0] = 1;
		for (int i = 1; i < _countof(m_uCofs); i++)
			m_uCofs[i] = m_uCofs[i - 1] * m_cuPrime;

		return true;
	}

	bool Match(const std::string& crStrPatter)
	{
		std::wstring wPatter = GBKToUNICODE(crStrPatter);

		int nPatterLen = (int)wPatter.size();
		unsigned int uHash = 0;

		m_uCof = m_uCofs[nPatterLen];

		for (int i = 0; i < nPatterLen; i++)
		{
			uHash *= m_cuPrime;
			uHash += (unsigned int)wPatter[i];
		}

		int nRow = 0;
		for (const auto& rStr : m_wDataVec)
		{
			nRow++;
			if (HashMatch(rStr, nPatterLen, uHash, wPatter))
			{
				assert(rStr.find(wPatter) != std::string::npos);
				PrintProgress();
				//std::wcout << rStr.c_str() << L" -> " << wPatter.c_str() << std::endl;
			}
			else
			{
				assert(rStr.find(wPatter) == std::string::npos);
			}
		}
		PrintProgressEnd();
		return true;
	}
private:
	unsigned int Check(const std::wstring& crText, int nOffset, int nLen)
	{
		unsigned int uHash = 0;
		for (int i = 0; i < nLen; i++)
		{
			uHash *= m_cuPrime;
			uHash += (unsigned int)crText[nOffset + i];
		}
		return uHash;
	}

	bool HashMatch(const std::wstring& crText, int nPatterLen, unsigned int nPatterHash, const std::wstring& wPatter)
	{
		int nTextLen = (int)crText.size();
		if (nPatterLen > nTextLen)
			return false;

		unsigned int uHash = 0;
		for (int i = 0; i < nTextLen; i++)
		{
			uHash *= m_cuPrime;
			uHash += (unsigned int)crText[i];

			if (i >= nPatterLen)
			{
				assert(i - nPatterLen >= 0);
				uHash -= crText[i - nPatterLen] * m_uCof;

				assert(Check(crText, i - nPatterLen + 1, nPatterLen) == uHash);
			}

			//if (uHash == nPatterHash && crText.substr(i - nPatterLen + 1, nPatterLen) == wPatter)
				//return true;
			if (uHash == nPatterHash && crText[i] == wPatter[nPatterLen - 1])
				return true;
		}

		return false;
	}

private:
	WDataVec m_wDataVec;
	const unsigned  int m_cuPrime = 911;
	unsigned int m_uCof = 0;
	unsigned int m_uCofs[64];
};

class HashMatcher2
{
public:
	const char* GetName()
	{
		return "HashMatcher2";
	}

	bool Init(const DataVec& crDataVec)
	{
		for (auto& rString : crDataVec)
			m_wDataVec.push_back(GBKToUNICODE(rString));

		return true;
	}

	bool Match(const std::string& crStrPatter)
	{
		std::wstring wPatter = GBKToUNICODE(crStrPatter);

		int nPatterLen = (int)wPatter.size();
		unsigned int uHash = 0;

		for (int i = 0; i < nPatterLen; i++)
		{
			uHash += (unsigned int)wPatter[i];
		}

		for (const auto& rStr : m_wDataVec)
		{
			if (HashMatch(rStr, nPatterLen, uHash, wPatter))
			{
				assert(rStr.find(wPatter) != std::string::npos);
				PrintProgress();
				//std::wcout << rStr.c_str() << L" -> " << wPatter.c_str() << std::endl;
			}
			else
			{
				assert(rStr.find(wPatter) == std::string::npos);
			}
		}
		PrintProgressEnd();
		return true;
	}
private:
	unsigned int Check(const std::wstring& crText, int nOffset, int nLen)
	{
		unsigned int uHash = 0;
		for (int i = 0; i < nLen; i++)
		{
			uHash += (unsigned int)crText[nOffset + i];
		}
		return uHash;
	}

	bool HashMatch(const std::wstring& crText, int nPatterLen, unsigned int nPatterHash, const std::wstring& wPatter)
	{
		int nTextLen = (int)crText.size();
		if (nPatterLen > nTextLen)
			return false;

		unsigned int uHash = 0;
		for (int i = 0; i < nTextLen; i++)
		{
			uHash += (unsigned int)crText[i];

			if (i >= nPatterLen)
			{
				assert(i - nPatterLen >= 0);
				uHash -= crText[i - nPatterLen];

				assert(Check(crText, i - nPatterLen + 1, nPatterLen) == uHash);
			}

			//if (uHash == nPatterHash && crText.substr(i - nPatterLen + 1, nPatterLen) == wPatter)
				//return true;
			if (uHash == nPatterHash && crText[i] == wPatter[nPatterLen - 1] && crText[i - nPatterLen + 1] == wPatter[0])
				return true;
		}

		return false;
	}

private:
	WDataVec m_wDataVec;
};


class FingerprintMatcher
{
public:
	const char* GetName()
	{
		return "FingerprintMatcher";
	}

	bool Init(const DataVec& crDataVec)
	{
		for (auto& rString : crDataVec)
			m_wDataVec.push_back(GBKToUNICODE(rString));

		for (const auto& wsStr : m_wDataVec)
			m_fingerprinters.push_back(GetFingerptinter(wsStr));

		return true;
	}

	bool Match(const std::string& crStrPatter)
	{
		std::wstring wPatter = GBKToUNICODE(crStrPatter);
		uint64_t ullFingerprinter = GetFingerptinter(wPatter);

		for (int i = 0; i < (int)m_wDataVec.size(); i++)
		{
			const auto& rStr = m_wDataVec[i];
			uint64_t uStoreFingerpinter = m_fingerprinters[i];
			if ((uStoreFingerpinter & ullFingerprinter) == ullFingerprinter && rStr.find(wPatter) != std::string::npos)
			{
				PrintProgress();
				//std::wcout << rStr.c_str() << L" -> " << wPatter.c_str() << std::endl;
			}
		}
		PrintProgressEnd();
		return true;
	}
private:
	inline uint64_t GetFingerptinter(const std::wstring& crText)
	{
		uint64_t ullFingerprint = 0;
		const int nJoinLen = 16;

		for (int i = 0; i < (int)crText.size(); i++)
		{
			int nHash = crText[i];
			for (int j = 1; j < nJoinLen; j++)
			{
				if (i < j)
					break;
				nHash = nHash * 60000 + crText[i - j];
				ullFingerprint |= (1ull << (nHash % 64));
			}
			
			//ullFingerprinter |= (1ull << (crText[i] % 64));
		}
		return ullFingerprint;
	}

private:
	WDataVec m_wDataVec;
	std::vector<uint64_t> m_fingerprinters;
};

class CombinMatcher
{
public:
	const char* GetName()
	{
		return "CombinMatcher";
	}

	bool Init(const DataVec& crDataVec)
	{
		for (auto& rString : crDataVec)
			m_wDataVec.push_back(GBKToUNICODE(rString));

		for (const auto& wsStr : m_wDataVec)
			m_fingerprinters.push_back(GetFingerptinter(wsStr));

		return true;
	}

	bool Match(const std::string& crStrPatter)
	{
		std::wstring wPatter = GBKToUNICODE(crStrPatter);
		uint64_t ullFingerprinter = GetFingerptinter(wPatter);
		uint64_t ullDouHash = 0;

		if (wPatter.size() == 2)
		{
			ullDouHash = ((uint64_t)wPatter[0] << 32) | wPatter[1];
		}

		for (int i = 0; i < (int)m_wDataVec.size(); i++)
		{
			bool bMatch = false;
			const auto& rStr = m_wDataVec[i];
			uint64_t uStoreFingerpinter = m_fingerprinters[i];
			if ((uStoreFingerpinter & ullFingerprinter) != ullFingerprinter)
				goto NEXT;

			if (rStr.size() < wPatter.size())
				goto NEXT;
			
			if (wPatter.size() == 1)
			{
				if (SingleCharMatch(rStr, wPatter[0]))
					bMatch = true;
				goto NEXT;
			}
			
			if (wPatter.size() == 2)
			{
				if (DouCharMatch(rStr, ullDouHash))
					bMatch = true;
				goto NEXT;
			}
			
			if (rStr.find(wPatter) != std::string::npos)
			{
				bMatch = true;
				goto NEXT;
			}
		NEXT:;
			if (bMatch)
				PrintProgress();

			assert(bMatch == (rStr.find(wPatter) != std::string::npos));
		}
		PrintProgressEnd();
		return true;
	}

private:
	inline uint64_t GetFingerptinter(const std::wstring& crText)
	{
		uint64_t ullFingerprint = 0;
		const int nJoinLen = 16;

		for (int i = 0; i < (int)crText.size(); i++)
		{
			int nHash = crText[i];
			for (int j = 1; j < nJoinLen; j++)
			{
				if (i < j)
					break;
				nHash = nHash * 60000 + crText[i - j];
				ullFingerprint |= (1ull << (nHash % 64));
			}

			//ullFingerprinter |= (1ull << (crText[i] % 64));
		}
		return ullFingerprint;
	}

	inline bool SingleCharMatch(const std::wstring& crText, wchar_t wc)
	{
		for (const wchar_t& rWc : crText)
			if (rWc == wc)
				return true;
		return false;
	}

	inline bool DouCharMatch(const std::wstring& crText, uint64_t ullPatter)
	{
		uint64_t ullHash = 0;
		if (crText.size() < 2)
			return false;
		ullHash = crText[0];
		for (int i = 1; i < (int)crText.size(); i++)
		{
			ullHash <<= 32;
			ullHash |= crText[i];
			if (ullHash == ullPatter)
				return true;
		}
		return false;
	}

private:
	WDataVec m_wDataVec;
	std::vector<uint64_t> m_fingerprinters;
};


int main()
{
	DataVec data;
	DataVec test;
	DataLenMap testMap;

	setlocale(LC_ALL, "chs");

	Load(data, test, testMap, 1000);

	/*
	BF bf;
	Test(data, test, bf);

	BoyerMoore bm;
	Test(data, test, bm);

	HashMatcher hm;
	Test(data, test, hm);

	HashMatcher2 hm2;
	Test(data, test, hm2);

	FingerprintMatcher fp;
	Test(data, test, fp);
	*/
	for (auto& LenAndTest : testMap)
	{
		auto& rTest = LenAndTest.second;

		if (rTest.empty())
			continue;

		printf("=====len %d, size %d:====\n", LenAndTest.first, (int)rTest.size());

		BF bf;
		Test(data, rTest, bf);

		BoyerMoore bm;
		Test(data, rTest, bm);

		HashMatcher hm;
		Test(data, rTest, hm);

		HashMatcher2 hm2;
		Test(data, rTest, hm2);

		FingerprintMatcher fp;
		Test(data, rTest, fp);

		CombinMatcher cb;
		Test(data, rTest, cb);
	}

	return 0;
}