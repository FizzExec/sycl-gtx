#pragma once

// 3.6.4 Accessors

#include "access.h"
#include "ranges.h"
#include "../data_ref.h"

namespace cl {
namespace sycl {

namespace detail {

// Forward declarations
template <typename DataType, int dimensions>
struct cl::sycl::buffer;
namespace kernel_ {
	class source;
}

class accessor_base {
protected:
	template <class ResourceType>
	static string_class obtain_resource_name(ResourceType* resource) {
		return resource->resource_name;
	}
};

// 3.6.4.3 Core accessors class
template <typename DataType, int dimensions, access::mode mode, access::target target>
class accessor_core : public accessor_base {
public:
	// Returns the size of the underlying buffer in number of elements.
	size_t get_size() const;

	// Returns the cl_mem object corresponding to the access.
	// TODO: Only available when target is cl_image or cl_buffer
	virtual cl_mem get_cl_mem_object() const {
		return nullptr;
	}

	// Returns the cl_event object corresponding to the last command to access the memory object.
	// TODO: Only available when target is cl_image or cl_buffer.
	cl_event get_cl_event_object() const;

protected:
	friend class kernel_::source;

	virtual string_class get_resource_name() const {
		DSELF() << "not implemented";
		return "";
	}

	virtual void* resource() const {
		return nullptr;
	}
};

template<bool>
struct select_target;

// This does not compile with enums (at least in MSVC 2013), use ints instead
template <typename DataType, int dimensions, int mode, int target, typename = select_target<true>>
class accessor_;

template <typename DataType, int dimensions>
class accessor_buffer {
protected:
	cl::sycl::buffer<DataType, dimensions>* buf;
public:
	accessor_buffer(
		cl::sycl::buffer<DataType, dimensions>& bufferRef,
		range<dimensions> offset,
		range<dimensions> range
	) : buf(&bufferRef) {
		DSELF() << "not implemented";
	}
protected:
	cl_mem get_buffer_object() const {
		return buf->device_data.get();
	}
};

#define SYCL_ACCESSOR_CLASS(condition)															\
template <typename DataType, int dimensions, int mode, int target>								\
class accessor_<DataType, dimensions, mode, target, select_target<(condition)>>					\
	: public accessor_core<DataType, dimensions, (access::mode)mode, (access::target)target>

// 3.6.4.4 Buffer accessors
SYCL_ACCESSOR_CLASS(
	target == access::cl_buffer			||
	target == access::constant_buffer	||
	target == access::global_buffer		||
	target == access::host_buffer
), public accessor_buffer<DataType, dimensions> {
private:
public:
	// This accessor limits the processing of the buffer to the [offset, offset + range] for every dimension
	// Any other parts of the buffer will be unaffected
	accessor_(
		cl::sycl::buffer<DataType, dimensions>& bufferRef,
		range<dimensions> offset,
		range<dimensions> range
	) : accessor_buffer(bufferRef, offset, range) {}

	accessor_(cl::sycl::buffer<DataType, dimensions>& bufferRef)
		: accessor_(
			bufferRef,
			detail::empty_range<dimensions>(),
			bufferRef.get_range()
		) {}

	virtual cl_mem get_cl_mem_object() const override {
		return get_buffer_object();
	}

protected:
	virtual string_class get_resource_name() const override {
		return obtain_resource_name(buf);
	}

	virtual void* resource() const override {
		return buf;
	}
};

} // namespace detail


template <typename DataType, int dimensions, access::mode mode, access::target target = access::global_buffer>
class accessor;

#define SYCL_ADD_ACCESSOR(mode)												\
	template <typename DataType, int dimensions, access::target target>		\
	class accessor<DataType, dimensions, mode, target>						\
		: public detail::accessor_<DataType, dimensions, mode, target>


// 3.6.4.4 Buffer accessors

SYCL_ADD_ACCESSOR(access::read) {
public:
#if MSVC_LOW
	accessor(buffer<DataType, dimensions>& targette)
		: detail::accessor_<DataType, dimensions, access::read, target>(targette) {}
#else
	using detail::accessor_<DataType, dimensions, access::read, target>::accessor_;
#endif
	// Read element from target data.
	//detail::__read_ref operator[](id<dimensions>) const {
	//	DSELF() << "not implemented";
	//	return detail::__read_ref();
	//}
};

SYCL_ADD_ACCESSOR(access::write) {
public:
	accessor(buffer<DataType, dimensions>& targette)
		: detail::accessor_<DataType, dimensions, access::write, target>(targette) {}

	detail::__write_ref operator[](int index) const {
		detail::kernel_::source::register_resource(*this);
		return detail::__write_ref(
			get_resource_name() + "[" + std::to_string(index) + "]"
		);
	}

	detail::__write_ref operator[](id<dimensions> index) const {
		detail::kernel_::source::register_resource(*this);
		return detail::__write_ref(
			get_resource_name() + "[" + detail::kernel_::source::get_name(index) + "]"
		);
	}
};

SYCL_ADD_ACCESSOR(access::atomic) {
public:
	accessor(buffer<DataType, dimensions>& targette)
		: detail::accessor_<DataType, dimensions, access::atomic, target>(targette) {}
	// Atomic reference to element from target data.
	//detail::__atomic_ref<DataType> operator[](id<dimensions>) const;
};

} // namespace sycl
} // namespace cl

#undef SYCL_ACCESSOR_CLASS
#undef SYCL_ADD_ACCESSOR
