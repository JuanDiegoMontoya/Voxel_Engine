#pragma once

#include "stdafx.h"
//#include "Serialization.h
//#include "GameObject.h"
//#include "SoundEvent.h"

// TODO: refactor this
namespace Settings
{
	struct CTL
	{
		int keyUp = GLFW_KEY_W;
		int keyUpAlt = GLFW_KEY_SPACE;
		int keyLeft = GLFW_KEY_A;
		int keyRight = GLFW_KEY_D;
		int keyDown = GLFW_KEY_S;
		int keySprint = GLFW_KEY_LEFT_SHIFT;
	};

	struct GFX
	{
		bool vsync = true;
		int multisamples = 0; // 0 = no msaa
		bool reinit = false; // enabled when reinitializing context

		bool fullscreen = false;

		// actual screen X and Y, use a temp to store settings before they're set
		int screenX = 1920;
		int screenY = 1080;

		// add supported resolutions to this list
		const std::vector<std::pair<int, int>> resolutions =
		{
			{ 3840, 2160 },
			{ 2560, 1440 },
			{ 1920, 1080 },
			{ 1600, 900 },
			{ 1440, 900 },
			{ 1366, 768 }
		};
		const size_t res_amt = resolutions.size();

		static inline bool blockAO = true;
	};

	struct SND
	{
		// from 0 to 10
		float master = 1;		// global multiplier
		float music = 1;		// music
		float sfx = 1;			// various sound effects
		float ambient = 1;  // environment, background, etc.
	};

	struct EDT
	{
		bool BSP = false;					// efficient collision algorithm
		bool radiusCheck = true;	// collision broadphase check
		bool priorityCheck = true;// collision broadphase check
		bool throwMode = false;		// throw held objects
		bool danglyBits = true;		// scaling squares on the edges of the selected object
	};

	// world settings
	struct WLD
	{

	};

	static typename GFX Graphics;
	static typename CTL Controls;
	static typename SND Sound;
	static typename EDT Editor;
	static typename WLD World;

//bool DrawOptions();
//void reinitializeDatMfGlfwContextBruh();
}