// Copyright (C) 2010 Dolphin Project.

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

#ifndef _CONTROLLEREMU_H_
#define _CONTROLLEREMU_H_

// windows crap
#define NOMINMAX

#include <cmath>
#include <vector>
#include <string>
#include <algorithm>

#include "GCPadStatus.h"

#include "ControllerInterface/ControllerInterface.h"
#include "IniFile.h"

#define sign(x) ((x)?(x)<0?-1:1:0)

enum
{
	GROUP_TYPE_OTHER,
	GROUP_TYPE_STICK,
	GROUP_TYPE_MIXED_TRIGGERS,
	GROUP_TYPE_BUTTONS,
	GROUP_TYPE_FORCE,
	GROUP_TYPE_EXTENSION,
	GROUP_TYPE_TILT,
	GROUP_TYPE_CURSOR,
	GROUP_TYPE_TRIGGERS,
	GROUP_TYPE_UDPWII,
	GROUP_TYPE_SLIDER,
};
enum
{
	SETTING_BACKGROUND_INPUT,
	SETTING_SIDEWAYS_WIIMOTE,
	SETTING_UPRIGHT_WIIMOTE,
	SETTING_MOTIONPLUS,
	SETTING_IR_HIDE,
};
enum
{
	AS_LEFT,
	AS_RIGHT,
	AS_UP,
	AS_DOWN,
	AS_MODIFIER,
};
enum
{
	AS_RADIUS,
	AS_DEADZONE,
	AS_SQUARE,
};
enum
{
	B_NR_THRESHOLD,
};
enum
{
	B_RANGE,
	B_THRESHOLD,
};
enum
{
	F_FORWARD,
	F_BACKWARD,
	F_LEFT,
	F_RIGHT,
	F_UP,
	F_DOWN,
};
enum
{
	F_RANGE,
	F_DEADZONE,
};
enum
{
	T_FORWARD,
	T_BACKWARD,
	T_LEFT,
	T_RIGHT,
	T_UP,
	T_DOWN,
	T_MODIFIER,
};
enum
{
	T_ACC_RANGE,
	T_GYRO_RANGE,
	T_DEADZONE,
	T_CIRCLESTICK,
};
enum
{
	T_NG_RANGE,
	T_NG_DEADZONE,
	T_NG_CIRCLESTICK,
};
enum
{
	C_SENSITIVITY,
	C_CENTER,
	C_WIDTH,
	C_HEIGHT,
};

const char * const named_directions[] = 
{
	"Up",
	"Down",
	"Left",
	"Right"
};

class ControllerEmu
{
public:

	class ControlGroup
	{
	public:

		class Control
		{
		protected:
			Control(ControllerInterface::ControlReference* const _ref, const char * const _name)
				: control_ref(_ref), name(_name){}
		public:

			virtual ~Control();
			ControllerInterface::ControlReference*		const control_ref;
			const char * const		name;

		};

		class Input : public Control
		{
		public:

			Input(const char * const _name)
				: Control(new ControllerInterface::InputReference, _name) {}

		};

		class Output : public Control
		{
		public:

			Output(const char * const _name)
				: Control(new ControllerInterface::OutputReference, _name) {}

		};

		class Setting
		{
		public:

			Setting(const char* const _name, const ControlState def_value
				, const unsigned int _low = 0, const unsigned int _high = 100 )
				: name(_name)
				, value(def_value)
				, default_value(def_value)
				, low(_low)
				, high(_high){}

			const char* const	name;
			ControlState		value;
			const ControlState	default_value;
			const unsigned int	low, high;
		};

		ControlGroup(const char* const _name, const unsigned int _type = GROUP_TYPE_OTHER) : name(_name), type(_type) {}
		virtual ~ControlGroup();
	
		virtual void LoadConfig(IniFile::Section *sec, const std::string& defdev = "", const std::string& base = "" );
		virtual void SaveConfig(IniFile::Section *sec, const std::string& defdev = "", const std::string& base = "" );

		const char* const			name;
		const unsigned int			type;

		std::vector< Control* >		controls;
		std::vector< Setting* >		settings;

	};

