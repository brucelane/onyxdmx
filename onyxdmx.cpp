// onyxdmx.cpp : Defines the entry point for the console application.
// Original code from Jason Hotchkiss
//

#include "stdafx.h"
#include "windef.h"
#include <iostream>
//#include "K8062D.h"
#include <windows.h>

HMIDIIN hMidiIn = NULL;
HMIDIOUT hMidiOut = NULL;
#define BUF_SIZE 200
char szBuffer[BUF_SIZE + 1] = {0};
int iBufferIndex = 0;
DWORD tm = GetTickCount();

typedef UINT (CALLBACK* LPFNDLLSTARTDEVICE)();
typedef UINT (CALLBACK* LPFNDLLSTOPDEVICE)();
typedef UINT (CALLBACK* LPFNDLLSETDATA)(LONG,LONG);

HINSTANCE hDLL;               // Handle to DLL
LPFNDLLSTARTDEVICE lpfnDllStartDevice;			// Function pointer
LPFNDLLSTOPDEVICE lpfnDllStopDevice;			// Function pointer
LPFNDLLSETDATA lpfnDllSetData;			// Function pointer
LONG channel, data;

//extern "C" __declspec(dllimport) void StartDevice();
//extern "C" __declspec(dllimport) void SetData(long Channel, long Data);
//extern "C" __declspec(dllimport) void SetChannelCount(long Count);
//extern "C" __declspec(dllimport) void StopDevice();
//end

//extern "C" __declspec( dllimport ) static void StartDevice();
//////////////////////////////////////////////////////////
//
// MIDI MESSAGE CALLBACK
//
//////////////////////////////////////////////////////////
void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	if(wMsg == MIM_DATA)
	{
		if ( GetTickCount() > tm + 300 )
		{
			fprintf(stdout, "!%d\n",  dwParam1);	
			tm = GetTickCount();
		}
	}
}

