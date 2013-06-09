/*
 * Copyright (c) 2006, Creative Labs Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided
 * that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and
 * 	     the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 * 	     and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of Creative Labs Inc. nor the names of its contributors may be used to endorse or
 * 	     promote products derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "Common.h"
#include "aldlist.h"
#ifdef _WIN32
#include "../../../../Externals/openal-soft/include/AL/al.h"
#include "../../../../Externals/openal-soft/include/AL/alc.h"
#elif defined(__APPLE__)
#include <al.h>
#include <alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif


/* 
 * Init call
 */
ALDeviceList::ALDeviceList()
{
	ALDEVICEINFO	ALDeviceInfo;
	char *devices;
	s32 index;
	const char *defaultDeviceName = NULL;
	const char *actualDeviceName = NULL;

	// DeviceInfo vector stores, for each enumerated device, it's device name, selection status, spec version #, and extension support
	vDeviceInfo.clear();
	vDeviceInfo.reserve(10);

	defaultDeviceIndex = 0;

	// grab function pointers for 1.0-API functions, and if successful proceed to enumerate all devices
	//if (LoadOAL10Library(NULL, &ALFunction) == TRUE) {
		if (alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT"))
		{
			devices = (char *)alcGetString(NULL, ALC_DEVICE_SPECIFIER);
			defaultDeviceName = (char *)alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
			index = 0;
			// go through device list (each device terminated with a single NULL, list terminated with double NULL)
			while (devices != NULL && strlen(devices) > 0) 
			{
				if (strcmp(defaultDeviceName, devices) == 0) 
				{
					defaultDeviceIndex = index;
				}
				ALCdevice *device = alcOpenDevice(devices);
				if (device) 
				{
					ALCcontext *context = alcCreateContext(device, NULL);
					if (context) 
					{
						alcMakeContextCurrent(context);
						// if new actual device name isn't already in the list, then add it...
						actualDeviceName = alcGetString(device, ALC_DEVICE_SPECIFIER);
						bool bNewName = true;
						for (s32 i = 0; i < GetNumDevices(); i++) 
						{
							if (strcmp(GetDeviceName(i), actualDeviceName) == 0) 
							{
								bNewName = false;
							}
						}
						if ((bNewName) && (actualDeviceName != NULL) && (strlen(actualDeviceName) > 0)) 
						{
							ALDeviceInfo.bSelected = true;
							ALDeviceInfo.strDeviceName = actualDeviceName;
							alcGetIntegerv(device, ALC_MAJOR_VERSION, sizeof(s32), &ALDeviceInfo.iMajorVersion);
							alcGetIntegerv(device, ALC_MINOR_VERSION, sizeof(s32), &ALDeviceInfo.iMinorVersion);

							ALDeviceInfo.pvstrExtensions = new vector<string>;

							// Check for ALC Extensions
							if (alcIsExtensionPresent(device, "ALC_EXT_CAPTURE") == AL_TRUE)
								ALDeviceInfo.pvstrExtensions->push_back("ALC_EXT_CAPTURE");
							if (alcIsExtensionPresent(device, "ALC_EXT_EFX") == AL_TRUE)
								ALDeviceInfo.pvstrExtensions->push_back("ALC_EXT_EFX");

							// Check for AL Extensions
							if (alIsExtensionPresent("AL_EXT_OFFSET") == AL_TRUE)
								ALDeviceInfo.pvstrExtensions->push_back("AL_EXT_OFFSET");

							if (alIsExtensionPresent("AL_EXT_LINEAR_DISTANCE") == AL_TRUE)
								ALDeviceInfo.pvstrExtensions->push_back("AL_EXT_LINEAR_DISTANCE");
							if (alIsExtensionPresent("AL_EXT_EXPONENT_DISTANCE") == AL_TRUE)
								ALDeviceInfo.pvstrExtensions->push_back("AL_EXT_EXPONENT_DISTANCE");
							
							if (alIsExtensionPresent("EAX2.0") == AL_TRUE)
								ALDeviceInfo.pvstrExtensions->push_back("EAX2.0");
							if (alIsExtensionPresent("EAX3.0") == AL_TRUE)
								ALDeviceInfo.pvstrExtensions->push_back("EAX3.0");
							if (alIsExtensionPresent("EAX4.0") == AL_TRUE)
								ALDeviceInfo.pvstrExtensions->push_back("EAX4.0");
							if (alIsExtensionPresent("EAX5.0") == AL_TRUE)
								ALDeviceInfo.pvstrExtensions->push_back("EAX5.0");

							if (alIsExtensionPresent("EAX-RAM") == AL_TRUE)
								ALDeviceInfo.pvstrExtensions->push_back("EAX-RAM");

							// Get Source Count
							ALDeviceInfo.uiSourceCount = GetMaxNumSources();

							vDeviceInfo.push_back(ALDeviceInfo);
						}
						alcMakeContextCurrent(NULL);
						alcDestroyContext(context);
					}
					alcCloseDevice(device);
				}
				devices += strlen(devices) + 1;
				index += 1;
			}
		}
	//}

	ResetFilters();
}

/* 
 * Exit call
 */
ALDeviceList::~ALDeviceList()
{
	for (u32 i = 0; i < vDeviceInfo.size(); i++) {
		if (vDeviceInfo[i].pvstrExtensions) {
			vDeviceInfo[i].pvstrExtensions->clear();
			delete vDeviceInfo[i].pvstrExtensions;
		}
	}

	vDeviceInfo.clear();
}

/*
 * Returns the number of devices in the complete device list
 */
s32 ALDeviceList::GetNumDevices()
{
	return (s32)vDeviceInfo.size();	
}

/* 
 * Returns the device name at an index in the complete device list
 */
char * ALDeviceList::GetDeviceName(s32 index)
{
	if (index < GetNumDevices())
		return (char *)vDeviceInfo[index].strDeviceName.c_str();
	else
		return NULL;
}

/*
 * Returns the major and minor version numbers for a device at a specified index in the complete list
 */
void ALDeviceList::GetDeviceVersion(s32 index, s32 *major, s32 *minor)
{
	if (index < GetNumDevices()) {
		if (major)
			*major = vDeviceInfo[index].iMajorVersion;
		if (minor)
			*minor = vDeviceInfo[index].iMinorVersion;
	}
	return;
}

/*
 * Returns the maximum number of Sources that can be generate on the given device
 */
u32 ALDeviceList::GetMaxNumSources(s32 index)
{
	if (index < GetNumDevices())
		return vDeviceInfo[index].uiSourceCount;
	else
		return 0;
}

/*
 * Checks if the extension is supported on the given device
 */
bool ALDeviceList::IsExtensionSupported(s32 index, char *szExtName)
{
	bool bReturn = false;

	if (index < GetNumDevices()) {
		for (u32 i = 0; i < vDeviceInfo[index].pvstrExtensions->size(); i++) {
			if (!strcasecmp(vDeviceInfo[index].pvstrExtensions->at(i).c_str(), szExtName)) {
				bReturn = true;
				break;
			}				
		}
	}

	return bReturn;
}

/*
 * returns the index of the default device in the complete device list
 */
s32 ALDeviceList::GetDefaultDevice()
{
	return defaultDeviceIndex;
}

/* 
 * Deselects devices which don't have the specified minimum version
 */
void ALDeviceList::FilterDevicesMinVer(s32 major, s32 minor)
{
	s32 dMajor = 0, dMinor = 0;
	for (u32 i = 0; i < vDeviceInfo.size(); i++) {
		GetDeviceVersion(i, &dMajor, &dMinor);
		if ((dMajor < major) || ((dMajor == major) && (dMinor < minor))) {
			vDeviceInfo[i].bSelected = false;
		}
	}
}

/* 
 * Deselects devices which don't have the specified maximum version
 */
void ALDeviceList::FilterDevicesMaxVer(s32 major, s32 minor)
{
	s32 dMajor = 0, dMinor = 0;
	for (u32 i = 0; i < vDeviceInfo.size(); i++) {
		GetDeviceVersion(i, &dMajor, &dMinor);
		if ((dMajor > major) || ((dMajor == major) && (dMinor > minor))) {
			vDeviceInfo[i].bSelected = false;
		}
	}
}

/*
 * Deselects device which don't support the given extension name
 */
void ALDeviceList::FilterDevicesExtension(char *szExtName)
{
	bool bFound;

	for (u32 i = 0; i < vDeviceInfo.size(); i++) {
		bFound = false;
		for (u32 j = 0; j < vDeviceInfo[i].pvstrExtensions->size(); j++) {
			if (!strcasecmp(vDeviceInfo[i].pvstrExtensions->at(j).c_str(), szExtName)) {
				bFound = true;
				break;
			}
		}
		if (!bFound)
			vDeviceInfo[i].bSelected = false;
	}
}

/*
 * Resets all filtering, such that all devices are in the list
 */
void ALDeviceList::ResetFilters()
{
	for (s32 i = 0; i < GetNumDevices(); i++) {
		vDeviceInfo[i].bSelected = true;
	}
	filterIndex = 0;
}

/*
 * Gets index of first filtered device
 */
s32 ALDeviceList::GetFirstFilteredDevice()
{
	s32 i;

	for (i = 0; i < GetNumDevices(); i++) {
		if (vDeviceInfo[i].bSelected == true) {
			break;
		}
	}
	filterIndex = i + 1;
	return i;
}

/*
 * Gets index of next filtered device
 */
s32 ALDeviceList::GetNextFilteredDevice()
{
	s32 i;

	for (i = filterIndex; i < GetNumDevices(); i++) {
		if (vDeviceInfo[i].bSelected == true) {
			break;
		}
	}
	filterIndex = i + 1;
	return i;
}

/*
 * Internal function to detemine max number of Sources that can be generated
 */
u32 ALDeviceList::GetMaxNumSources()
{
	ALuint uiSources[256];
	u32 iSourceCount = 0;

	// Clear AL Error Code
	alGetError();

	// Generate up to 256 Sources, checking for any errors
	for (iSourceCount = 0; iSourceCount < 256; iSourceCount++)
	{
		alGenSources(1, &uiSources[iSourceCount]);
		if (alGetError() != AL_NO_ERROR)
			break;
	}

	// Release the Sources
	alDeleteSources(iSourceCount, uiSources);
	if (alGetError() != AL_NO_ERROR)
	{
		for (u32 i = 0; i < 256; i++)
		{
			alDeleteSources(1, &uiSources[i]);
		}
	}

	return iSourceCount;
}
