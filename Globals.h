#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>




#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

#include <iostream>
#include <Windows.h>
#include <algorithm>

#include <cmath>


#pragma once

#include <string>
#include <Windows.h>

std::string keyToString(int virtualKeyCode);
extern bool colorbotfonction;





extern float maxDisplacementX;  // Adjust maximum displacement as needed
extern float maxDisplacementY;
extern float displacementX;  // Adjust as needed
extern float displacementY;  // Adjust as needed

			


namespace globals {
	// yall know its scanning size..
	inline float ScanningSize = 200;
	//our smoothing
	inline float Smoothing = 0.5;
	inline float colorTolerance = 30;
	inline HWND hWnd = nullptr;

    inline int targetCR = 255.0f;
    inline int targetCG = 52.0f;
    inline int targetCB = 52.0f;

	//inline int targetCR = 255.0f;
	//inline int targetCG = 110.0f;
	//inline int targetCB = 199.0f;

	inline static float col21[3];


	inline int IngameSensitivity = 1.5;


	inline float speedofF = 0.1;

	inline int triggerKey = VK_LMENU; // Use VK_LMENU for left Alt key

}



