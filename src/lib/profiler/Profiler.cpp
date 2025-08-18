/*
 * Profiler.cpp
 *
 *  Created on: 07.01.2020
 *      Author: michi
 */

#include "Profiler.h"
#include "../base/iter.h"

namespace profiler {
	int frames = -1;
	bool just_cleared = true;
	std::chrono::high_resolution_clock::time_point frame_start;
	float temp_frame_time = 0;
	float avg_frame_time = 0;
	float frame_dt = 0;



	Array<Channel> channels;

	FrameTimingData current_frame_timing;
	FrameTimingData previous_frame_timing;
	FrameHistory history;
	int max_history_length = 1024;

	void _reset() {
		temp_frame_time = 0;
		frames = 0;
	}

	int create_channel(const string &name, int parent) {
		for (auto&& [i, c]: enumerate(channels))
			if (!c.used) {
				c.used = true;
				c.name = name;
				c.parent = parent;
				return i;
			}
		channels.add({name, true, parent});
		return channels.num - 1;
	}

	void delete_channel(int channel) {
		channels[channel].used = false;
	}

	void set_parent(int channel, int parent) {
		channels[channel].parent = parent;
	}

	void set_name(int channel, const string& name) {
		channels[channel].name = name;
	}

	string get_name(int channel) {
		return channels[channel].name;
	}

	void begin(int channel) {
		auto now = std::chrono::high_resolution_clock::now();
		current_frame_timing.cpu0.add({channel, std::chrono::duration<float, std::chrono::seconds::period>(now - frame_start).count()});
	}

	void end(int channel) {
		auto now = std::chrono::high_resolution_clock::now();
		current_frame_timing.cpu0.add({channel | (int)0x80000000, std::chrono::duration<float, std::chrono::seconds::period>(now - frame_start).count()});
	}

	/*void begin_gpu(int channel, float t) {
		current_frame_timing.gpu.add({channel | (int)0x80000000, t});
	}
	void end_gpu(int channel, float t) {
		current_frame_timing.gpu.add({channel, t});
	}*/

	void next_frame() {
		auto now = std::chrono::high_resolution_clock::now();

		current_frame_timing.total_time = std::chrono::duration<float, std::chrono::seconds::period>(now - frame_start).count();

		if (!just_cleared) {
			frame_dt = std::chrono::duration<float, std::chrono::seconds::period>(now - frame_start).count();
			temp_frame_time += frame_dt;
		}
		frame_start = now;
		frames ++;
		just_cleared = false;

		if (temp_frame_time > 0.2f) {
			avg_frame_time = temp_frame_time / (float)frames;
			_reset();
		}

		previous_frame_timing = current_frame_timing;
		current_frame_timing.cpu0.clear();
		current_frame_timing.cpu0.simple_reserve(256);
		current_frame_timing.gpu.clear();
		current_frame_timing.gpu.simple_reserve(256);

		history.total_times.add(frame_dt);
		while (history.total_times.num > max_history_length)
			history.total_times.erase(0);
	}

	Array<ChannelStats> digest_report(const FrameTimingData& td) {
		Array<ChannelStats> r;
		Array<float> prev_begin;
		prev_begin.resize(channels.num);
		Array<int> channel_map;
		channel_map.resize(channels.num);

		for (const auto&& [i, c]: enumerate(channels))
			if (c.used) {
				channel_map[i] = r.num;
				r.add({i});
			}

		for (const auto& t: td.cpu0) {
			int id = t.channel & 0x7fffffff;
			if (t.channel & 0x80000000) {
				// end
				r[channel_map[id]].total += (t.offset - prev_begin[id]);
				r[channel_map[id]].count ++;
			} else {
				// begin
				prev_begin[id] = t.offset;
			}
		}

		for (auto& s: r)
			if (s.count > 0)
				s.average = s.total / (float)s.count;
		return r;
	}
}
