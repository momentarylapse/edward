#include "_kaba_export.h"
#include "../base/base.h"
#include "../kapi/KabaExporter.h"
#include "Profiler.h"


void _export_package_profiler_internal(kaba::IExporter* e) {
	e->declare_class_size("profiler.Channel", sizeof(profiler::Channel));
	e->declare_class_element("profiler.Channel.name", &profiler::Channel::name);
	e->declare_class_element("profiler.Channel.parent", &profiler::Channel::parent);

	e->declare_class_size("profiler.TimingData", sizeof(profiler::TimingData));
	e->declare_class_element("profiler.TimingData.channel", &profiler::TimingData::channel);
	e->declare_class_element("profiler.TimingData.offset", &profiler::TimingData::offset);

	e->declare_class_size("profiler.FrameTimingData", sizeof(profiler::FrameTimingData));
	e->declare_class_element("profiler.FrameTimingData.cpu0", &profiler::FrameTimingData::cpu0);
	e->declare_class_element("profiler.FrameTimingData.gpu", &profiler::FrameTimingData::gpu);
	e->declare_class_element("profiler.FrameTimingData.total_time", &profiler::FrameTimingData::total_time);

	e->declare_class_size("profiler.FrameHistory", sizeof(profiler::FrameHistory));
	e->declare_class_element("profiler.FrameHistory.total_times", &profiler::FrameHistory::total_times);

	e->declare_class_size("profiler.ChannelStats", sizeof(profiler::ChannelStats));
	e->declare_class_element("profiler.ChannelStats.channel", &profiler::ChannelStats::channel);
	e->declare_class_element("profiler.ChannelStats.total", &profiler::ChannelStats::total);
	e->declare_class_element("profiler.ChannelStats.average", &profiler::ChannelStats::average);
	e->declare_class_element("profiler.ChannelStats.count", &profiler::ChannelStats::count);

	//e->declare_class_size("profiler", sizeof(profiler::Profiler));
	e->link_func("profiler.create_channel", &profiler::create_channel);
	e->link_func("profiler.delete_channel", &profiler::delete_channel);
	e->link_func("profiler.set_name", &profiler::set_name);
	e->link_func("profiler.set_parent", &profiler::set_parent);
	e->link_func("profiler.get_name", &profiler::get_name);
	e->link_func("profiler.begin", &profiler::begin);
	e->link_func("profiler.end", &profiler::end);
	e->link_func("profiler.next_frame", &profiler::next_frame);
	e->link_func("profiler.digest_report", &profiler::digest_report);
	e->link("profiler.avg_frame_time", &profiler::avg_frame_time);
	e->link("profiler.frames", &profiler::frames);
	e->link("profiler.channels", &profiler::channels);
	e->link("profiler.previous_frame_timing", &profiler::previous_frame_timing);
	e->link("profiler.history", &profiler::history);
}

void export_package_profiler(kaba::IExporter* ext) {
	ext->package_info("profiler", "0.2");
	_export_package_profiler_internal(ext);
}