	class AnalogStick : public ControlGroup
	{
	public:

		template <typename C>
		void GetState(C* const x, C* const y, const unsigned int base, const unsigned int range)
		{
			// this is all a mess

			ControlState yy = controls[0]->control_ref->State() - controls[1]->control_ref->State();
			ControlState xx = controls[3]->control_ref->State() - controls[2]->control_ref->State();

			ControlState radius = settings[AS_RADIUS]->value;
			ControlState deadzone = settings[AS_DEADZONE]->value;
			ControlState square = settings[AS_SQUARE]->value;
			ControlState m = controls[AS_MODIFIER]->control_ref->State();

			// modifier code
			if (m)
			{
				yy = (fabsf(yy)>deadzone) * sign(yy) * (m + deadzone/2);
				xx = (fabsf(xx)>deadzone) * sign(xx) * (m + deadzone/2);
			}

			// deadzone / square stick code
			if (radius != 1 || deadzone || square)
			{
				// this section might be all wrong, but its working good enough, i think

				ControlState ang = atan2(yy, xx);
				ControlState ang_sin = sin(ang);
				ControlState ang_cos = cos(ang);
				ControlState rad = sqrt(xx*xx + yy*yy);

				// the amt a full square stick would have at current angle
				ControlState square_full = std::min(ang_sin ? 1/fabsf(ang_sin) : 2, ang_cos ? 1/fabsf(ang_cos) : 2);

				// the amt a full stick would have that was ( user setting squareness) at current angle
				// i think this is more like a pointed circle rather than a rounded square like it should be
				ControlState stick_full = (1 + (square_full - 1) * square);

				// radius
				rad *= radius;

				// dead zone code
				rad = std::max(0.0f, rad - deadzone * stick_full);
				rad /= (1 - deadzone);

				// square stick code
				ControlState amt = rad / stick_full;
				rad -= ((square_full - 1) * amt * square);

				yy = std::max(-1.0f, std::min(1.0f, ang_sin * rad));
				xx = std::max(-1.0f, std::min(1.0f, ang_cos * rad));
			}

			*y = C(yy * range + base);
			*x = C(xx * range + base);
		}

		AnalogStick(const char* const _name);

	};

	class Buttons : public ControlGroup
	{
	public:
		Buttons(const char* const _name, bool range = false);

		template <typename C>
		void GetState(C* const buttons, const C* bitmasks)
		{
			std::vector<Control*>::iterator i = controls.begin(),
				e = controls.end();
			for (; i!=e; ++i, ++bitmasks)
				if ((*i)->control_ref->State() > settings[m_range?B_THRESHOLD:B_NR_THRESHOLD]->value) // threshold
					*buttons |= *bitmasks;
		}
	private:
		bool	m_range;
	};

	class MixedTriggers : public ControlGroup
	{
	public:

		template <typename C, typename S>
		void GetState(C* const digital, const C* bitmasks, S* analog, const unsigned int range)
		{
			const unsigned int trig_count = ((unsigned int) (controls.size() / 2));
			for (unsigned int i=0; i<trig_count; ++i,++bitmasks,++analog)
			{
				if (controls[i]->control_ref->State() > settings[0]->value) //threshold
				{
					*analog = range;
					*digital |= *bitmasks;
				}
				else
					*analog = S(controls[i+trig_count]->control_ref->State() * range);
					
			}
		}

		MixedTriggers(const char* const _name);

	};

	class Triggers : public ControlGroup
	{
	public:

		template <typename S>
		void GetState(S* analog, const unsigned int range)
		{
			const unsigned int trig_count = ((unsigned int) (controls.size()));
			const ControlState deadzone = settings[0]->value;
			for (unsigned int i=0; i<trig_count; ++i,++analog)
				*analog = S(std::max(controls[i]->control_ref->State() - deadzone, 0.0f) / (1 - deadzone) * range);
		}

		Triggers(const char* const _name);

	};

	class Slider : public ControlGroup
	{
	public:

