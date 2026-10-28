//
// Created by michi on 9/28/26.
//

#pragma once

#if __has_include("../os/path.h")
#include "Observable.h"
#include "../os/path.h"

namespace obs {

	class FileWatcher : public Node<VirtualBase> {
	public:
		FileWatcher();
		~FileWatcher() override;
		void add(const Path& path);
		void iterate();

		struct MoveInfo {
			Path from, to;
		};

		xsource<Path> out_edited{this, "edited"};
		xsource<Path> out_deleted{this, "deleted"};
		xsource<Path> out_created{this, "created"};
		xsource<MoveInfo> out_moved{this, "moved"};

		int epoll_fd;
		int inotify_fd;
		struct Item {
			int w;
			Path path;

			string last_move_from;
		};
		Array<Item> items;
	};
} // obs

#endif
