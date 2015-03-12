// vJoyClient.cpp : Simple feeder application
//
// Supports both types of POV Hats

#include "stdafx.h"
#include "public.h"
#include "vjoyinterface.h"
#include <malloc.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

#pragma comment( lib, "VJOYINTERFACE" )
#define  _CRT_SECURE_NO_WARNINGS

// Use either ROBUST or EFFICIENT functions
#define ROBUST
//#define EFFICIENT

int
__cdecl
_tmain(__in int argc, __in PZPWSTR argv)
{

	USHORT X, Y, XR, YR, S0, S1;							// Position of several axes
	BYTE id=1;										// ID of the target vjoy device (Default is 1)
	UINT iInterface=1;								// Default target vJoy device
	BOOL ContinuousPOV=FALSE;						// Continuous POV hat (or 4-direction POV Hat)
	int count=0;
    char c;
    string input;
    int degree;


	// Get the ID of the target vJoy device
	if (argc>1 && wcslen(argv[1]))
		sscanf_s((char *)(argv[1]), "%d", &iInterface);


	// Get the driver attributes (Vendor ID, Product ID, Version Number)
	if (!vJoyEnabled())
	{
		_tprintf("vJoy driver not enabled: Failed Getting vJoy attributes.\n");
		return -2;
	}
	else
	{
		_tprintf("Vendor: %S\nProduct :%S\nVersion Number:%S\n", TEXT(GetvJoyManufacturerString()),  TEXT(GetvJoyProductString()), TEXT(GetvJoySerialNumberString()));
	};

	WORD VerDll, VerDrv;
	if (!DriverMatch(&VerDll, &VerDrv))
		_tprintf("Failed\r\nvJoy Driver (version %04x) does not match vJoyInterface DLL (version %04x)\n", VerDrv ,VerDll);
	else
		_tprintf( "OK - vJoy Driver and vJoyInterface DLL match vJoyInterface DLL (version %04x)\n", VerDrv);

	// Get the state of the requested device
	VjdStat status = GetVJDStatus(iInterface);
	switch (status)
	{
	case VJD_STAT_OWN:
		_tprintf("vJoy Device %d is already owned by this feeder\n", iInterface);
		break;
	case VJD_STAT_FREE:
		_tprintf("vJoy Device %d is free\n", iInterface);
		break;
	case VJD_STAT_BUSY:
		_tprintf("vJoy Device %d is already owned by another feeder\nCannot continue\n", iInterface);
		return -3;
	case VJD_STAT_MISS:
		_tprintf("vJoy Device %d is not installed or disabled\nCannot continue\n", iInterface);
		return -4;
	default:
		_tprintf("vJoy Device %d general error\nCannot continue\n", iInterface);
		return -1;
	};

	// Check which axes are supported
	BOOL AxisX  = GetVJDAxisExist(iInterface, HID_USAGE_X);
	BOOL AxisY  = GetVJDAxisExist(iInterface, HID_USAGE_Y);
	BOOL AxisRX = GetVJDAxisExist(iInterface, HID_USAGE_RX);
	BOOL AxisRY = GetVJDAxisExist(iInterface, HID_USAGE_RY);
    BOOL AxisSl0 = GetVJDAxisExist(iInterface, HID_USAGE_SL0);
    BOOL AxisSl1 = GetVJDAxisExist(iInterface, HID_USAGE_SL1);
	// Get the number of buttons and POV Hat switchessupported by this vJoy device
	int nButtons  = GetVJDButtonNumber(iInterface);
	int ContPovNumber = GetVJDContPovNumber(iInterface);
	int DiscPovNumber = GetVJDDiscPovNumber(iInterface);

	// Print results
	_tprintf("\nvJoy Device %d capabilities:\n", iInterface);
	_tprintf("Numner of buttons\t\t%d\n", nButtons);
	_tprintf("Numner of Continuous POVs\t%d\n", ContPovNumber);
	_tprintf("Numner of Descrete POVs\t\t%d\n", DiscPovNumber);
	_tprintf("Axis X\t\t%s\n", AxisX?"Yes":"No");
	_tprintf("Axis Y\t\t%s\n", AxisX?"Yes":"No");
	_tprintf("Axis Rx\t\t%s\n", AxisRX?"Yes":"No");
	_tprintf("Axis Ry\t\t%s\n", AxisRY?"Yes":"No");
    _tprintf("Axis Rx\t\t%s\n", AxisSl0? "Yes" : "No");
    _tprintf("Axis Ry\t\t%s\n", AxisSl1? "Yes" : "No");



	// Acquire the target
	if ((status == VJD_STAT_OWN) || ((status == VJD_STAT_FREE) && (!AcquireVJD(iInterface))))
	{
		_tprintf("Failed to acquire vJoy device number %d.\n", iInterface);
		return -1;
	}
	else
	{
		_tprintf("Acquired: vJoy device number %d.\n", iInterface);
	}



	_tprintf("\npress enter to stat feeding");
	getchar();

	X = 0;
	Y = 20;
	XR = 40;
	YR = 60;
    S0 = 80;
    S1 = 100;

	long value = 0;
	BOOL res = FALSE;

#ifdef ROBUST
	// Reset this device to default values
	ResetVJD(iInterface);

	// Feed the device in endless loop
	while(1)
	{
		// Set position of 4 axes
		res = SetAxis(X, iInterface, HID_USAGE_X);
		res = SetAxis(Y, iInterface, HID_USAGE_Y);
		res = SetAxis(XR, iInterface, HID_USAGE_RX);
		res = SetAxis(YR, iInterface, HID_USAGE_RY);
        res = SetAxis(S0, iInterface, HID_USAGE_SL0);
        res = SetAxis(S1, iInterface, HID_USAGE_SL1);

		// Press/Release Buttons
        cin >> input;
        for (int i = 0; i <= 11; i++) if (input[0] == 65 + i) res = SetBtn(TRUE, iInterface, i + 1);
        for (int i = 0; i <= 11; i++) if (input[0] == 97 + i) res = SetBtn(FALSE, iInterface, i + 1);
        if (input[0] >= 54 && input[0] <= 57) SetDiscPov(input[0] - 54, iInterface, 1);
        if (input[0] == 45) SetDiscPov(-1, iInterface, 1);
        if (input[0] >= 48 && input[0] <= 54) degree = atoi(input.substr(1).c_str());
        if (input[0] == 48) X = degree;
        if (input[0] == 49) Y = degree;
        if (input[0] == 50) XR = degree;
        if (input[0] == 51) YR = degree;
        if (input[0] == 52) S0 = degree;
        if (input[0] == 53) S1 = degree;


		//res = SetBtn(TRUE, iInterface, count/50);
		//res = SetBtn(FALSE, iInterface, 1+count/50);

		// If Continuous POV hat switches installed - make them go round
		// For high values - put the switches in neutral state
		/*if (ContPovNumber)
		{
			if ((count*70) < 30000)
			{
				res = SetContPov((DWORD)(count*70)		, iInterface, 1);
				res = SetContPov((DWORD)(count*70)+2000	, iInterface, 2);
				res = SetContPov((DWORD)(count*70)+4000	, iInterface, 3);
				res = SetContPov((DWORD)(count*70)+6000	, iInterface, 4);
			}
			else
			{
				res = SetContPov(-1 , iInterface, 1);
				res = SetContPov(-1 , iInterface, 2);
				res = SetContPov(-1 , iInterface, 3);
				res = SetContPov(-1 , iInterface, 4);
			};
		};

		// If Discrete POV hat switches installed - make them go round
		// From time to time - put the switches in neutral state
		if (DiscPovNumber)
		{
			if (count < 550)
			{
				SetDiscPov(((count/20) + 0)%4, iInterface, 1);
				SetDiscPov(((count/20) + 1)%4, iInterface, 2);
				SetDiscPov(((count/20) + 2)%4, iInterface, 3);
				SetDiscPov(((count/20) + 3)%4, iInterface, 4);
			}
			else
			{
				SetDiscPov(-1, iInterface, 1);
				SetDiscPov(-1, iInterface, 2);
				SetDiscPov(-1, iInterface, 3);
				SetDiscPov(-1, iInterface, 4);
			};
		};

		Sleep(20);
		X+=100;
		Y+=125;
		XR+=150;
		YR+=175;
        S0 +=200;
        S1 += 225;*/
		count++;
		if (count > 640) count=0;
	} // While
#endif

	_tprintf("OK\n");

	return 0;
}
