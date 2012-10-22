#ifndef __ONYXDMX_H__
#define __ONYXDMX_H__

#include <queue>
typedef std::queue<DWORD> CDWORDQueue;
class CMidiEventQueue: protected CDWORDQueue
{
    CRITICAL_SECTION m_critSec;
public:
    CMidiEventQueue()
    {
        ::InitializeCriticalSection(&m_critSec);
    }
    void PutMsg(DWORD msg)
    {
        ::EnterCriticalSection(&m_critSec);
        if (size() < 5) push(msg);
        ::LeaveCriticalSection(&m_critSec);
    }
    BOOL Waiting()
    {
        BOOL b = FALSE;
        ::EnterCriticalSection(&m_critSec);
        if(!empty())
            b = TRUE;
        ::LeaveCriticalSection(&m_critSec);
    }
    DWORD GetMsg()
    {
        DWORD dw = 0;
        ::EnterCriticalSection(&m_critSec);
        if(!CDWORDQueue::empty())
        {
            dw = front();
            pop();
        }
        ::LeaveCriticalSection(&m_critSec);
        return dw;
    }

};

// 6543210
// 0GGabRR  a = clear b = copy
class CMidiInDevice
{
    HMIDIIN m_hMidiIn;
    CMidiEventQueue m_MidiEventQueue;

public:
    CMidiInDevice() : m_hMidiIn(NULL) {}
    ~CMidiInDevice() { Close(); }
    BOOL Open(const WCHAR *wcsDevice);
    BOOL Close();
    DWORD Receive() {return m_MidiEventQueue.GetMsg();}
    void MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD dwParam1, DWORD dwParam2);
};

class CMidiOutDevice
{
    HMIDIOUT m_hMidiOut;
public:
    CMidiOutDevice() : m_hMidiOut(NULL) {}
    ~CMidiOutDevice() { Close(); }
    BOOL Open(const WCHAR *wcsDevice);
    BOOL Close();
    BOOL SendMsg(DWORD msg);
    BOOL SendNote(BYTE uchChannel, BYTE uchNote, BYTE uchVelocity);
    BOOL StopNote(BYTE uchChannel, BYTE uchNote, BYTE uchVelocity);
    BOOL SendController(BYTE uchChannel, BYTE uchController, BYTE uchValue);
};

#endif // __ONYXDMX_H__