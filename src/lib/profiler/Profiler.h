/*
 * Profiler.h
 *
 *  Created on: 07.01.2020
 *      Author: michi
 */

#pragma once

#include <chrono>
#include "../base/base.h"

namespace profiler {
	struct Channel {
		string name;
		bool used = false;
		int parent = -1;
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

	struct FrameHistory {
		Array<float> total_times;
	};

	struct ChannelStats {
		int channel = -1;
		float total = 0, average = 0;
		int count = 0;
	};
	Array<ChannelStats> digest_report(const FrameTimingData& td);


	int create_channel(const string &name, int parent = -1);
	void delete_channel(int channel);
	void set_parent(int channel, int parent);
	void set_name(int channel, const string &name);
	string get_name(int channel);

	void begin(int channel);
	void end(int channel);

	void next_frame();
	void _reset();


	int frames();
	bool just_cleared();

	float temp_frame_time();
	float avg_frame_time();

	Array<Channel>& channels();
	FrameTimingData& current_frame_timing();
	FrameTimingData& previous_frame_timing();
	FrameHistory& history();

	float frame_dt();

	struct State;
	extern State* state;
	void init();
	void init_external(void* state);
}

