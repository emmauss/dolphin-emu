// Copyright 2013 Dolphin Emulator Project
// Licensed under GPLv2
// Refer to the license.txt file included.
#pragma once
#include "CommonTypes.h"

namespace DriverDetails
{
	// Enum of known vendors
	// Tegra and Nvidia are separated out due to such substantial differences
	enum Vendor
	{
		VENDOR_NVIDIA = 0,
		VENDOR_ATI,
		VENDOR_INTEL,
		VENDOR_ARM,
		VENDOR_QUALCOMM,
		VENDOR_IMGTEC,
		VENDOR_TEGRA,
		VENDOR_VIVANTE,
		VENDOR_UNKNOWN
	};

	// Enum of known bugs
	// These can be vendor specific, but we put them all in here
	// For putting a new bug in here, make sure to put a detailed comment above the enum
	// This'll ensure we know exactly what the issue is.
	enum Bug
	{
		// Bug: No Dynamic UBO array object access
		// Affected Devices: Qualcomm/Adreno
		// Started Version: 14
		// Ended Version: -1
		// Accessing UBO array members dynamically causes the Adreno shader compiler to crash
		// Errors out with "Internal Error"
		BUG_NODYNUBOACCESS = 0,
		// Bug: Centroid is broken in shaders
		// Affected devices: Qualcomm/Adreno
		// Started Version: 14
		// Ended Version: -1
		// Centroid in/out, used in the shaders, is used for multisample buffers to get the texel correctly
		// When MSAA is disabled, it acts like a regular in/out
		// Tends to cause the driver to render full white or black
		BUG_BROKENCENTROID,
		// Bug: INFO_LOG_LENGTH broken
		// Affected devices: Qualcomm/Adreno
		// Started Version: ? (Noticed on v14)
		// Ended Version: -1
		// When compiling a shader, it is important that when it fails, 
		// you first get the length of the information log prior to grabbing it.
		// This allows you to allocate an array to store all of the log
		// Adreno devices /always/ return 0 when querying GL_INFO_LOG_LENGTH
		// They also max out at 1024 bytes(1023 characters + null terminator) for the log
		BUG_BROKENINFOLOG,
		// Bug: Uploading data with rendering causes issues
		// Affected devices: Qualcomm/Adreno
		// Started Version: 14
		// Ended Version: -1
		// When drawing our elements, the instruction buffer on Adreno devices
		// becomes too long, causing the device to quickly run out of RAM
		// I've watched the kernel module go up to ~700MB of RAM in a few seconds
		// The "workaround" is calling swapbuffers every single time we flush
		// This causes flickering, but it is the only known way to work around it
		BUG_BROKENBUFFERS,
	};
	
	// Initializes our internal vendor, device family, and driver version	
	void Init(Vendor vendor, const u32 devfamily, const double version);
	
	// Once Vendor and driver version is set, this will return if it has the applicable bug passed to it.
	bool HasBug(Bug bug);
}
