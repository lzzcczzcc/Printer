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
	//���ö˿ڻ���
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
	/* ---------- �������� ------- */
	dcb.DCBlength = sizeof(dcb); //DCB���С
	dcb.BaudRate = rate_arg;//������
	dcb.Parity = NOPARITY; //��żУ��0-4���ֱ��ʾ��У�顢��У�飬żУ�顢��š��ո�  
	dcb.fParity = 0;//��������żУ��  
	dcb.StopBits = ONESTOPBIT;//ֹͣλ  
	dcb.ByteSize = 8;//����λ8�����ֽڱ�ʾ4-8 
	dcb.fOutxCtsFlow = 0; //CTS���������  
	dcb.fOutxDsrFlow = 0;   //DSR���������  
	dcb.fDtrControl = DTR_CONTROL_DISABLE;  //DTR����������  
	dcb.fDsrSensitivity = 0;   //��DSR�ź��߲�����  
	dcb.fRtsControl = RTS_CONTROL_DISABLE;  //RTS������  
	dcb.fOutX = 0;   //XON/XOFF���������  
	dcb.fInX = 0;   //XON/XOFF����������  
					/* ---------- �ݴ���� ------- */
	dcb.fErrorChar = 0;   //��������滻  
	dcb.fBinary = 1;   //������ģʽ�������EOF 
	dcb.fNull = 0;   //������룬ȥ��NULL�ַ� 
	dcb.fAbortOnError = 0;   //�д���ʱ��ֹ��д����
	dcb.wReserved = 0;   //  
	dcb.XonLim = 2;   //XON�����ַ�֮ǰ��������������յ���С�ֽ���
	dcb.XoffLim = 4;   //XON�����ַ�֮ǰ���������������С�����ֽ���
	dcb.XonChar = 0x13;   //���ͺͽ���XON�ַ�  
	dcb.XoffChar = 0x19;   //���ͺͽ���XOFF�ַ� 
	dcb.EvtChar = 0;   //���յ����¼��ַ� 
	if (!SetCommState(hComm, &dcb))
		return FALSE;
	else
		return TRUE;
}

bool  LPrinter::setupTimeout(DWORD ReadInterval, DWORD ReadTotalMultiplier,
	DWORD ReadTotalConstant, DWORD WriteTotalMultiplier, DWORD WriteTotalConstant)
{
	COMMTIMEOUTS time;
	time.ReadIntervalTimeout = ReadInterval;   //��ʱ�䳬ʱ  
	time.ReadTotalTimeoutConstant = ReadTotalConstant;  //��ʱ�䳣��  
	time.ReadTotalTimeoutMultiplier = ReadTotalMultiplier;  //��ʱ��ϵ��  
	time.WriteTotalTimeoutConstant = WriteTotalConstant;  //дʱ�䳣��  
	time.WriteTotalTimeoutMultiplier = WriteTotalMultiplier;  //дʱ��ϵ��  
	PurgeComm(hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);//��մ���������
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
			bResult = ReadFile(hComm,  //ͨ���豸���˴�Ϊ���ڣ��������CreateFile()����ֵ�õ�  
				&RXBuff,  //ָ����ջ�����  
				1,  //ָ��Ҫ�Ӵ����ж�ȡ���ֽ���  
				&BytesRead,   //
				&OverLapped);  //OVERLAPPED�ṹ  
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
		bResult = WriteFile(hComm,  //ͨ���豸�����CreateFile()����ֵ�õ�  
			szWriteBuffer,  //ָ��д�����ݻ�����
			dwSend,  //����Ҫд���ֽ���  
			&BytesSent,  //  
			&OverLapped);  //ָ���첽I/O����  
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


//�õ����еĴ��ں�

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

	if (ERROR_SUCCESS == result)   //   �򿪴���ע���   
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
				//   ö�ٴ���
				break;   //   commName���Ǵ�������"COM4"
			}
			cout << "��ǰ���ںţ�" << commName[3] - '0' << endl; /*Ĭ�����ַ��ͣ���ת��Ϊ����������*/
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
	//m_hFile�ļ������m_buf��������m_len����

	DWORD dw = WaitForSingleObject(ov.hEvent, 1000);

	CloseHandle(ov.hEvent);
	return 1;
}


bool LPrinter::SendData(int prt, const char *msg, DWORD dwSend)
{
	string s;

	//���ڷ�������
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