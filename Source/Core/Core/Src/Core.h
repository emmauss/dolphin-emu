// Copyright 2013 Dolphin Emulator Project
// Licensed under GPLv2
// Refer to the license.txt file included.


// Core

// The external interface to the emulator core. Plus some extras.
// This is another part of the emu that needs cleaning - Core.cpp really has
// too much random junk inside.

#ifndef _CORE_H
#define _CORE_H

#include <vector>
#include <string>
#include <functional>

#include "Common.h"
#include "CoreParameter.h"
#include "Thread.h"

namespace Core
{

// Get core parameters
// TODO: kill, use SConfig instead
extern SCoreStartupParameter g_CoreStartupParameter;

class Thread
{
public:
	void RunCPU()
	{
		if (g_CoreStartupParameter.m_BootType == SCoreStartupParameter::BOOT_DFF)
			cpu.Run(std::mem_fun(&Thread::FIFO), this, "FIFO");
		else
			cpu.Run(std::mem_fun(&Thread::CPU), this, "CPU");
	}

	void RunGPU()
	{
		gpu.Run(std::mem_fun(&Thread::GPU), this, "GPU");
	}

	void CPU();
	void GPU();
	void FIFO();

	Common::Thread cpu;
	Common::Thread gpu;
};

extern bool isTabPressed;

void Callback_VideoCopiedToXFB(bool video_update);

enum EState
{
	CORE_UNINITIALIZED,
	CORE_PAUSE,
	CORE_RUN,
	CORE_STOPPING
};

bool Init();
void Stop();

bool IsRunning();
bool IsRunningAndStarted(); // is running and the cpu loop has been entered
bool IsRunningInCurrentThread(); // this tells us whether we are running in the cpu thread.
bool IsCPUThread(); // this tells us whether we are the cpu thread.
bool IsGPUThread();

void SetState(EState _State);
EState GetState();

void SaveScreenShot();

void Callback_WiimoteInterruptChannel(int _number, u16 _channelID, const void* _pData, u32 _Size);

void* GetWindowHandle();

void StartTrace(bool write);

// This displays messages in a user-visible way.
void DisplayMessage(const char *message, int time_in_ms);

inline void DisplayMessage(const std::string &message, int time_in_ms)
{
	DisplayMessage(message.c_str(), time_in_ms);
}
	
std::string GetStateFileName();
void SetStateFileName(std::string val);

int SyncTrace();
void SetBlockStart(u32 addr);
void StopTrace();

bool ShouldSkipFrame(int skipped);
void VideoThrottle();
void RequestRefreshInfo();

void UpdateTitle();

// waits until all systems are paused and fully idle, and acquires a lock on that state.
// or, if doLock is false, releases a lock on that state and optionally unpauses.
// calls must be balanced (once with doLock true, then once with doLock false) but may be recursive.
// the return value of the first call should be passed in as the second argument of the second call.
bool PauseAndLock(bool doLock, bool unpauseOnUnlock=true);

}  // namespace

#endif

