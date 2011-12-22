// Copyright (C) 2003 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/


/* HID reports access guide. */

/* 0x10 - 0x1a   Output   EmuMain.cpp: HidOutputReport()
       0x10 - 0x14: General
	   0x15: Status report request from the Wii
	   0x16 and 0x17: Write and read memory or registers
       0x19 and 0x1a: General
   0x20 - 0x22   Input    EmuMain.cpp: HidOutputReport() to the destination
       0x15 leads to a 0x20 Input report
       0x17 leads to a 0x21 Input report
	   0x10 - 0x1a leads to a 0x22 Input report
   0x30 - 0x3f   Input    This file: Update() */

#include <vector>
#include <string>
#include <fstream>

#include "Common.h"
#include "FileUtil.h"

#include "../Wiimote.h"
#include "WiimoteEmu.h"
#include "WiimoteHid.h"
#include "../WiimoteReal/WiimoteReal.h"

#include "Attachment/Attachment.h"

namespace WiimoteEmu
{

void Wiimote::ReportMode(const wm_report_mode* const dr)
{
	//INFO_LOG(WIIMOTE, "Set data report mode");
	//DEBUG_LOG(WIIMOTE, "  Rumble: %x", dr->rumble);
	//DEBUG_LOG(WIIMOTE, "  Continuous: %x", dr->continuous);
	//DEBUG_LOG(WIIMOTE, "  All The Time: %x", dr->all_the_time);
	//DEBUG_LOG(WIIMOTE, "  Mode: 0x%02x", dr->mode);

	//m_reporting_auto = dr->all_the_time;
	m_reporting_auto = dr->continuous;	// this right?
	m_reporting_mode = dr->mode;
	//m_reporting_channel = _channelID;	// this is set in every Interrupt/Control Channel now

	// reset IR camera
	//memset(m_reg_ir, 0, sizeof(*m_reg_ir));  //ugly hack

	if (dr->mode > 0x37)
		PanicAlert("Wiimote: Unsupported Reporting mode.");
	else if (dr->mode < WM_REPORT_CORE)
		PanicAlert("Wiimote: Reporting mode < 0x30.");
}

/* Here we process the Output Reports that the Wii sends. Our response will be
   an Input Report back to the Wii. Input and Output is from the Wii's
   perspective, Output means data to the Wiimote (from the Wii), Input means
   data from the Wiimote.
   
   The call browser:

   1. Wiimote_InterruptChannel > InterruptChannel > HidOutputReport
   2. Wiimote_ControlChannel > ControlChannel > HidOutputReport

   The IR enable/disable and speaker enable/disable and mute/unmute values are
		bit2: 0 = Disable (0x02), 1 = Enable (0x06)
*/
void Wiimote::HidOutputReport(const wm_report* const sr, const bool send_ack)
{
	INFO_LOG(WIIMOTE, "HidOutputReport (page: %i, cid: 0x%02x, wm: 0x%02x)", m_index, m_reporting_channel, sr->wm);

	// wiibrew:
	// In every single Output Report, bit 0 (0x01) of the first byte controls the Rumble feature.
	m_rumble_on = sr->rumble;

	switch (sr->wm)
	{
	case WM_RUMBLE : // 0x10
		// this is handled above
		return;	// no ack
		break;

	case WM_LEDS : // 0x11
		//INFO_LOG(WIIMOTE, "Set LEDs: 0x%02x", sr->data[0]);
		m_status.leds = sr->data[0] >> 4;
		break;

	case WM_REPORT_MODE :  // 0x12
		ReportMode((wm_report_mode*)sr->data);
		break;

	case WM_IR_PIXEL_CLOCK : // 0x13
		//INFO_LOG(WIIMOTE, "WM IR Clock: 0x%02x", sr->data[0]);
		//m_ir_clock = sr->enable;
		if (false == sr->ack)
			return;
		break;

	case WM_SPEAKER_ENABLE : // 0x14
		//ERROR_LOG(WIIMOTE, "WM Speaker Enable: %02x", sr->enable);
		//PanicAlert( "WM Speaker Enable: %d", sr->data[0] );
		m_status.speaker = sr->enable;
		if (false == sr->ack)
			return;
		break;

	case WM_REQUEST_STATUS : // 0x15
		RequestStatus((wm_request_status*)sr->data);
		return;	// sends its own ack
		break;

	case WM_WRITE_DATA : // 0x16
		WriteData((wm_write_data*)sr->data);
		break;

	case WM_READ_DATA : // 0x17
		ReadData((wm_read_data*)sr->data);
		return;	// sends its own ack
		break;

	case WM_WRITE_SPEAKER_DATA : // 0x18
		{
		//wm_speaker_data *spkz = (wm_speaker_data*)sr->data;
		//ERROR_LOG(WIIMOTE, "WM_WRITE_SPEAKER_DATA len:%x %s", spkz->length,
		//	ArrayToString(spkz->data, spkz->length, 100, false).c_str());
		Wiimote::SpeakerData((wm_speaker_data*)sr->data);
		}
		return;	// no ack
		break;

	case WM_SPEAKER_MUTE : // 0x19
		//ERROR_LOG(WIIMOTE, "WM Speaker Mute: %02x", sr->enable);
		//PanicAlert( "WM Speaker Mute: %d", sr->data[0] & 0x04 );
		// testing
		//if (sr->data[0] & 0x04)
		//	memset(&m_channel_status, 0, sizeof(m_channel_status));
		m_speaker_mute = sr->enable;
		if (false == sr->ack)
			return;
		break;

	case WM_IR_LOGIC: // 0x1a
		// comment from old plugin:
		// This enables or disables the IR lights, we update the global variable g_IR
	    // so that WmRequestStatus() knows about it
		//INFO_LOG(WIIMOTE, "WM IR Enable: 0x%02x", sr->data[0]);
		m_status.ir = sr->enable;
		if (false == sr->ack)
			return;
		break;

	default:
		PanicAlert("HidOutputReport: Unknown channel 0x%02x", sr->wm);
		return; // no ack
		break;
	}

	// send ack
	if (send_ack)
		SendAck(sr->wm);
}

/* This will generate the 0x22 acknowledgement for most Input reports.
   It has the form of "a1 22 00 00 _reportID 00".
   The first two bytes are the core buttons data,
   00 00 means nothing is pressed.
   The last byte is the success code 00. */
void Wiimote::SendAck(u8 _reportID)
{
	u8 data[6];

	data[0] = 0xA1;
	data[1] = WM_ACK_DATA;

	wm_acknowledge* const ack = (wm_acknowledge*)(data + 2);

	ack->buttons = m_status.buttons;
	ack->reportID = _reportID;
	ack->errorID = 0;

	::Wiimote::Eavesdrop(this, data, (int)sizeof(data));
	Core::Callback_WiimoteInterruptChannel( m_index, m_reporting_channel, data, sizeof(data));
}

void Wiimote::HandleExtensionSwap()
{
	// m+ switch
	if (GetMotionPlusActive() && !GetMotionPlusAttached()) {
		m_reg_motion_plus.ext_identifier[2] = 0xa6;
	}
	// handle switch extension
	if (m_extension->active_extension != m_extension->switch_extension)
	{
		// if an extension is currently connected and we want to switch to a different extension
		if ((m_extension->active_extension > 0) && m_extension->switch_extension)
			// detach extension first, wait til next Update() or RequestStatus() call to change to the new extension
			m_extension->active_extension = 0;
		else
			// set the wanted extension
			m_extension->active_extension = m_extension->switch_extension;

		// set register, I hate this
		const std::vector<u8> &reg = ((WiimoteEmu::Attachment*)m_extension->attachments[m_extension->active_extension])->reg;
		memset(&m_reg_ext, 0, WIIMOTE_REG_EXT_SIZE);
		memcpy(&m_reg_ext, &reg[0], reg.size());			
		if(m_extension->active_extension == EXT_NUNCHUK && GetMotionPlusAttached()) {
			memcpy(&m_reg_motion_plus.ext_calib, nunchuck_calibration, sizeof(nunchuck_calibration));			
		}
	}
}

// old comment
/* Here we produce a 0x20 status report to send to the Wii. We currently ignore
   the status request rs and all its eventual instructions it may include (for
   example turn off rumble or something else) and just send the status
   report. */
void Wiimote::RequestStatus(const wm_request_status* const rs, int ext)
{
	HandleExtensionSwap();

	// update status struct
	if (ext == -1)
		m_status.extension = (m_extension->active_extension || GetMotionPlusActive()) ? 1 : 0;
	else
		m_status.extension = ext;

	m_status.speaker = m_status.speaker;

	// set up report
	u8 data[8];
	data[0] = 0xA1;
	data[1] = WM_STATUS_REPORT;

	// status values
	*(wm_status_report*)(data + 2) = m_status;

	// hybrid wiimote stuff
	if (WIIMOTE_SRC_HYBRID == g_wiimote_sources[m_index] && (m_extension->switch_extension <= 0))
	{
		using namespace WiimoteReal;

		std::lock_guard<std::mutex> lk(g_refresh_lock);

		if (g_wiimotes[m_index])
		{
			wm_request_status rpt;
			rpt.rumble = 0;
			g_wiimotes[m_index]->SendPacket(WM_REQUEST_STATUS, &rpt, sizeof(rpt));
		}

		return;
	}

	// send report
	::Wiimote::Eavesdrop(this, data, sizeof(data));
	Core::Callback_WiimoteInterruptChannel(m_index, m_reporting_channel, data, sizeof(data));
}

/* Write data to Wiimote and Extensions registers. */
void Wiimote::WriteData(const wm_write_data* const wd)
{
	u32 address = swap24(wd->address);
	u8 addressHI = (address >> 16) & 0xFE;
	u8 addressLO = address & 0xFFFF;
	address &= ~0x010000;

	if (wd->size > 16)
	{
		PanicAlert("WriteData: size is > 16 bytes");
		return;
	}

	switch (wd->space)
	{
	case WM_SPACE_EEPROM :
		{
			// Write to EEPROM

			if (address + wd->size > WIIMOTE_EEPROM_SIZE)
			{
				ERROR_LOG(WIIMOTE, "WriteData: address + size out of bounds!");
				PanicAlert("WriteData: address + size out of bounds!");
				return;
			}
			memcpy(m_eeprom + address, wd->data, wd->size);

			// write mii data to file
			// i need to improve this greatly
			if (address >= 0x0FCA && address < 0x12C0)
			{
				// writing the whole mii block each write :/
				std::ofstream file;
				file.open((File::GetUserPath(D_WIIUSER_IDX) + "mii.bin").c_str(), std::ios::binary | std::ios::out);
				file.write((char*)m_eeprom + 0x0FCA, 0x02f0);
				file.close();
			}
		}
		break;
		
	case WM_SPACE_REGS1 :
	case WM_SPACE_REGS2 :
		{
			// Write to Control Register
			const u8 region_offset = (u8)address;
			void *region_ptr = NULL;
			int region_size = 0;

			switch (addressHI)
			{
			// speaker
			case 0xa2 :
				region_ptr = &m_reg_speaker;
				region_size = WIIMOTE_REG_SPEAKER_SIZE;
				break;

			// extension register
			case 0xa4 :
				region_ptr = GetMotionPlusActive() ? (void*)&m_reg_motion_plus : (void*)&m_reg_ext;
				region_size = WIIMOTE_REG_EXT_SIZE;
				break;

			// motion plus
			case 0xa6 :
				if (!GetMotionPlusActive() && GetMotionPlusAttached())
				{
					region_ptr = &m_reg_motion_plus;
					region_size = WIIMOTE_REG_EXT_SIZE;
				}
				break;

			// ir
			case 0xB0 :
				region_ptr = &m_reg_ir;
				region_size = WIIMOTE_REG_IR_SIZE;

				//if (5 == m_reg_ir->mode)
				//	PanicAlert("IR Full Mode is Unsupported!");
				break;
			}

			if (region_ptr && (region_offset + wd->size <= region_size))
			{
				memcpy((u8*)region_ptr + region_offset, wd->data, wd->size);
			}
			else
				return;	// TODO: generate a writedata error reply

			/* TODO?
			if (region_ptr == &m_reg_speaker)
			{
				ERROR_LOG(WIIMOTE, "Write to speaker reg %x %s", address,
					ArrayToString(wd->data, wd->size, 100, false).c_str());
			}
			*/

			if (&m_reg_ext == region_ptr)
			{
				// Run the key generation on all writes in the key area, only the last full key will have an effect
				if(addressLO >= 0x40 && addressLO <= 0x4c)
					wiimote_gen_key(&m_ext_key, m_reg_ext.encryption_key);
			}

			else if (&m_reg_motion_plus == region_ptr)
			{
				switch (addressHI)
				{
				case 0xa4:
				switch (region_offset) {
					case 0xfb:
						if (mp_last_write_reg == 0xfe) {
						}
						if (mp_last_write_reg == 0xf0) {
							ERROR_LOG(CONSOLE, "W[0x%02x 0x%02x:%04x]: WM+ already disabled [ext:%i]", wd->data[0], addressHI, region_offset, m_extension->active_extension);							
							m_reg_motion_plus.ext_identifier[4] = 0x05;
							m_reg_motion_plus.state = 0x08;
						}
					break;
					// calibration
					case 0xf1:
						// failed, try again. TODO: make it never fail
						if (!wd->data[0]) {
							m_reg_motion_plus.ext_identifier[2] = 0xa6;
							m_reg_motion_plus.state = 0x08;
							RequestStatus(NULL, 0);
							if (m_extension->active_extension != EXT_NONE) {
								RequestStatus(NULL, 1);
								RequestStatus(NULL, 0);
							}
							RequestStatus(NULL, 1);
						} else {
							m_reg_motion_plus.state = 0x1a;
							memcpy(&m_reg_motion_plus.gyro_calib, motionplus_accel_gyro_syncing2, sizeof(motionplus_accel_gyro_syncing2));
						}
					break;
					case 0xf2:
						if(m_reg_motion_plus.state < 0x0e) {
							m_reg_motion_plus.state = 0x0e;
							memcpy(&m_reg_motion_plus.gyro_calib, motionplus_accel_gyro_syncing, sizeof(motionplus_accel_gyro_syncing));
						}
					break;
				}
				break;
				case 0xa6:
				switch (region_offset) {
					case 0xfe:
					// activate
					if (!GetMotionPlusActive() && 0x05 == wd->data[0])
					{
						ERROR_LOG(CONSOLE, "W[0x%02x 0x%02x:%04x]: Enabling WM+", wd->data[0], addressHI, region_offset);
						m_reg_motion_plus.ext_identifier[2] = 0xa4;
						m_reg_motion_plus.ext_identifier[4] = wd->data[0];
						m_reg_motion_plus.state = 0x08;
						RequestStatus(NULL, 0);
						RequestStatus(NULL, 1);
					}
					break;
				}
				break;			
				}
			}
			mp_last_write_reg = region_offset;
		}
		break;

	default:
		PanicAlert("WriteData: unimplemented parameters!");
		break;
	}
}
// Read data from Wiimote and Extensions registers
void Wiimote::ReadData(const wm_read_data* const rd)
{
	u32 address = swap24(rd->address);
	u16 size = Common::swap16(rd->size);
	// ignore the 0x010000 bit
	address &= 0xFEFFFF;

	// hybrid wiimote stuff
	// relay the read data request to real-wiimote
	if (WIIMOTE_SRC_HYBRID == g_wiimote_sources[m_index] && ((0xA4 != (address >> 16)) || (m_extension->switch_extension <= 0)))
	{
		WiimoteReal::InterruptChannel(m_index, m_reporting_channel, ((u8*)rd) - 2, sizeof(wm_read_data) + 2); // hacky
		
		// don't want emu-wiimote to send reply
		return;
	}

	ReadRequest rr;
	u8 *const block = new u8[size];

	switch (rd->space)
	{
	case WM_SPACE_EEPROM :
		{
			//PanicAlert("ReadData: reading from EEPROM: address: 0x%x size: 0x%x", address, size);
			// Read from EEPROM
			if (address + size >= WIIMOTE_EEPROM_FREE_SIZE) 
			{
				if (address + size > WIIMOTE_EEPROM_SIZE) 
				{
					PanicAlert("ReadData: address + size out of bounds");
					return;
				}
				// generate a read error
				size = 0;
			}

			// read mii data from file
			// i need to improve this greatly
			if (address >= 0x0FCA && address < 0x12C0)
			{
				// reading the whole mii block :/
				std::ifstream file;
				file.open((File::GetUserPath(D_WIIUSER_IDX) + "mii.bin").c_str(), std::ios::binary | std::ios::in);
				file.read((char*)m_eeprom + 0x0FCA, 0x02f0);
				file.close();
			}

			// read mem to be sent to wii
			memcpy(block, m_eeprom + address, size);
		}
		break;

	case WM_SPACE_REGS1 :
	case WM_SPACE_REGS2 :
		{
			// Read from Control Register

			// ignore second byte for extension area
			if (0xA4 == (address >> 16))
				address &= 0xFF00FF;

			const u8 region_offset = (u8)address;
			void *region_ptr = NULL;
			int region_size = 0;

			switch (address >> 16)
			{
			// speaker
			case 0xa2:
				region_ptr = &m_reg_speaker;
				region_size = WIIMOTE_REG_SPEAKER_SIZE;
				break;

			// extension
			case 0xa4:
				region_ptr = GetMotionPlusActive() ? (void*)&m_reg_motion_plus : (void*)&m_reg_ext;
				region_size = WIIMOTE_REG_EXT_SIZE;
				break;

			// motion plus
			case 0xa6:
				// reading from 0xa6 returns error when mplus is activated
				if (false == GetMotionPlusActive())
				{
					region_ptr = &m_reg_motion_plus;
					region_size = WIIMOTE_REG_EXT_SIZE;
				}
				break;

			// ir
			case 0xb0:
				region_ptr = &m_reg_ir;
				region_size = WIIMOTE_REG_IR_SIZE;
				break;
			}

			if (region_ptr && (region_offset + size <= region_size))
			{
				memcpy(block, (u8*)region_ptr + region_offset, size);
			} else
				size = 0;	// generate read error

			if (&m_reg_ext == region_ptr)
			{
				// encrypt
				if (0xaa == m_reg_ext.encryption) {
					wiimote_encrypt(&m_ext_key, block, address&0xffff, (u8)size);
				}
			}
		}
		break;

	default :
		PanicAlert("WmReadData: unimplemented parameters (size: %i, addr: 0x%x)!", size, rd->space);
		break;
	}

	// want the requested address, not the above modified one
	rr.address = swap24(rd->address);
	rr.size = size;
	//rr.channel = _channelID;
	rr.position = 0;
	rr.data = block;

	// send up to 16 bytes
	SendReadDataReply(rr);

	// if there is more data to be sent, add it to the queue
	if (rr.size)
		m_read_requests.push( rr );
	else
		delete[] rr.data;
}

// old comment
/* Here we produce the actual 0x21 Input report that we send to the Wii. The
   message is divided into 16 bytes pieces and sent piece by piece. There will
   be five formatting bytes at the begging of all reports. A common format is
   00 00 f0 00 20, the 00 00 means that no buttons are pressed, the f means 16
   bytes in the message, the 0 means no error, the 00 20 means that the message
   is at the 00 20 offest in the registry that was read.
*/
void Wiimote::SendReadDataReply(ReadRequest& _request)
{
	u8 data[23];
	data[0] = 0xA1;
	data[1] = WM_READ_DATA_REPLY;

	wm_read_data_reply* const reply = (wm_read_data_reply*)(data + 2);
	reply->buttons = m_status.buttons;
	reply->address = Common::swap16(_request.address);

	// generate a read error
	// Out of bounds. The real Wiimote generate an error for the first
	// request to 0x1770 if we dont't replicate that the game will never
	// read the calibration data at the beginning of Eeprom. I think this
	// error is supposed to occur when we try to read above the freely
	// usable space that ends at 0x16ff.
	if (0 == _request.size)
	{
		reply->size = 0x0f;
		reply->error = 0x08;

		memset(reply->data, 0, sizeof(reply->data));
	}
	else
	{
		// Limit the amt to 16 bytes
		// AyuanX: the MTU is 640B though... what a waste!
		const int amt = std::min( (unsigned int)16, _request.size );

		// no error
		reply->error = 0;

		// 0x1 means two bytes, 0xf means 16 bytes
		reply->size = amt - 1;

		// Clear the mem first
		memset(reply->data, 0, sizeof(reply->data));

		// copy piece of mem
		memcpy(reply->data, _request.data + _request.position, amt);

		// update request struct
		_request.size -= amt;
		_request.position += amt;
		_request.address += amt;
	}

	// Send a piece
	::Wiimote::Eavesdrop(this, data, sizeof(data));
	Core::Callback_WiimoteInterruptChannel(m_index, m_reporting_channel, data, sizeof(data));
}

void Wiimote::DoState(PointerWrap& p)
{
	//if (p.MODE_READ == p.GetMode()) Reset();
	p.Do(m_reporting_channel);
	p.Do(m_reporting_mode);
	p.Do(m_reporting_auto);
	p.Do(mp_last_write_reg);
	p.Do(mp_passthrough);
	p.Do(m_ext_key);
	p.DoArray(m_eeprom, sizeof(m_eeprom));
	p.Do(m_reg_ir);
	p.Do(m_reg_speaker);
	p.Do(m_reg_ext);
	p.Do(m_reg_motion_plus);
}

}