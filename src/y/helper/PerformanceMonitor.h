/*
 * PerformanceMonitor.h
 *
 *  Created on: 07.01.2020
 *      Author: michi
 */

#pragma once

#include <chrono>
#include "../lib/base/base.h"

/*enum class PMLabel {
	UNKNOWN,
	PRE,
	OUT,
	END,
	PREPARE_LIGHTS,
	SHADOWS,
	WORLD,
	PARTICLES,
	SKYBOXES,
	GUI,
	ITERATE,
	ANIMATION,
};*/


struct PerformanceChannel {
	string name;
	int parent = -1;
	std::chrono::high_resolution_clock::time_point prev;
	float dt = 0, average = 0;
	int count = 0;
};

struct TimingData {
	int channel;
	float offset;
};

struct FrameTimingData {
	Array<TimingData> cpu0;
	Array<TimingData> gpu;
	float total_time;
};

class PerformanceMonitor {
public:
	//PerformanceMonitor();

	static int create_channel(const string &name, int parent = -1);
	static string get_name(int channel);

	static void begin(int channel);
	static void end(int channel);

	static void begin_gpu(int channel, float t);
	static void end_gpu(int channel, float t);

	static void next_frame();
	static void _reset();


	static int frames;
	static bool just_cleared;
	static std::chrono::high_resolution_clock::time_point frame_start;

	static float temp_frame_time;
	static float avg_frame_time;

	static Array<PerformanceChannel> channels;
	static FrameTimingData current_frame_timing;
	static FrameTimingData previous_frame_timing;

	static float frame_dt;
};