//////////////////////////////////////////////////////////
//
// STOP MIDI IN
//
//////////////////////////////////////////////////////////
void StopMidiIn()
{
    if(hMidiIn)
    {
        midiInClose(hMidiIn);
        hMidiIn = NULL;
    }
	fprintf(stdout, "--OK\n");
}
//////////////////////////////////////////////////////////
//
// STOP MIDI OUT
//
//////////////////////////////////////////////////////////
void StopMidiOut()
{
    if(hMidiOut)
    {
        midiOutReset(hMidiOut);
        midiOutClose(hMidiOut);
        hMidiOut = NULL;
    }
	fprintf(stdout, "--OK\n");
}
//////////////////////////////////////////////////////////
//
// START MIDI IN
//
//////////////////////////////////////////////////////////
int StartMidiIn(char *szDevice)
{
	int iCount;
	int iDevice;
	UINT uiDevices;
	MMRESULT result;

	// convert device name to lower case
	WCHAR wcsDevice[MAXPNAMELEN + 1];
	for(iCount = 0; szDevice[iCount] && iCount < MAXPNAMELEN; ++iCount)
		wcsDevice[iCount] = tolower(szDevice[iCount]);
	wcsDevice[iCount] = 0;

	// stop midi if already open
	if(hMidiIn)
		StopMidiIn();

	//////////////////////////////////////////////////////////
	// OPEN UP MIDI INPUT DEVICE
    uiDevices = midiInGetNumDevs(); 
    for(iDevice = 0; iDevice < (int)uiDevices; ++iDevice)
    {
        MIDIINCAPS stMIC = {0};
        if(MMSYSERR_NOERROR == midiInGetDevCaps(iDevice, &stMIC, sizeof(stMIC)))
        {
			_wcslwr(stMIC.szPname);
            if(wcsstr(stMIC.szPname, wcsDevice))
            {
				result = midiInOpen(&hMidiIn, iDevice, (DWORD)MidiInProc, NULL, CALLBACK_FUNCTION);
                if(result != MMSYSERR_NOERROR)
                {
					fprintf(stdout, "*** failed to open MIDI input device \'%S\', status=%lx\n", stMIC.szPname, result);
					return 0;
				}
				result = midiInStart(hMidiIn);
                if(result != MMSYSERR_NOERROR)
				{
					fprintf(stdout, "*** failed to start MIDI input device \'%S\', status=%lx\n", stMIC.szPname, result);
					return 0;
				}
				fprintf(stdout, "-opened input device \'%S\'\n", stMIC.szPname);
				break;
            }
        }
    }
	fprintf(stdout, "--OK");
	return 1;
} 
//////////////////////////////////////////////////////////
//
// START MIDI OUT
//
//////////////////////////////////////////////////////////
int StartMidiOut(char *szDevice)
{
	int iCount;
	int iDevice;
	UINT uiDevices;
	MMRESULT result;

	// convert device name to lower case
	WCHAR wcsDevice[MAXPNAMELEN + 1];
	for(iCount = 0; szDevice[iCount] && iCount < MAXPNAMELEN; ++iCount)
		wcsDevice[iCount] = tolower(szDevice[iCount]);
	wcsDevice[iCount] = 0;

	// stop midi if already open
	if(hMidiOut)
		StopMidiOut();

	// OPEN UP MIDI OUTPUT DEVICE
    uiDevices = midiOutGetNumDevs(); 
    for(iDevice = 0; iDevice < (int)uiDevices; ++iDevice)
    {
        MIDIOUTCAPS stMOC = {0};
        if(MMSYSERR_NOERROR == midiOutGetDevCaps(iDevice, &stMOC, sizeof(stMOC)))
        {
			_wcslwr(stMOC.szPname);
            if(wcsstr(stMOC.szPname, wcsDevice))
            {
				result = midiOutOpen(&hMidiOut, iDevice, NULL, NULL, 0);
                if(result != MMSYSERR_NOERROR)
                {
					fprintf(stdout, "*** failed to open MIDI output device \'%S\', status=%lx\n", stMOC.szPname, result);
					return 0;
                }
				fprintf(stdout, "- opened output device \'%S\'\n", stMOC.szPname);
				break;
            }
        }
    }

	fprintf(stdout, "--OK");
	return 1;
} 
//////////////////////////////////////////////////////////
//
// SEND CHANNEL MESSAGE
//
//////////////////////////////////////////////////////////
int sendChannelMessage(char *szBuffer)
{
	char *pchPos = szBuffer;
	MMRESULT result;
	int iStatus = 0;
	int iParam1 = 0;
	int iParam2 = 0;

	if(!hMidiOut)
	{
		fprintf(stdout, "*** must open MIDI device before sending channel message\n");
		return 0;
	}

	char *pchEndPtr = NULL;
	iStatus = strtol(pchPos, &pchEndPtr, 10);
	if(!pchEndPtr || *pchEndPtr != ',')
	{
		fprintf(stdout, "*** invalid channel message \'%s\', param 1\n", szBuffer);
		return 0;
	}
	pchPos = pchEndPtr + 1;
	iParam1 = strtol(pchPos, &pchEndPtr, 10);
	if(!pchEndPtr || *pchEndPtr != ',')
	{
		fprintf(stdout, "*** invalid channel message \'%s\', param 2\n", szBuffer);
		return 0;
	}
	pchPos = pchEndPtr + 1;
	iParam2 = strtol(pchPos, &pchEndPtr, 10);
	if(pchEndPtr && *pchEndPtr != '\0')
	{
		fprintf(stdout, "*** invalid channel message %s, param 3\n", szBuffer);
		return 0;
	}

	if(iStatus < 0 || iStatus > 255)
	{
		fprintf(stdout, "*** invalid channel message %s, param 1 out of range\n", szBuffer);
		return 0;
	}
	if(iParam1 < 0 || iParam1 > 127)
	{
		fprintf(stdout, "*** invalid channel message %s, param 2 out of range\n", szBuffer);
		return 0;
	}
	if(iParam2 < 0 || iParam2 > 127)
	{
		fprintf(stdout, "*** invalid channel message %s, param 2 out of range\n", szBuffer);
		return 0;
	}

	DWORD dwMsg = iStatus | (iParam1 << 8) | (iParam2 << 16);
	result = midiOutShortMsg(hMidiOut, dwMsg);
    if(MMSYSERR_NOERROR != result)
	{
		fprintf(stdout, "*** channel send failed, status=%lx\n", result);
		return 0;
    }
	
	//fflush(stdout);

	return 1;
}