		template <typename S>
		void GetState(S* const slider, const unsigned int range, const unsigned int base = 0)
		{
			const float deadzone = settings[0]->value;
			const float state = controls[1]->control_ref->State() - controls[0]->control_ref->State();

			if (fabsf(state) > deadzone)
				*slider = (S)((state - (deadzone * sign(state))) / (1 - deadzone) * range + base);
			else
				*slider = 0;
		}

		Slider(const char* const _name);

	};

	class Force : public ControlGroup
	{
	public:
		Force(const char* const _name);

		template <typename C, typename R>
		void GetState(C* axis, const u8 base, const R range, bool step = true)
		{
			const float master_range = settings[F_RANGE]->value;
			const float deadzone = settings[F_DEADZONE]->value;
			for (unsigned int i=0; i<6; i+=2)
			{
				float dz = 0;
				ControlState state = controls[i]->control_ref->State() - controls[i+1]->control_ref->State();
				if (fabsf(state) > deadzone)
					dz = ((state - (deadzone * sign(state))) / (1 - deadzone));
				step = true;
				if (step) {
					if (state > m_thrust[i])
						m_thrust[i] = std::min(m_thrust[i] + 0.1f, state);
					else if (state < m_thrust[i])
						m_thrust[i] = std::max(m_thrust[i] - 0.1f, state);
				}
				
				*axis++ = (C)((abs(m_thrust[i]) >= 0.7 ? -2*sign(state)+m_thrust[i]*2 : m_thrust[i]) * sign(state) * range * master_range + base);
				//if(i==0) SWARN_LOG(CONSOLE, "%d | %0.2f %0.2f | %0.2f %0.2f | state %0.2f, sign %0.2f, dz %0.2f", i, master_range, range, *(axis-1), m_thrust[i], state, (float)sign(state), dz);
			}
		}
	private:
		float	m_thrust[3];
	};

	class Tilt : public ControlGroup
	{
	public:
		Tilt(const char* const _name, bool gyro = false);

		template <typename C, typename R>
		void GetState(C* const x, C* const y, C* const z, const unsigned int base, const R range, const bool step = true)
		{
			// this is all a mess
			ControlState xx = controls[T_RIGHT]->control_ref->State() - controls[T_LEFT]->control_ref->State();
			ControlState yy = controls[T_FORWARD]->control_ref->State() - controls[T_BACKWARD]->control_ref->State();			
			ControlState zz = controls[T_DOWN]->control_ref->State() - controls[T_UP]->control_ref->State();

			ControlState deadzone = settings[m_gyro?T_DEADZONE:T_NG_DEADZONE]->value;
			ControlState circle = settings[m_gyro?T_CIRCLESTICK:T_NG_CIRCLESTICK]->value;
			ControlState m = controls[T_MODIFIER]->control_ref->State();

			// modifier code
			if (m)
			{
				xx = (fabsf(xx)>deadzone) * sign(xx) * (m + deadzone/2);
				yy = (fabsf(yy)>deadzone) * sign(yy) * (m + deadzone/2);				
				zz = (fabsf(zz)>deadzone) * sign(zz) * (m + deadzone/2);
			}

			// deadzone / circle stick code
			if (deadzone || circle)
			{
				// this section might be all wrong, but its working good enough, i think

				ControlState ang = atan2(yy, xx); 
				ControlState ang_sin = sin(ang);
				ControlState ang_cos = cos(ang);

				// the amt a full square stick would have at current angle
				ControlState square_full = std::min(ang_sin ? 1/fabsf(ang_sin) : 2, ang_cos ? 1/fabsf(ang_cos) : 2);

				// the amt a full stick would have that was (user setting circular) at current angle
				// i think this is more like a pointed circle rather than a rounded square like it should be
				ControlState stick_full = (square_full * (1 - circle)) + (circle);

				ControlState dist = sqrt(xx*xx + yy*yy);

				// dead zone code
				dist = std::max(0.0f, dist - deadzone * stick_full);
				dist /= (1 - deadzone);

				// circle stick code
				ControlState amt = dist / stick_full;
				dist += (square_full - 1) * amt * circle;

				yy = std::max(-1.0f, std::min(1.0f, ang_sin * dist));
				xx = std::max(-1.0f, std::min(1.0f, ang_cos * dist));
			}

			// this is kinda silly here
			// gui being open will make this happen 2x as fast, o well
			
			// silly
			if (step)
			{
				if (xx > m_tilt[0])
					m_tilt[0] = std::min(m_tilt[0] + 0.1f, xx);
				else if (xx < m_tilt[0])
					m_tilt[0] = std::max(m_tilt[0] - 0.1f, xx);

				if (yy > m_tilt[1])
					m_tilt[1] = std::min(m_tilt[1] + 0.1f, yy);
				else if (yy < m_tilt[1])
					m_tilt[1] = std::max(m_tilt[1] - 0.1f, yy);

				if (zz > m_tilt[2])
					m_tilt[2] = std::min(m_tilt[2] + 0.1f, zz);
				else if (zz < m_tilt[2])
					m_tilt[2] = std::max(m_tilt[2] - 0.1f, zz);
			}

			*x = C(m_tilt[0] * range + base);
			*y = C(m_tilt[1] * range + base);			
			*z = C(m_tilt[2] * range + base);
		}
	private:
		float	m_tilt[3];
		bool	m_gyro;
	};

