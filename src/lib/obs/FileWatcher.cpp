//
// Created by michi on 9/28/26.
//

#include "FileWatcher.h"

#if __has_include("../os/path.h")

#if defined(OS_LINUX)
#include <cstdio>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <linux/limits.h>
#endif

namespace obs {


FileWatcher::FileWatcher() {
#if defined(OS_LINUX)
	epoll_fd = epoll_create(1);
	if (epoll_fd < 0) {
		perror("Error while creating epoll instance");
		return;
	}

	inotify_fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
	if (inotify_fd < 0) {
		perror("Error while creating inotify instance");
		return;
	}

	epoll_event ev{.events = EPOLLIN};
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, inotify_fd, &ev) < 0) {
		perror("Error while adding inotify fd to epoll");
		return;
	}
#endif
}

FileWatcher::~FileWatcher() {
#if defined(OS_LINUX)
	if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, inotify_fd, NULL) < 0) {
		perror("Error while removing inotify fd from epoll");
		return;
	}

	/*if (inotify_rm_watch(inotify_fd, wd) < 0) {
		perror("Error while removing inotify watch descriptor");
		return;
	}*/

	if (close(inotify_fd) < 0) {
		perror("Error while destroying inotify instance");
		return;
	}

	if (close(epoll_fd) < 0) {
		perror("Error while destroying epoll instance");
		return;
	}
#endif
}

void FileWatcher::add(const Path& path) {
#if defined(OS_LINUX)
	int wd = inotify_add_watch(inotify_fd, path.c_str(), IN_ALL_EVENTS);
	if (wd < 0) {
		perror("Error while registering inotify watch");
		return;
	}
	items.add({wd, path});
#endif
}

void FileWatcher::iterate() {
#if defined(OS_LINUX)
	while (true) {
		epoll_event event = {};
		if (epoll_wait(epoll_fd, &event, 1, 0) < 0) {
			perror("Error while waiting for IO events");
			return;
		}
		constexpr int BUF_LEN = sizeof(inotify_event) + NAME_MAX + 1;
		char buf[BUF_LEN] __attribute__ ((aligned(8)));

		const ssize_t read_bytes = read(inotify_fd, &buf, BUF_LEN);
		if (read_bytes < 0) {
			//perror("Error while reading event data");
			return;
		}

		int offset = 0;
		while (read_bytes - offset >= sizeof(inotify_event)) {
			auto in_event = (inotify_event*)&buf[offset];

			string name;
			if (in_event->len > 0)
				name = string(&buf[offset + sizeof(inotify_event)]);//, in_event->len - 1);

			for (auto& i: items)
				if (i.w == in_event->wd) {
					if (in_event->mask == IN_CLOSE_WRITE)
						out_edited(i.path | name);
					if (in_event->mask == IN_DELETE)
						out_deleted(i.path | name);
					if (in_event->mask == IN_DELETE_SELF)
						out_edited(i.path);
					if (in_event->mask == IN_CREATE)
						out_created(i.path | name);
//					if (in_event->mask == IN_MOVE)
//						msg_write("MOVE: " + str(name));
					if (in_event->mask == IN_MOVE_SELF)
						out_moved({i.path, ""});
					if (in_event->mask == IN_MOVED_FROM)
						i.last_move_from = name;
					if (in_event->mask == IN_MOVED_TO)
						out_moved({i.path | i.last_move_from, i.path | name});
				}
			offset += sizeof(inotify_event) + in_event->len;
		}
	}
#endif
}
} // obs

#endif