//////////////////////////////////////////////////////////
//
// LIST INSTALLED MIDI DEVICES
//
//////////////////////////////////////////////////////////
void ListMidiDevices()
{
	UINT uiDevices;
	int iDevice;
    uiDevices = midiInGetNumDevs(); 
    for(iDevice = 0; iDevice < (int)uiDevices; ++iDevice)
    {
        MIDIINCAPS stMIC = {0};
        if(MMSYSERR_NOERROR == midiInGetDevCaps(iDevice, &stMIC, sizeof(stMIC)))
			fprintf(stdout, "--IN:%S\n", stMIC.szPname);
    }
    uiDevices = midiOutGetNumDevs(); 
    for(iDevice = 0; iDevice < (int)uiDevices; ++iDevice)
    {
        MIDIOUTCAPS stMOC = {0};
        if(MMSYSERR_NOERROR == midiOutGetDevCaps(iDevice, &stMOC, sizeof(stMOC)))
			fprintf(stdout, "--OT:%S\n", stMOC.szPname);
    }

	fprintf(stdout, "--OK\n");
	fflush(stdout);

}

//////////////////////////////////////////////////////////
//
// COMMAND HANDLER
//
//////////////////////////////////////////////////////////
int handleMessage(char *szBuffer)
{
	/* FILE *fo = fopen("midipipe.txt", "at");
	fprintf(fo, "<%s\n", szBuffer);
	fclose(fo);
	*/

	_strlwr(szBuffer);
	if(isdigit(szBuffer[0]))
	{
		return sendChannelMessage(szBuffer);
	}
	else if(strncmp(szBuffer, "inpt ", 5) == 0)
	{
		return StartMidiIn(&szBuffer[5]);
	}
	else if(strncmp(szBuffer, "outp ", 5) == 0)
	{
		return StartMidiOut(&szBuffer[5]);
	}
	else if(strncmp(szBuffer, "chan ", 5) == 0)
	{
		channel = (int)szBuffer[5];
		fprintf(stdout, "*** channel=%lx\n", channel);
		//lpfnDllSetData(channel, data);
		return 1;
	}
	else if(strncmp(szBuffer, "data ", 5) == 0)
	{
		data = (int)szBuffer[5];
		fprintf(stdout, "*** data=%lx\n", data);
		lpfnDllSetData(channel, data);
		return 1;
	}
	else if(strcmp(szBuffer, "close") == 0)
	{
		StopMidiIn();
		StopMidiOut();
		return 1;
	}
	else if(strcmp(szBuffer, "list") == 0)
	{
		ListMidiDevices();
		return 1;
	}
	else if(strcmp(szBuffer, "quit") == 0)
	{
		return -1;
	}
	else
	{
		fprintf(stdout, "*** unrecognised command\n");
		return 0;
	}
}

//////////////////////////////////////////////////////////
//
// SHUTDOWN HANDLER
//
//////////////////////////////////////////////////////////
void terminalHandler( int sig ) 
{
	StopMidiIn();
	StopMidiOut();
    fclose( stdout );
    exit(1);
}
 

