#include "../base/base.h"
#include "Profiler.h"
#include "../kabaexport/KabaExporter.h"


void export_package_profiler(kaba::Exporter* e) {
	e->declare_class_size("Profiler.Channel", sizeof(profiler::Channel));
	e->declare_class_element("Profiler.Channel.name", &profiler::Channel::name);
	e->declare_class_element("Profiler.Channel.parent", &profiler::Channel::parent);

	e->declare_class_size("Profiler.TimingData", sizeof(profiler::TimingData));
	e->declare_class_element("Profiler.TimingData.channel", &profiler::TimingData::channel);
	e->declare_class_element("Profiler.TimingData.offset", &profiler::TimingData::offset);

	e->declare_class_size("Profiler.FrameTimingData", sizeof(profiler::FrameTimingData));
	e->declare_class_element("Profiler.FrameTimingData.cpu0", &profiler::FrameTimingData::cpu0);
	e->declare_class_element("Profiler.FrameTimingData.gpu", &profiler::FrameTimingData::gpu);
	e->declare_class_element("Profiler.FrameTimingData.total_time", &profiler::FrameTimingData::total_time);

	e->declare_class_size("Profiler.FrameHistory", sizeof(profiler::FrameHistory));
	e->declare_class_element("Profiler.FrameHistory.total_times", &profiler::FrameHistory::total_times);

	e->declare_class_size("Profiler", sizeof(profiler::Profiler));
	e->link_func("Profiler.get_name", &profiler::get_name);
	e->link("Profiler.avg_frame_time", &profiler::avg_frame_time);
	e->link("Profiler.frames", &profiler::frames);
	e->link("Profiler.channels", &profiler::channels);
	e->link("Profiler.previous_frame_timing", &profiler::previous_frame_timing);
	e->link("Profiler.history", &profiler::history);
}


