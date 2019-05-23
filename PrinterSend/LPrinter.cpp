#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <cstdlib>
#include <string.h>
#include <tchar.h>
#include <fstream>
#include <assert.h>
#include <string>
#include "LPrinter.h"
#include <vector>
#include <cstring>

using namespace std;

bool LPrinter::PortInit(LPCWSTR Com, int rate_arg)
{
	//设置端口缓冲
	if (OpenPort(Com))
	{
		cout << "Port open succeed " << endl;
	}
	if (setupDCB(rate_arg))
	{
		cout << "DCB setup succeed " << endl;
	}
	setupTimeout(0, 0, 0, 0, 0);

	return 1;
}

bool LPrinter::PortClose()
{
	CloseHandle(hComm);
	return 1;
}

bool LPrinter::OpenPort(LPCWSTR Com)
{
	hComm = CreateFile(Com,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		NULL);
	if (hComm == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hComm);
		return false;
	}
	else
	{
		return TRUE;
	}
}

bool LPrinter::setupDCB(int rate_arg)
{
	DCB dcb;
	memset(&dcb, 0, sizeof(dcb));
	if (!GetCommState(hComm, &dcb))
	{
		return FALSE;
	}
	/* ---------- 串口设置 ------- */
	dcb.DCBlength = sizeof(dcb); //DCB块大小
	dcb.BaudRate = rate_arg;//波特率
	dcb.Parity = NOPARITY; //奇偶校验0-4：分别表示不校验、奇校验，偶校验、标号、空格  
	dcb.fParity = 0;//不允许奇偶校验  
	dcb.StopBits = ONESTOPBIT;//停止位  
	dcb.ByteSize = 8;//数据位8，以字节表示4-8 
	dcb.fOutxCtsFlow = 0; //CTS输出流控制  
	dcb.fOutxDsrFlow = 0;   //DSR输出流控制  
	dcb.fDtrControl = DTR_CONTROL_DISABLE;  //DTR流控制类型  
	dcb.fDsrSensitivity = 0;   //对DSR信号线不敏感  
	dcb.fRtsControl = RTS_CONTROL_DISABLE;  //RTS流控制  
	dcb.fOutX = 0;   //XON/XOFF输出流控制  
	dcb.fInX = 0;   //XON/XOFF输入流控制  
					/* ---------- 容错机制 ------- */
	dcb.fErrorChar = 0;   //允许错误替换  
	dcb.fBinary = 1;   //二进制模式，不检测EOF 
	dcb.fNull = 0;   //允许剥离，去掉NULL字符 
	dcb.fAbortOnError = 0;   //有错误时终止读写操作
	dcb.wReserved = 0;   //  
	dcb.XonLim = 2;   //XON发送字符之前缓冲区中允许接收的最小字节数
	dcb.XoffLim = 4;   //XON发送字符之前缓冲区中允许的最小可用字节数
	dcb.XonChar = 0x13;   //发送和接受XON字符  
	dcb.XoffChar = 0x19;   //发送和接受XOFF字符 
	dcb.EvtChar = 0;   //接收到的事件字符 
	if (!SetCommState(hComm, &dcb))
		return FALSE;
	else
		return TRUE;
}

bool  LPrinter::setupTimeout(DWORD ReadInterval, DWORD ReadTotalMultiplier,
	DWORD ReadTotalConstant, DWORD WriteTotalMultiplier, DWORD WriteTotalConstant)
{
	COMMTIMEOUTS time;
	time.ReadIntervalTimeout = ReadInterval;   //读时间超时  
	time.ReadTotalTimeoutConstant = ReadTotalConstant;  //读时间常量  
	time.ReadTotalTimeoutMultiplier = ReadTotalMultiplier;  //读时间系数  
	time.WriteTotalTimeoutConstant = WriteTotalConstant;  //写时间常量  
	time.WriteTotalTimeoutMultiplier = WriteTotalMultiplier;  //写时间系数  
	PurgeComm(hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);//清空串口区缓存
	if (!SetCommTimeouts(hComm, &time))
		return FALSE;
	else
		return TRUE;
}