//////////////////////////////////////////////////////////
//
// main
//
//////////////////////////////////////////////////////////
int main(int argc, char** argv)
{

	//hDLL = LoadLibrary((LPCWSTR)"K8062D.dll");
	hDLL = LoadLibrary(L"K8062D.dll");
	if (hDLL != NULL)
	{
		fprintf(stdout,"*** hDLL");
		lpfnDllStartDevice = (LPFNDLLSTARTDEVICE)GetProcAddress(hDLL,"StartDevice");
		lpfnDllStopDevice = (LPFNDLLSTOPDEVICE)GetProcAddress(hDLL,"StopDevice");
		lpfnDllSetData = (LPFNDLLSETDATA)GetProcAddress(hDLL,"SetData");

		if (!lpfnDllStartDevice)
		{
			// handle the error
			FreeLibrary(hDLL);       
		}
		else
		{
			// call the function
			lpfnDllStartDevice();
		}
		if (!lpfnDllSetData)
		{
			// handle the error
			FreeLibrary(hDLL);       
		}
		else
		{
			// call the function
			channel = 2;
			data = 1;
			lpfnDllSetData(channel, data);
		}
	}
	else
	{
		fprintf(stdout,"*** !hDLL");
	}
    _setmode( _fileno( stdin ), _O_BINARY );
    _setmode( _fileno( stdout ), _O_BINARY );
	setvbuf( stdout, NULL, _IONBF, 0 );

    signal( SIGABRT, terminalHandler );
    signal( SIGTERM, terminalHandler );
    signal( SIGINT, terminalHandler );

    // close the pipe to exit the app
    while ( !feof( stdin ) )
    {
		/*for (int i=0;i<254;i++)
		{

			channel++;
			if (channel>16) channel = 1;
			data++;
			if (data>254) data = 1;
			lpfnDllSetData(channel, i);
		}*/
		// poll for 
		char ch;
		int iCharsRead = fread( &ch, 1, 1, stdin);
		//fprintf(stdout, "*** iCharsRead=%lx\n", iCharsRead);
		//fprintf(stdout, "*** ch=%c\n", ch);
		//data++;
		//if (data>254) data = 1;
		//lpfnDllSetData(channel, data);
        if( ferror( stdin ))
        {
           perror("*** read from stdin failed");
           exit(1);
        }
		else if(iBufferIndex >= BUF_SIZE-1)
		{
           perror("*** input buffer overflow");
           exit(2);
		}
		else if(ch == '\n')
		{
			szBuffer[iBufferIndex] = '\0';
			if(handleMessage(szBuffer)<0)
				break;
			iBufferIndex = 0;
		}
		else if(ch != '\r')
		{
			szBuffer[iBufferIndex++] = ch;
		}
	}
	if (hDLL != NULL)
	{
		if (!lpfnDllStopDevice)
		{
			fprintf(stdout,"*** !lpfnDllStopDevice");
			// handle the error
			FreeLibrary(hDLL);       
			//return SOME_ERROR_CODE;
		}
		else
		{
			fprintf(stdout,"*** lpfnDllStopDevice");
			// call the function
			lpfnDllStopDevice();
		}
	}
	
    return 0;
}
/*
en gros, le dmx 512 est une boucle continue de 512 trames (appellees canaux) de valeurs comprises entre 0 et 255.
chaque appareil possede un certain nombre de canaux definis.
par exemple, ton petit projo a peut etre 4 canaux : 1/ intensité 2/ rouge 3/ vert 4/ bleu.
chaque appareil a une adresse de depart definie.
si tu veux que tous tes projos s'allument en meme temps avec les memes valeurs, tu leur donne la meme adresse de depart.
mais si tu veux les controler separement, tu dois leur donner une adresse differente.
si l'appareil possede 4 canaux, tu mets le premier sur 1 et le deuxieme sur 5 etc...
donc si tu mets le canal 1 à 255 tu auras l'intensité à donf sur la premiere becane et si tu mets le canal 5 a 255 tu auras l'intensité à donf sur la deuxieme becane.
si tu veux les allumer les deux en rouge, tu mettras les canaux 1 et 5 à 255 (pour l'intensité) et 2 et 6 à 255 pour le rouge.
apres, niveau programmation, l'habitude est d'utiliser des "scenes" avec temps de montee, descente etc... mais c'est une prise de tete a programmer (au cas, tu as des editeurs dmx opensource comme qlite mais pas en C#)

*/