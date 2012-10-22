//#ifdef __cplusplus
//extern "C" {         /* Assume C declarations for C++ */
//#endif

//#define FUNCTION __declspec(dllimport)
	//extern "C" __declspec( dllimport ) static void StartDevice();

extern "C" __declspec(dllimport) void StartDevice();
extern "C" __declspec(dllimport) void SetData(long Channel, long Data);
extern "C" __declspec(dllimport) void SetChannelCount(long Count);
extern "C" __declspec(dllimport) void StopDevice();

//#ifdef __cplusplus
//}
//#endif