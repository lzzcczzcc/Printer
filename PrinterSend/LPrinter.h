#pragma once
#pragma once

#ifndef _PRINTER_H_
#define _PRINTER_H_
#include <iostream>
#include <string.h>
#include <sstream>
#include <Windows.h>
using namespace std;
class LPrinter
{
public:
	LPrinter() {}
	~LPrinter() {}
	bool SendData(int prt, const char* msg, DWORD dwSend);
	bool Lpt_writedata(string msg, int len);
	bool PortClose();
	bool PortInit(LPCWSTR Com, int rate_arg);
	bool OpenPort(LPCWSTR Com);
	bool setupDCB(int rate_arg);
	bool setupTimeout(DWORD ReadInterval, DWORD ReadTotalMultiplier, DWORD ReadTotalConstant, DWORD WriteTotalMultiplier, DWORD WriteTotalConstant);
	void ReciveChar();
	bool WriteChar(const char* szWriteBuffer, DWORD dwSend);
	int GetComList_Reg();

private:
	HANDLE hComm;
	OVERLAPPED OverLapped;
	COMSTAT Comstat;
	DWORD dwCommEvents;
};


#endif