	class Cursor : public ControlGroup
	{
	public:
		Cursor(const char* const _name);

		template <typename C>
		void GetState(C* const x, C* const y, C* const z, const bool adjusted = false, const bool relative = false)
		{
			if (relative) {
				//std::string state = ""; for(int i=0; i<controls.size(); i++)
				//	state += StringFromFormat("%0.2f ", controls[i]->control_ref->State(0, true));				
				//SWARN_LOG(CONSOLE, "Cursor::GetState: size %d, state %s", controls.size(), state.c_str());				
				float yy = controls[0]->control_ref->State(0, true);
				float xx = controls[2]->control_ref->State(0, true);
				float zz = controls[4]->control_ref->State(0, true);
				// settings
				if (adjusted)
				{
					yy *= settings[C_SENSITIVITY]->value;
					xx *= settings[C_SENSITIVITY]->value;
					zz *= settings[C_SENSITIVITY]->value;
				}
				*y = yy;
				*x = xx;
				*z = zz;
				return;
			}

			const float zz = controls[4]->control_ref->State(0, settings[C_SENSITIVITY]->value) - controls[5]->control_ref->State(0, settings[C_SENSITIVITY]->value);

			// silly being here
			if (zz > m_z)
				m_z = std::min(m_z + 0.1f, zz);
			else if (zz < m_z)
				m_z = std::max(m_z - 0.1f, zz);

			*z = m_z;
			
			// hide
			if (controls[6]->control_ref->State() > 0.5f)
			{
				*x = 10000; *y = 0;
			}
			else
			{
				float yy = controls[0]->control_ref->State() - controls[1]->control_ref->State();
				float xx = controls[3]->control_ref->State() - controls[2]->control_ref->State();				

				// adjust cursor according to settings
				if (adjusted)
				{
					xx *= (settings[C_WIDTH]->value * 2);
					yy *= (settings[C_HEIGHT]->value * 2);
					yy += (settings[C_CENTER]->value - 0.5f);
				}

				*x = xx;
				*y = yy;
			}
		}

		float	m_z;
	};

	class Extension : public ControlGroup
	{
	public:
		Extension(const char* const _name)
			: ControlGroup(_name, GROUP_TYPE_EXTENSION)
			, switch_extension(0)
			, active_extension(0) {}
		~Extension();

		void GetState(u8* const data, const bool focus = true);

		std::vector<ControllerEmu*>		attachments;

		int	switch_extension;
		int	active_extension;
	};

	virtual ~ControllerEmu();

	virtual std::string GetName() const = 0;

	virtual void LoadDefaults(const ControllerInterface& ciface);

	virtual void LoadConfig(IniFile::Section *sec, const std::string& base = "");
	virtual void SaveConfig(IniFile::Section *sec, const std::string& base = "");
	void UpdateDefaultDevice();

	void UpdateReferences(ControllerInterface& devi);

	std::vector< ControlGroup* >		groups;

	ControllerInterface::DeviceQualifier	default_device;

};


#endif