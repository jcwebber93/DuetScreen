/*
 * DisplayHelper.h
 *
 *  Created on: 2025-02-26
 *      Author: Andy Everitt
 */

#pragma once

#include <cstdint>

enum class DisplayRotation
{
	ROTATION_0 = 0,
	ROTATION_90,
	ROTATION_180,
	ROTATION_270,
};

namespace Display
{
	// Native panel resolution, in the panel's own (landscape) coordinate space. This is the size of the framebuffer
	// and is unaffected by the rotation.
	constexpr int32_t PANEL_WIDTH = 1024;
	constexpr int32_t PANEL_HEIGHT = 600;

	// Whether the rotation swaps the width and height of the visible canvas.
	constexpr bool isPortrait(DisplayRotation rotation)
	{
		return rotation == DisplayRotation::ROTATION_90 || rotation == DisplayRotation::ROTATION_270;
	}

	// The resolution the UI is laid out against, which has the axes swapped in the portrait rotations.
	constexpr int32_t canvasWidth(DisplayRotation rotation)
	{
		return isPortrait(rotation) ? PANEL_HEIGHT : PANEL_WIDTH;
	}

	constexpr int32_t canvasHeight(DisplayRotation rotation)
	{
		return isPortrait(rotation) ? PANEL_WIDTH : PANEL_HEIGHT;
	}

	// Whether the canvas the UI is currently laid out against is taller than it is wide. Views use this to pick a
	// layout. It is derived from the display rather than the stored setting so that it is correct on both backends:
	// on hardware the rotation is applied to the display, in simulation the window is created at the canvas size.
	// Only valid once the display has been created in hal_init().
	bool isPortrait();
} // namespace Display

class DisplayHelper
{
  public:
	// Sets the display brightness.
	// brightness should be in the range [0, 100]
	static bool setBrightness(unsigned int percentage);
	static bool setScreenSaverBrightness(unsigned int percentage);

	static void enableScreenSaver(bool enable);

	// Gets the current display brightness.
	// Returns a brightness value in the range [0, 100].
	static unsigned int getBrightness();

	// Rotates the display to match a rotation that has already been decided on.
	static void applyRotation(DisplayRotation rotation);

	// Persists the rotation, applying it immediately if that is possible without re-laying out the UI. Rotating
	// between landscape and portrait needs a restart, which is the caller's responsibility.
	static void setRotation(DisplayRotation rotation);

	// Gets the currently stored display rotation.
	static DisplayRotation getRotation();

  private:
	DisplayHelper(const char* device = "/dev/disp", unsigned int screen = 0);
	~DisplayHelper();

	static DisplayHelper& instance();

	bool setBrightnessInner(unsigned int percentage);

	int m_fd = -1;		   // File descriptor for the display device
	unsigned int m_screen; // Screen number (typically 0 or 1)
	unsigned int m_percentage = 100;
	unsigned int m_screensaverPercentage = 0;
	unsigned int m_currentBrightness = 0;
};
