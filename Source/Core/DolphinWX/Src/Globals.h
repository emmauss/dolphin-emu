// Copyright 2013 Dolphin Emulator Project
// Licensed under GPLv2
// Refer to the license.txt file included.


// This file holds global data for DolphinWx and DebuggerWx


#ifndef _WX_GLOBALS_H
#define _WX_GLOBALS_H

#include "Common.h"

enum
{
	Toolbar_Step,
	Toolbar_StepOver,
	Toolbar_Skip,
	Toolbar_GotoPC,
	Toolbar_SetPC,
	ToolbarDebugBitmapMax
};

enum
{
	// emulation
	IDM_LOADSTATE = 200,
	IDM_SAVESTATE,
	IDM_SAVEFIRSTSTATE,
	IDM_UNDOLOADSTATE,
	IDM_UNDOSAVESTATE,
	IDM_LOADSTATEFILE,
	IDM_SAVESTATEFILE,
	IDM_SAVESLOT1,
	IDM_SAVESLOT2,
	IDM_SAVESLOT3,
	IDM_SAVESLOT4,
	IDM_SAVESLOT5,
	IDM_SAVESLOT6,
	IDM_SAVESLOT7,
	IDM_SAVESLOT8,
	IDM_LOADSLOT1,
	IDM_LOADSLOT2,
	IDM_LOADSLOT3,
	IDM_LOADSLOT4,
	IDM_LOADSLOT5,
	IDM_LOADSLOT6,
	IDM_LOADSLOT7,
	IDM_LOADSLOT8,
	IDM_LOADLAST1,
	IDM_LOADLAST2,
	IDM_LOADLAST3,
	IDM_LOADLAST4,
	IDM_LOADLAST5,
	IDM_LOADLAST6,
	IDM_LOADLAST7,
	IDM_LOADLAST8,

	IDM_FRAMESKIP0,
	IDM_FRAMESKIP1,
	IDM_FRAMESKIP2,
	IDM_FRAMESKIP3,
	IDM_FRAMESKIP4,
	IDM_FRAMESKIP5,
	IDM_FRAMESKIP6,
	IDM_FRAMESKIP7,
	IDM_FRAMESKIP8,
	IDM_FRAMESKIP9,

	IDM_PLAY,
	IDM_STOP,
	IDM_RESET,
	IDM_TOGGLE_FULLSCREEN,
	IDM_RECORD,
	IDM_PLAYRECORD,
	IDM_RECORDEXPORT,
	IDM_RECORDREADONLY,
	IDM_TASINPUT,
	IDM_TOGGLE_PAUSEMOVIE,
	IDM_SHOWLAG,
	IDM_FRAMESTEP,
	IDM_SCREENSHOT,
	IDM_BROWSE,
	IDM_DRIVES,
	IDM_DRIVE1,
	IDM_DRIVE24 = IDM_DRIVE1 + 23,//248,

	// options
	IDM_CONFIG_GFX_BACKEND,
	IDM_CONFIG_DSP_EMULATOR,
	IDM_CONFIG_PAD_PLUGIN,
	IDM_CONFIG_WIIMOTE_PLUGIN,
	IDM_CONFIG_HOTKEYS,

	// tools
	IDM_MEMCARD,
	IDM_CHEATS,
	IDM_NETPLAY,
	IDM_RESTART,
	IDM_CHANGEDISC,
	IDM_PROPERTIES,
	IDM_GAMEWIKI,
	IDM_LOAD_WII_MENU,
	IDM_MENU_INSTALLWAD,
	IDM_LIST_INSTALLWAD,
	IDM_FIFOPLAYER,
	IDM_CONNECT_WIIMOTE1,
	IDM_CONNECT_WIIMOTE2,
	IDM_CONNECT_WIIMOTE3,
	IDM_CONNECT_WIIMOTE4,

	IDM_LISTWAD,
	IDM_LISTWII,
	IDM_LISTGC,
	IDM_LISTJAP,
	IDM_LISTPAL,
	IDM_LISTUSA,
	IDM_LISTFRANCE,
	IDM_LISTITALY,
	IDM_LISTKOREA,
	IDM_LISTTAIWAN,
	IDM_LIST_UNK,
	IDM_LISTDRIVES,
	IDM_PURGECACHE,

	// view
	IDM_LOGWINDOW,
	IDM_LOGCONFIGWINDOW,
	IDM_CONSOLEWINDOW,
	IDM_REGISTERWINDOW,
	IDM_BREAKPOINTWINDOW,
	IDM_MEMORYWINDOW,
	IDM_JITWINDOW,
	IDM_SOUNDWINDOW,
	IDM_VIDEOWINDOW,
	IDM_CODEWINDOW,

	// float Window IDs
	IDM_LOGWINDOW_PARENT,
	IDM_LOGCONFIGWINDOW_PARENT,
	IDM_CONSOLEWINDOW_PARENT,
	IDM_REGISTERWINDOW_PARENT,
	IDM_BREAKPOINTWINDOW_PARENT,
	IDM_MEMORYWINDOW_PARENT,
	IDM_JITWINDOW_PARENT,
	IDM_SOUNDWINDOW_PARENT,
	IDM_VIDEOWINDOW_PARENT,
	IDM_CODEWINDOW_PARENT,

	// float popup menu IDs
	IDM_FLOAT_LOGWINDOW,
	IDM_FLOAT_LOGCONFIGWINDOW,
	IDM_FLOAT_CONSOLEWINDOW,
	IDM_FLOAT_REGISTERWINDOW,
	IDM_FLOAT_BREAKPOINTWINDOW,
	IDM_FLOAT_MEMORYWINDOW,
	IDM_FLOAT_JITWINDOW,
	IDM_FLOAT_SOUNDWINDOW,
	IDM_FLOAT_VIDEOWINDOW,
	IDM_FLOAT_CODEWINDOW,

