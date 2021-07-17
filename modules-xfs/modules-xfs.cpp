// modules-xfs.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <windows.h>
#include <thread>
#include <string>
#include "XFSAPI.H"
#include "XFSADMIN.H"
#include "XFSCONF.H"
#include <XFSBCR.H>

using namespace std;

HANDLE hRegEvent = NULL;
HINSTANCE hInst = NULL;
LPCWSTR lpszClass = L"XFSTEST";
HWND messageWindow = NULL;
HANDLE hModuleThread = NULL;
bool OpenOK = false;
HSERVICE hService;

void ExecuteCloseAndClening();
void ExecuteGetInfo();
void ExecuteCommands();
boolean CreateXFSMonitor();
DWORD FAR PASCAL ThreadMonitor(string name);
boolean RegisterCallback();
LRESULT CALLBACK  PostCallBack(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void ExecuteCloseAndClening()
{
	cout << "Press ENTER>>>>>>>>" << endl;
	cin.ignore();
	cout << "Executing Close and Cleaning..." << endl;

	HRESULT hResult;

	hResult = WFSClose(hService);
	cout << "WFSClose Result: " << dec << hResult << endl;

	hResult = WFSCleanUp();
	cout << "WFSCleanUp Result: " << dec << hResult << endl;

	if (messageWindow)
		DestroyWindow(messageWindow);

	if (hInst)
		UnregisterClass(lpszClass, hInst);
}

void ExecuteGetInfo()
{
	cout << "Press ENTER>>>>>>>>" << endl;
	cin.ignore();
	cout << "Executing GetInfo..." << endl;

	REQUESTID requestID;
	HRESULT hResult;

	hResult = WFSAsyncGetInfo(hService, WFS_INF_BCR_STATUS, NULL, (30 * 1000), messageWindow, &requestID);
	cout << "Info(status BCR)->ReqID(" << dec << requestID << ") = " << hResult << endl;

	hResult = WFSAsyncGetInfo(hService, WFS_INF_BCR_CAPABILITIES, NULL, (30 * 1000), messageWindow, &requestID);
	cout << "Info(Capabilities BCR)->ReqID(" << dec << requestID << ") = " << hResult << endl;
}

void ExecuteCommands() {
	cout << "Press ENTER>>>>>>>>" << endl;
	cin.ignore();
	cout << "Executing BCR Commands..." << endl;
	cout << "BCR hService: " << dec << hService << endl;

	REQUESTID requestID;
	HRESULT hResult;
	WFSBCRREADINPUT lpReadInput = { 0 };
	lpReadInput.lpwSymbologies = nullptr;
	
	cout << "wfs_bcr_read_input.lpwSymbologies" << lpReadInput.lpwSymbologies << endl;

	hResult = WFSAsyncExecute(hService, WFS_CMD_BCR_READ, &lpReadInput, (30 * 1000), messageWindow, &requestID);
	cout << "Exec(WFS_CMD_BCR_READ)->ReqID(" << dec << requestID << ") = " << hResult << endl;
}

boolean CreateXFSMonitor() {
	cout << "---" << "Monitor de Eventos XFS" << "---" << endl;
	DWORD IdThread;

	hModuleThread = CreateThread(
		(LPSECURITY_ATTRIBUTES)NULL,
		0L,
		(LPTHREAD_START_ROUTINE)ThreadMonitor,
		(LPVOID)NULL,
		CREATE_SUSPENDED, // Nao inicia de imediato
		&IdThread); // IdThread

	if (hModuleThread) {

		hRegEvent = CreateEvent(
			NULL,    // no security
			TRUE,    // explicit reset req
			FALSE,   // initial event reset
			NULL     // no name
			);

		if (hRegEvent) {
			ResumeThread(hModuleThread);
			WaitForSingleObject(hRegEvent, 3000l);
		}
	}
	else {
		return false;
	}

	return true;
}

DWORD FAR PASCAL ThreadMonitor(string name) {
	cout << "---" << __FUNCTION__ << "---" << endl;
	if (!RegisterCallback())
		return 0;

	SetEvent(hRegEvent); // Processo de regitro Efetuado

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 1; // Thread finalizada normalmente
}

boolean RegisterCallback() {
	cout << "---" << __FUNCTION__ << "---" << endl;
	WNDCLASS WinClass;
	HWND hwndPost = NULL;
	
	memset(&WinClass, 0, sizeof(WNDCLASS));
	WinClass.style = 0; // Not Global
	WinClass.lpfnWndProc = (WNDPROC)PostCallBack;
	WinClass.hInstance = hInst;
	WinClass.lpszClassName = lpszClass;

	if (RegisterClass(&WinClass)) {
		hwndPost = CreateWindow(lpszClass,  /* Class Name */
			NULL,                           /* Caption */
			0,                              /* Style */
			0, 0, 0, 0,                     /* x,y,cx,cy */
			NULL,                           /* Parent Window = Desktop */
			NULL,                           /* Menu Handle */
			hInst,                          /* Module Instance */
			NULL);                          /* Initialization data */

		if (!hwndPost) {
			cout << "falha ao criar janela" << endl;
			return false;
		}
	}
	else {
		return false;
	}

	cout << "hwndPost: " << hex << hwndPost << endl;

	messageWindow = hwndPost;

	return true;
}

LRESULT CALLBACK  PostCallBack(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	cout << "---" << __FUNCTION__ << "---" << endl;
	cout << "uMsg: " << dec << uMsg << endl;
	switch (uMsg) {
		case WFS_TIMER_EVENT:
			cout << "WM_TIMER" << endl;
			return 0;
		case WFS_EXECUTE_EVENT:
			cout << "WFS_EXECUTE_EVENT" << endl;
			return 0;
		case WFS_SERVICE_EVENT:
			cout << "WFS_SERVICE_EVENT" << endl;
			return 0;
		case WFS_USER_EVENT:
			cout << "WFS_USER_EVENT" << endl;
			return 0;
		case WFS_SYSTEM_EVENT:
			cout << "WFS_SYSTEM_EVENT" << endl;
			return 0;
		case WFS_OPEN_COMPLETE:
			cout << "WFS_OPEN_COMPLETE" << endl;
			return 0;
		case WFS_CLOSE_COMPLETE:
			cout << "WFS_CLOSE_COMPLETE" << endl;
			return 0;
		case WFS_LOCK_COMPLETE:
			cout << "WFS_LOCK_COMPLETE" << endl;
			return 0;
		case WFS_UNLOCK_COMPLETE:
			cout << "WFS_UNLOCK_COMPLETE" << endl;
			return 0;
		case WFS_REGISTER_COMPLETE:
			cout << "WFS_REGISTER_COMPLETE" << endl;
			return 0;
		case WFS_DEREGISTER_COMPLETE:
			cout << "WFS_DEREGISTER_COMPLETE" << endl;
			return 0;
		case WFS_GETINFO_COMPLETE:
		{
			LPWFSRESULT res = (LPWFSRESULT)lParam;
			if (res != NULL) {
				char buf[1024];
				sprintf_s(buf, "%s WFS_GETINFO_COMPLETE(ReqID=%d)->hResult: %d", "\t\t\t\t\t\t", res->RequestID, res->hResult);
				cout << string(buf) << endl;
				cout << "res->u.dwCommandCode: " << res->u.dwCommandCode << endl;
				switch (res->u.dwCommandCode) {
					case WFS_INF_BCR_STATUS:
						if (res->lpBuffer != NULL) {
							LPWFSBCRSTATUS lpStatus = (LPWFSBCRSTATUS)res->lpBuffer;
							char buf[1024];
							string aux;

							sprintf_s(buf, "%s lpStatus->fwDevice: %d", "\t\t\t\t\t\t\t", lpStatus->fwDevice);
							cout << string(buf) << endl;

							sprintf_s(buf, "%s lpStatus->fwBCRScanner: %d", "\t\t\t\t\t\t\t", lpStatus->fwBCRScanner);
							cout << string(buf) << endl;
						}
						break;
					case WFS_INF_BCR_CAPABILITIES:
						if (res->lpBuffer != NULL) {
							LPWFSBCRCAPS lpCaps = (LPWFSBCRCAPS)res->lpBuffer;
							char buf[1024];
							string aux;
							int i = 0;
							while (lpCaps->lpwSymbologies[i]){
								int st = (int)lpCaps->lpwSymbologies[i];
								aux.append(to_string(st));
								aux.append("\n");
								i++;
							}
							sprintf_s(buf, "%s lpCaps->lpwSymbologies: %s", "\t\t\t\t\t\t\t", aux.c_str());
							cout << string(buf) << endl;

							aux.clear();
							sprintf_s(buf, "%s lpCaps->wClass: %d", "\t\t\t\t\t\t\t", lpCaps->wClass);
							cout << string(buf) << endl;
								
							aux.clear();
							sprintf_s(buf, "%s lpCaps->bCanFilterSymbologies: %d", "\t\t\t\t\t\t\t", lpCaps->bCanFilterSymbologies);
							cout << string(buf) << endl;
						}
						break;
					}					
				WFSFreeResult(res);
			}
			else {
				cout << "WFS_GETINFO_COMPLETE" << endl;
			}
		}
		return 0;
		case WFS_EXECUTE_COMPLETE:
		{			
			LPWFSRESULT res = (LPWFSRESULT)lParam;
			
			if (res != NULL) {
				char buf[1024];
				sprintf_s(buf, "%s WFS_EXECUTE_COMPLETE(ReqID=%d)->hResult: %d", "\t\t\t\t\t\t", res->RequestID, res->hResult);
				cout << string(buf) << endl;
				
				switch (res->u.dwCommandCode) {
					case WFS_CMD_BCR_READ:
						cout << "0 WFS_CMD_BCR_READ" << endl;
						if (res->lpBuffer != NULL) {
							cout << "1 WFS_CMD_BCR_READ" << endl;
							LPWFSBCRREADOUTPUT *lppReadOutput = (LPWFSBCRREADOUTPUT*)(res->lpBuffer);
							sprintf_s(buf, "%s lppReadOutput->wSymbology: %d", "\t\t\t\t\t\t\t", lppReadOutput[0]->wSymbology);
							cout << string(buf) << endl;
							
							LPWFSBCRXDATA lpBcrXData = lppReadOutput[0]->lpxBarcodeData;
							sprintf_s(buf, "%s lpBcrXData->lpbDatas: %S", "\t\t\t\t\t\t\t", lpBcrXData->lpbData);
							cout << string(buf) << endl;
							cout << "wfsBcrXdata->lpbData: " << lpBcrXData->lpbData << endl;
							cout << "wfsBcrXdata->usLength: " << lpBcrXData->usLength << endl;
						}
						break;
				}
				WFSFreeResult(res);
			}
			else {
				cout << "WFS_EXECUTE_COMPLETE" << endl;
			}
		}
			return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int main(int argc, _TCHAR* argv[])
{
	HRESULT hResult;
	WFSVERSION xfs_version;
	WFSVERSION sp_version;
	DWORD LOW_VERSION = 0x1E03;// 0x0A03;
	DWORD HIGH_VERSION = 0x1E03;

	hResult = WFSStartUp(MAKELONG(LOW_VERSION, HIGH_VERSION), &xfs_version);

	cout << "WFSStartUp Result: " << hResult << endl;
	cout << "Description: " << xfs_version.szDescription << endl;
	cout << "SystemStatus: " << xfs_version.szSystemStatus << endl;
	cout << "HighVersion: " << hex << xfs_version.wHighVersion << endl;
	cout << "LowVersion: " << hex << xfs_version.wLowVersion << endl;
	cout << "Version: " << xfs_version.wVersion << endl;
	cout << endl;

	hResult = WFSOpen(
		const_cast<LPSTR>("DN_BarCodeScanner4_BCR"),
		WFS_DEFAULT_HAPP,
		const_cast<LPSTR>("SEP"),
		0,
		5000,
		MAKELONG(LOW_VERSION, HIGH_VERSION),
		&xfs_version,
		&sp_version,
		&hService
		);

	cout << "WFSOpen Result: " << dec << hResult << endl;
	cout << "hService: " << dec << hService << endl;

	cout << "XFS Version: " << endl;
	cout << "Description: " << xfs_version.szDescription << endl;
	cout << "SystemStatus: " << xfs_version.szSystemStatus << endl;
	cout << "HighVersion: " << hex << xfs_version.wHighVersion << endl;
	cout << "LowVersion: " << hex << xfs_version.wLowVersion << endl;
	cout << "Version: " << xfs_version.wVersion << endl;
	cout << endl;
	cout << "SP Version: " << endl;
	cout << "Description: " << sp_version.szDescription << endl;
	cout << "SystemStatus: " << sp_version.szSystemStatus << endl;
	cout << "HighVersion: " << hex << sp_version.wHighVersion << endl;
	cout << "LowVersion: " << hex << sp_version.wLowVersion << endl;
	cout << "Version: " << sp_version.wVersion << endl;

	if (hResult == WFS_SUCCESS) {
		OpenOK = true;
		
		CreateXFSMonitor();
		if (messageWindow != NULL) {
			HRESULT hResult = WFSRegister(hService, (SYSTEM_EVENTS | USER_EVENTS | SERVICE_EVENTS | EXECUTE_EVENTS), messageWindow);
			cout << "WFSRegister Result: " << dec << hResult << endl;

			if (hResult == WFS_SUCCESS) {
				//thread monitor(ThreadMonitor, "Loop de Mensagens");
				//monitor.detach();
			}
			else {
				cout << "Error WFSRegister Result: " << dec << hResult << endl;
			}
		}
		else {
			cout << "falha ao criar janela" << endl;
		}
	}
	if (OpenOK){
		ExecuteCommands();
		ExecuteGetInfo();
		ExecuteCloseAndClening();
	}
	

	system("pause");
	return 0;
}

