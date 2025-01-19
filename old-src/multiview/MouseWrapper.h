/*
 * MouseWrapper.h
 *
 *  Created on: Jan 15, 2021
 *      Author: michi
 */

#ifndef SRC_MULTIVIEW_MOUSEWRAPPER_H_
#define SRC_MULTIVIEW_MOUSEWRAPPER_H_

namespace hui {
	class Window;
}
namespace MultiView {
	class MultiView;
}

namespace MouseWrapper {
	void start(hui::Window *w);
	void stop(hui::Window *w);
	void update(MultiView::MultiView *view);
}




#endif /* SRC_MULTIVIEW_MOUSEWRAPPER_H_ */
