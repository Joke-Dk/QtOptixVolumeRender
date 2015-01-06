
/*
* Copyright (c) 2008 - 2009 NVIDIA Corporation.  All rights reserved.
*
* NVIDIA Corporation and its licensors retain all intellectual property and proprietary
* rights in and to this software, related documentation and any modifications thereto.
* Any use, reproduction, disclosure or distribution of this software and related
* documentation without an express license agreement from NVIDIA Corporation is strictly
* prohibited.
*
* TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
* SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
* LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
* BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
* INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGES
*/

#pragma once
//#define NOMINMAX
//
#include <QtGui>
#include <QGLWidget>

#include <optixu/optixpp_namespace.h>

#include <string>
#include <sutil.h>
#include <SampleScene.h>

class Mouse;
class PinholeCamera;


//-----------------------------------------------------------------------------
// 
// QTGLUTDisplay 
//
//-----------------------------------------------------------------------------

class QTGLUTDisplay : public QGLWidget
{
	Q_OBJECT
		friend class Widget;
public:
	QTGLUTDisplay(int argc, char **argv, QWidget *parent = 0);
	~QTGLUTDisplay();

	QSize minimumSizeHint() const;
	QSize sizeHint() const;
	//SampleScene* scene;
protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
public:
	enum contDraw_E { CDNone=0, CDProgressive=1, CDAnimated=2, CDBenchmark=3, CDBenchmarkTimed=4 };

	static void init( int& argc, char** argv );
	static void run( const std::string& title, SampleScene* scene, contDraw_E continuous_mode = CDNone );
	static void printUsage();

	static void setTextColor( const optix::float3& c )
	{ _text_color = c; }

	static void setTextShadowColor( const optix::float3& c )
	{ _text_shadow_color = c; }

	static void setProgressiveDrawingTimeout( double t )
	{ _progressive_timeout = t; }

	static contDraw_E getContinuousMode() { return _app_continuous_mode; }
	static void setContinuousMode(contDraw_E continuous_mode);
	static void setCamera(SampleScene::InitialCameraData& camera_data);

	static bool isBenchmark() { return _cur_continuous_mode == CDBenchmark || _cur_continuous_mode == CDBenchmarkTimed ||
		_app_continuous_mode == CDBenchmark || _app_continuous_mode == CDBenchmarkTimed; }

	// Make sure you only call this from the callback functions found in SampleScene:
	// initScene, trace, getOutputBuffer, cleanUp, resize, doResize, and keyPressed.
	static void postRedisplay();

	static void setUseSRGB(bool enabled) { _use_sRGB = enabled; }

	static SampleScene*   _scene;

private:

	// Draw text to screen at window pos x,y.  To make this public we will need to have
	// a public helper that caches the text for use in the display func
	static void drawText( const std::string& text, float x, float y, void* font );

	// Do the actual rendering to the display
	static void displayFrame();

	// Executed if _benchmark_no_display is true:
	//   - Renders '_warmup_frames' frames without timing
	//   - Renders '_timed_frames' frames for benchmarking
	//   OR
	//   - Renders '_warmup_time' seconds without timing
	//   - Renders '_timed_time' seconds for benchmarking
	//   - Prints results to screen using sutilPrintBenchmark.
	//   - If _save_frames_to_file is set, prints one copy of frame to file:
	//     ${_title}.ppm. _title is set to _save_frames_basename if non-empty.
	static void runBenchmarkNoDisplay();

	// Set the current continuous drawing mode, while preserving the app's choice.
	static void setCurContinuousMode(contDraw_E continuous_mode);

	// Cleans up the rendering context and quits.  If there wasn't error cleaning up, the 
	// return code is passed out, otherwise 2 is used as the return code.
	static void quit(int return_code=0);

	// Glut callbacks
	static void idle();
	static void display();
	static void keyPressed(unsigned char key, int x, int y);
	static void mouseButton(int button, int state, int x, int y);
	static void mouseMotion(int x, int y);
	static void resize(int width, int height);

	static Mouse*         _mouse;
	static PinholeCamera* _camera;


	static double         _last_frame_time;
	static unsigned int   _last_frame_count;
	static unsigned int   _frame_count;

	static bool           _display_fps;
	static double         _fps_update_threshold;
	static char           _fps_text[32];
	static optix::float3  _text_color;
	static optix::float3  _text_shadow_color;

	static bool           _print_mem_usage;

	static contDraw_E     _app_continuous_mode;
	static contDraw_E     _cur_continuous_mode;
	static bool           _display_frames;
	static bool           _save_frames_to_file;
	static std::string    _save_frames_basename;

	static std::string    _camera_pose;

	static int            _initial_window_width;
	static int            _initial_window_height;

	static int            _old_window_height;
	static int            _old_window_width;
	static int            _old_window_x;
	static int            _old_window_y;

	static unsigned int   _texId;
	static bool           _sRGB_supported;
	static bool           _use_sRGB;
	static bool           _initialized;

	static bool           _benchmark_no_display;
	static unsigned int   _warmup_frames;
	static unsigned int   _timed_frames;
	static double         _warmup_start; // time when the warmup started
	static double         _warmup_time; // used instead of _warmup_frames to specify a time to run instead of number of frames to run
	static double         _benchmark_time; // like _warmup_time, but for the time to benchmark
	static unsigned int   _benchmark_frame_start;
	static double         _benchmark_frame_time;
	static std::string    _title;

	static double         _progressive_timeout; // how long to do continuous rendering for progressive refinement (ignored when benchmarking or animating)
	static double         _start_time; // time since continuous rendering last started

	static int            _num_devices;
};