#include "Globals.h"
#include "gui.h"
#include "mouse.h"
#include <chrono>
#include <windows.h>
#include "styles.h" // Include the styles header file


#include "opencv2/opencv.hpp"





using namespace globals;
using namespace cv;
using namespace std;
////////////////////////////////////////////////////////////////

// Move the mouse using InterceptionMouse

////////////////////////////////////////////////////////////////

bool isKeyPressed() {
    return GetAsyncKeyState(triggerKey) & 0x8000;
}


#include <thread>
#include <vector>

// bitMap ---> mat
void GetBitmapToMat(HBITMAP hBitmap, Mat& mat) {
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    int width = bmp.bmWidth;
    int height = bmp.bmHeight;

    BITMAPINFO bi;
    memset(&bi, 0, sizeof(BITMAPINFO));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = width;
    bi.bmiHeader.biHeight = -height;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    mat.create(height, width, CV_8UC4);
    GetDIBits(GetDC(0), hBitmap, 0, height, mat.data, &bi, DIB_RGB_COLORS);
}
//getting da center X and Y,
POINT GetScreenCenter() {
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    POINT center;
    center.x = screenWidth / 2;
    center.y = screenHeight / 2;

    return center;
}

float displacementX = 20.0f;  // Adjust as needed
float displacementY = 10.0f;  // Adjust as needed

float maxDisplacementX = 0.0f;  // Adjust maximum displacement as needed
float maxDisplacementY = -20.0f;  // Adjust maximum displacement as needed


void ColorAimbotMain() {

    // Set up screen capture
    HDC hDC = GetDC(globals::hWnd);
    HDC hCaptureDC = CreateCompatibleDC(hDC);
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDC, screenWidth, screenHeight);

    SelectObject(hCaptureDC, hCaptureBitmap);

        // Capture the screen
        if (!isKeyPressed()) {
            // Clean up stuff
            DeleteObject(hCaptureBitmap);
            DeleteDC(hCaptureDC);
            ReleaseDC(globals::hWnd, hDC);
            return; // Exit the function
        }
        if (GetAsyncKeyState(triggerKey) & 0x8000) {
            Scalar lowerColorBound(
                max(globals::targetCB - globals::colorTolerance, 0.0),
                max(globals::targetCG - globals::colorTolerance, 0.0),
                max(globals::targetCR - globals::colorTolerance, 0.0)
            );
            Scalar upperColorBound(
                min(globals::targetCB + globals::colorTolerance, 255.0),
                min(globals::targetCG + globals::colorTolerance, 255.0),
                min(globals::targetCR + globals::colorTolerance, 255.0)
            );
            BitBlt(hCaptureDC, 0, 0, screenWidth, screenHeight, hDC, 0, 0, SRCCOPY | CAPTUREBLT);
            Mat screen;
            //converting the bitmap image to the and we give it to Mat Screen we have.
            GetBitmapToMat(hCaptureBitmap, screen);
            cvtColor(screen, screen, COLOR_BGRA2BGR);


            // Get the scanning area to center and Calculate the size.
            POINT center = GetScreenCenter();

            Rect centerRect(center.x - globals::ScanningSize / 2, center.y - globals::ScanningSize / 2, globals::ScanningSize, globals::ScanningSize);
            Mat croppedScreen = screen(centerRect);

            cv::Mat downscaled;
            double scale_factor = 0.1;
            cv::resize(screen, downscaled, cv::Size(), scale_factor, scale_factor);


            // Threshold or tolerance for that specific color we gave it.
            Mat colorMask;
            inRange(croppedScreen, lowerColorBound, upperColorBound, colorMask);

            // Find the centroid of the color in the scannned image
            Moments m = moments(colorMask, true);
            float x = (float)(m.m10 / m.m00);
            float y = (float)(m.m01 / m.m00);

            // Move the mouse to the color centroid if found, Note that if you dont do this if statement your aimbot still works but when the color isnt found it'll go crazy, so here we are making sure the color is found
            if (m.m00 > 0) {
                //some Other calculation for the cordinates of the Color and CenterX, so we move Center X to the Color not the actual cursor. 
                float aimX = (center.x - globals::ScanningSize / 2 + x) - center.x;
                float aimY = (center.y - globals::ScanningSize / 2 + y) - center.y;

                // Calculate relative position within the scanning rectangle
                float relativeX = static_cast<float>(x) / static_cast<float>(centerRect.width);
                float relativeY = static_cast<float>(y) / static_cast<float>(centerRect.height);

                // Calculate displacement based on enemy size

                float displacementX = relativeX * maxDisplacementX;
                float displacementY = relativeY * maxDisplacementY;

                // Apply displacement
                aimX += displacementX;
                aimY += displacementY;


               aimX /= globals::Smoothing;
               aimY /= globals::Smoothing;

 

                // Using mouse event to move mouse.
                mouse_event(MOUSEEVENTF_MOVE, aimX, aimY, 0, 0);

            }

            // D
        }

    // Clean up stff
    DeleteObject(hCaptureBitmap);
    DeleteDC(hCaptureDC);
    ReleaseDC(globals::hWnd, hDC);
}
bool isLoading = true;

void LoadingPage()
{
    ImGui::SetNextWindowSize(ImVec2(300, 100));
    ImGui::SetNextWindowPos(ImVec2((gui::WIDTH - 300) * 0.5f, (gui::HEIGHT - 100) * 0.5f));

    ImGui::Begin("Loading", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);


    ImGui::Text("Loading, please wait...");



    ImGui::End();
}

bool colorbotfonction = false;


int __stdcall wWinMain(
    HINSTANCE instance,
    HINSTANCE previousInstance,
    PWSTR arguments,
    int commandShow)
{
    // create gui
    gui::CreateHWindow("Cheat Menu");
    gui::CreateDevice();
    gui::CreateImGui();

    gui::BeginRender();
    LoadingPage();
    gui::EndRender();
    // simulate loading for 3 seconds
    std::this_thread::sleep_for(std::chrono::seconds(3));
    ApplyCustomStyles();



    while (gui::isRunning)
    {


        if (colorbotfonction) {
            ColorAimbotMain();
           

        }



            gui::BeginRender();
            gui::Render();
            gui::EndRender();



            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        
       
    }

    // destroy gui
    gui::DestroyImGui();
    gui::DestroyDevice();
    gui::DestroyHWindow();

    return EXIT_SUCCESS;
}