void  LPrinter::ReciveChar()
{
	bool bRead = TRUE;
	bool bResult = TRUE;
	DWORD dwError = 0;
	DWORD BytesRead = 0;
	char RXBuff;
	for (;;)
	{
		bResult = ClearCommError(hComm, &dwError, &Comstat);
		if (Comstat.cbInQue == 0)
			continue;
		if (bRead)
		{
			bResult = ReadFile(hComm,  //通信设备（此处为串口）句柄，由CreateFile()返回值得到  
				&RXBuff,  //指向接收缓冲区  
				1,  //指明要从串口中读取的字节数  
				&BytesRead,   //
				&OverLapped);  //OVERLAPPED结构  
			std::cout << RXBuff << std::endl;
			if (!bResult)
			{
				switch (dwError = GetLastError())
				{
				case ERROR_IO_PENDING:
					bRead = FALSE;
					break;
				default:
					break;
				}
			}
		}
		else
		{
			bRead = TRUE;
		}
	}
	if (!bRead)
	{
		bRead = TRUE;
		bResult = GetOverlappedResult(hComm,
			&OverLapped,
			&BytesRead,
			TRUE);
	}
}

bool LPrinter::WriteChar(const char* szWriteBuffer, DWORD dwSend)
{
	bool bWrite = TRUE;
	bool bResult = TRUE;
	DWORD BytesSent = 0;
	HANDLE hWriteEvent = NULL;
	ResetEvent(hWriteEvent);
	if (bWrite)
	{
		OverLapped.Offset = 0;
		OverLapped.OffsetHigh = 0;
		bResult = WriteFile(hComm,  //通信设备句柄，CreateFile()返回值得到  
			szWriteBuffer,  //指向写入数据缓冲区
			dwSend,  //设置要写的字节数  
			&BytesSent,  //  
			&OverLapped);  //指向异步I/O数据  
		if (!bResult)
		{
			DWORD dwError = GetLastError();
			switch (dwError)
			{
			case ERROR_IO_PENDING:
				BytesSent = 0;
				bWrite = FALSE;
				break;
			default:
				break;
			}
		}
	}
	if (!bWrite)
	{
		bWrite = TRUE;
		bResult = GetOverlappedResult(hComm,
			&OverLapped,
			&BytesSent,
			TRUE);
		if (!bResult)
		{
			std::cout << "GetOverlappedResults() in WriteFile()" << std::endl;
		}
	}

	if (BytesSent != dwSend)
	{
		std::cout << "WARNING: WriteFile() error.. Bytes Sent:" << BytesSent << "; Message Length: " << strlen((char*)szWriteBuffer) << std::endl;
	}
	return TRUE;
}


//得到所有的串口号

int LPrinter::GetComList_Reg()
{
	HKEY hkey;
	int result;
	int i = 0;

	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("Hardware\\DeviceMap\\SerialComm"),
		NULL,
		KEY_READ,
		&hkey);

	if (ERROR_SUCCESS == result)   //   打开串口注册表   
	{
		TCHAR portName[0x100], commName[0x100];
		DWORD dwLong, dwSize;

		do
		{
			dwSize = sizeof(portName) / sizeof(TCHAR);
			dwLong = dwSize;
			result = RegEnumValue(hkey, i, portName, &dwLong, NULL, NULL, (LPBYTE)commName, &dwSize);
			if (ERROR_NO_MORE_ITEMS == result)
			{
				//   枚举串口
				break;   //   commName就是串口名字"COM4"
			}
			cout << "当前串口号：" << commName[3] - '0' << endl; /*默认是字符型，需转换为阿拉伯数字*/
			i++;
		} while (1);

		RegCloseKey(hkey);
	}
	return i;
}

bool LPrinter::Lpt_writedata(string msg, int len)
{

	OVERLAPPED ov;
	memset(&ov, 0, sizeof(ov));
	ov.hEvent = CreateEvent(0, true, 0, 0);
	DWORD dwBytesWritten = 0;

	int iRet = WriteFile(hComm, msg.c_str(), (DWORD)len, &dwBytesWritten, &ov);
	//m_hFile文件句柄，m_buf缓冲区，m_len长度

	DWORD dw = WaitForSingleObject(ov.hEvent, 1000);

	CloseHandle(ov.hEvent);
	return 1;
}


bool LPrinter::SendData(int prt, const char *msg, DWORD dwSend)
{
	string s;

	//串口发送数据
	if (prt == 1)
	{
		WriteChar(msg, dwSend);
	}

	if (prt == 2)
	{
		s = msg;
		Lpt_writedata(s, sizeof(s));
	}
	return 0;
}