#pragma once

// 3.2.6 Command group class

#include "queue.h"

namespace cl {
namespace sycl {

struct event {};

namespace helper {

struct command_group_interface {
	virtual event kernel_event() {
		return event();
	}
	virtual event start_event() {
		return event();
	}
	virtual event complete_event() {
		return event();
	}
};

// This class is the actual implementation of the command_group
// Three classes are needed as a workaround to the inability to deduce templated class constructor arguments
template <typename functorT>
class command_group : public command_group_interface {
public:
	command_group(queue q, functorT functor) {}
	virtual event kernel_event() override {
		return event();
	}
	virtual event start_event() override {
		return event();
	}
	virtual event complete_event() override {
		return event();
	}
};

} // namespace helper


// A command group in SYCL as it is defined in 2.3.1 includes a kernel to be enqueued along with all the commands
// for queued data transfers that it needs in order for its execution to be successful.
class command_group {
private:
	helper::command_group_interface group;
public:
	// typename functorT: kernel functor or lambda function
	template <typename functorT>
	command_group(queue q, functorT functor)
		: group(helper::command_group<functorT>(q, functor))
	{}
	event kernel_event() {
		group.kernel_event();
	}
	event start_event() {
		group.start_event();
	}
	event complete_event() {
		group.complete_event();
	}
};

} // namespace sycl
} // namespace cl