	// --------------------------------------------------------------
	// debug
	// --------------------

	// menu

	IDM_DEBUG_BEGIN,

	// options
	IDM_FONTPICKER,

	// CPU
	IDM_CPU_BEGIN,
	IDM_BOOTTOPAUSE,
	IDM_AUTOMATICSTART,
	IDM_LOG_MEMORY,
	IDM_LOG_GECKO,
	IDM_JIT_CLEAR_CACHE,
	IDM_LOGINSTRUCTIONS,
	IDM_SEARCHINSTRUCTION,
	IDM_CPU_END,

	// change CPU
	IDM_CPU_CHANGE_BEGIN,
	IDM_DEBUGGING,
	IDM_INTERPRETER,
	IDM_COMPILER_DEFAULT,
	IDM_COMPILER_IL,
	IDM_SKIP_IDLE,
	IDM_JIT_BLOCK_LINK,
	IDM_JIT_LARGE_CACHE,
	IDM_JIT_BRANCH,
	IDM_CPU_CHANGE_END,

	// refresh CPU
	IDM_CPU_REFRESH_BEGIN,
	IDM_JIT,
	IDM_JIT_LS,
	IDM_JIT_LS_LXZ,
	IDM_JIT_LS_LWZ,
	IDM_JIT_LS_LBZX,
	IDM_JIT_LS_F,
	IDM_JIT_LS_P,
	IDM_JIT_I,
	IDM_JIT_FP,
	IDM_JIT_P,
	IDM_JIT_SR,
	IDM_CPU_REFRESH_END,

	// symbols
	IDM_CLEARSYMBOLS,
	IDM_SCANFUNCTIONS,
	IDM_LOADMAPFILE,
	IDM_SAVEMAPFILE, IDM_SAVEMAPFILEWITHCODES,
	IDM_CREATESIGNATUREFILE,
	IDM_RENAME_SYMBOLS,
	IDM_USESIGNATUREFILE,
	IDM_PATCHHLEFUNCTIONS,

	// profiler
	IDM_PROFILE_BEGIN,
	IDM_PROFILEBLOCKS,
	IDM_WRITEPROFILE,
	IDM_PROFILE_END,

	// toolbar
	IDT_DEBUG_BEGIN,
	IDM_STEP,
	IDM_STEPOVER,
	IDM_TOGGLE_BREAKPOINT,
	IDM_SKIP,
	IDM_SETPC,
	IDM_GOTO,
	IDM_GOTOPC,
	IDT_DEBUG_END,

	IDM_DEBUG_END,

	// other
	IDM_ADDRBOX,
	ID_TOOLBAR_DEBUG,
	// --------------------------------------------------------------

	// help
	IDM_HELPWEBSITE,
	IDM_HELPGOOGLECODE,

	// aui
	ID_TOOLBAR_AUI,
	IDM_SAVE_PERSPECTIVE,
	IDM_ADD_PERSPECTIVE,
	IDM_PERSPECTIVES_ADD_PANE,
	IDM_EDIT_PERSPECTIVES,
	IDM_TAB_SPLIT,
	IDM_NO_DOCKING,
	IDM_PERSPECTIVES_0,
	IDM_PERSPECTIVES_100 = IDM_PERSPECTIVES_0 + 100,

	// other
	IDM_CONFIG_LOGGER,
	IDM_TOGGLE_DUALCORE,
	IDM_TOGGLE_SKIPIDLE,
	IDM_TOGGLE_TOOLBAR,
	IDM_TOGGLE_STATUSBAR,
	IDM_NOTIFYMAPLOADED,
	IDM_OPENCONTAININGFOLDER,
	IDM_OPENSAVEFOLDER,
	IDM_EXPORTSAVE,
	IDM_IMPORTSAVE,
	IDM_SETDEFAULTGCM,
	IDM_DELETEGCM,
	IDM_COMPRESSGCM,
	IDM_MULTICOMPRESSGCM,
	IDM_MULTIDECOMPRESSGCM,
	IDM_UPDATELOGDISPLAY,
	IDM_UPDATEDISASMDIALOG,
	IDM_UPDATEGUI,
	IDM_UPDATESTATUSBAR,
	IDM_UPDATETITLE,
	IDM_UPDATEBREAKPOINTS,
	IDM_PANIC,
	IDM_KEYSTATE,
	IDM_WINDOWSIZEREQUEST,
	IDM_HOST_MESSAGE,

	IDM_MPANEL, ID_STATUSBAR,

	ID_TOOLBAR = 500,
	LIST_CTRL = 1000
};

#include <wx/wx.h>
#include <wx/toolbar.h>
#include <wx/log.h>
#include <wx/image.h>
#include <wx/aboutdlg.h>
#include <wx/filedlg.h>
#include <wx/spinctrl.h>
#include <wx/srchctrl.h>
#include <wx/listctrl.h>
#include <wx/progdlg.h>
#include <wx/imagpng.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/cmdline.h>
#include <wx/busyinfo.h>

// custom message macro
#define EVT_HOST_COMMAND(id, fn) \
	DECLARE_EVENT_TABLE_ENTRY(\
			wxEVT_HOST_COMMAND, id, wxID_ANY, \
			(wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent(wxCommandEventFunction, &fn), \
			(wxObject*) NULL \
			),

extern const wxEventType wxEVT_HOST_COMMAND;

#endif // _WX_GLOBALS_H
