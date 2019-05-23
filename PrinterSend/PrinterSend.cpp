// PrinterSend.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "LPrinter.h"
#include <vector>
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) // 设置入口地址
LPrinter ZBRPrinter;
int PrinterSelect;//选择串口还是并口


LPCWSTR stringToLPCWSTR(std::string orig)
{
	size_t origsize = orig.length() + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t* wcstring = (wchar_t*)malloc(sizeof(wchar_t) * (orig.length() - 1));
	mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);

	return wcstring;
}

//从串口发送数据，直到发送完毕
bool ZBRPrintMessage(string SenddWord)
{
	char a;
	if (PrinterSelect == 1)
	{
		for (int i = 0; i < SenddWord.length(); i++)
		{
			a = SenddWord[i];
			cout << " " << a;

			ZBRPrinter.WriteChar(&a, sizeof(a));
		}
	}
	if (PrinterSelect == 2)
	{
		ZBRPrinter.Lpt_writedata(SenddWord, SenddWord.length());
	}

	ZBRPrinter.PortClose();  //关闭串口

	return TRUE;
}


bool PrinterInitial(vector<string> ins)
{
	int CycleTime = 0;
	int ZBRPrinterBaudrate = 115200;
	string  s;  //用来解析VECTOR里面的内容
	for (int j = 0; j < ins.size(); j++)		//逐行读取文件
	{
		CycleTime++;
		s = ins[j];
		cout << "第" << j << "次 : s的值为 " << s << endl;
		if (CycleTime == 1)
		{
			string::size_type s_pos;
			if (s_pos = s.find("COM") == std::string::npos)
			{
				if (s_pos = s.find("LPT") == std::string::npos)
				{
					cout << "不能打开端口" << endl;
					return 0;
				}
				else
				{
					LPCWSTR TempPort;
					PrinterSelect = 2;
					string OpenPrt;
					s_pos = s.find("LPT", 0);
					OpenPrt = s.substr(s_pos);
					cout << "LPT POS is : " << s_pos << endl;
					cout << "OpenPort is : " << OpenPrt << endl;
					TempPort = stringToLPCWSTR(OpenPrt);
					ZBRPrinter.PortInit(TempPort, 115200);
				}
			}
			else
			{
				LPCWSTR TempPort;
				PrinterSelect = 1;
				string OpenPrt;
				s_pos = s.find("COM", 0);
				OpenPrt = s.substr(s_pos);
				cout << "COM POS is : " << s_pos << endl;
				cout << "OpenPort is : " << OpenPrt << endl;
				TempPort = stringToLPCWSTR(OpenPrt);
				ZBRPrinter.PortInit(TempPort, 115200);
			}
		}
		if (CycleTime == 2)
		{
			ZBRPrintMessage(s);
		}
	}

	return TRUE;
}


//string转LPCWSTR

//切割字符串
void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c)
{
	std::string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (std::string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
}


int main(int argc , char ** argv)
{
	vector<string> s;
	string OriginCode;
	OriginCode = argv[1];//获得原始字符串
	SplitString(OriginCode, s, ";");
	for (int i = 0; i < s.size(); i++)
	{
		cout << s[i] << endl;
	}
	PrinterInitial(s);
	//system("pause");

    return 0;
}

