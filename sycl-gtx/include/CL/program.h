#pragma once

// 3.5.5 Program class

#include "context.h"
#include "device.h"
#include "error_handler.h"
#include "info.h"
#include "kernel.h"
#include "param_traits.h"
#include "refc.h"
#include "detail/common.h"
#include "detail/function_traits.h"
#include "detail/kernel_name.h"
#include "detail/src_handlers/invoke_source.h"
#include "detail/src_handlers/kernel_source.h"
#include <map>

namespace cl {
namespace sycl {

// Forward declarations
class kernel;
class queue;


class program {
protected:
  friend class handler;
  friend class kernel;
  friend class detail::kernel_::source;

  detail::refc<cl_program, clRetainProgram, clReleaseProgram> prog;
  bool linked = false;

  context ctx;
  vector_class<device> devices;
  std::map<::size_t, shared_ptr_class<kernel>> kernels;

  program(
    cl_program clProgram, const context& context, vector_class<device> deviceList);

  void init_kernels();
  vector_class<cl_program> get_program_pointers() const;

  void compile(
    string_class compile_options,
    ::size_t kernel_name_id,
    shared_ptr_class<kernel> kern
  );
  void report_compile_error(shared_ptr_class<kernel> kern, device& dev) const;

  template <class KernelType>
  void compile(KernelType kernFunctor, string_class compile_options = "") {
    auto src =
      detail::kernel_::constructor<typename detail::first_arg<KernelType>::type>::get(
        kernFunctor);
    auto kern = shared_ptr_class<kernel>(new kernel(true));
    kern->src = std::move(src);
    compile(compile_options, detail::kernel_name::get<KernelType>(), kern);
  }

  template <class KernelType>
  void build(KernelType kernFunctor, string_class compile_options = "") {
    compile(kernFunctor, compile_options);
    link();
  }
public:
  // Creates an empty program object for all devices associated with context
  explicit program(const context& context);

  // Creates an empty program object
  // for all devices in list associated with the context
  program(const context& context, vector_class<device> deviceList);

  // Creates a program object from a cl_program object
  program(const context& context, cl_program clProgram);

  // Creates a program by linking a list of other programs
  program(vector_class<program> programList, string_class linkOptions = "");

  // Obtains a SYCL program object from a SYCL kernel name
  // and compiles it ready-to-link
  // TODO: Can only compile well-defined functors with a public default constructor
  template <typename kernelT>
  void compile_from_kernel_name(string_class compile_options = "") {
    kernelT functor;
    compile(functor, compile_options);
  }

  // Obtains a SYCL program object from a SYCL kernel name and builds it ready-to-run
  template <typename kernelT>
  void build_from_kernel_name(string_class compile_options = "") {
    compile_from_kernel_name<kernelT>(compile_options);
    link();
  }

  // Link all compiled programs that are added in the program class
  void link(string_class linking_options = "");

  // Gets a kernel from a given name (Functor)
  template <typename kernelT>
  kernel get_kernel() const {
    return *(kernels.at(detail::kernel_name::get<kernelT>()));
  }

  bool is_linked() const {
    return linked;
  }

private:
  template <typename ReturnType, info::program param>
  struct traits
    : detail::array_traits<
    ReturnType,
    info::program,
    param,
    detail::traits_buffer_default<ReturnType>::size
    > {
    using return_t = typename detail::array_traits<
      ReturnType,
      info::program,
      param,
      detail::traits_buffer_default<ReturnType>::size
    >::return_t;
    return_t get_info(const program* p) {
      return this->get(p->prog.get());
    }
  };

  template <typename Contained_, info::program param>
  struct traits<vector_class<Contained_>, param>
    : detail::array_traits<Contained_, info::program, param> {
    using Container =
      typename detail::array_traits<Contained_, info::program, param>::Container;
    Container get_info(const program* p) {
      this->get(p->prog.get());
      return Container(
        this->param_value, this->param_value + this->actual_size / this->type_size);
    }
  };

  template <class Contained_>
  struct traits<vector_class<vector_class<Contained_>>, info::program::binaries>
    : detail::array_traits<Contained_*, info::program, info::program::binaries> {
    using DoubleContainer = vector_class<vector_class<Contained_>>;
    DoubleContainer get_info(const program* p) {
      auto binary_sizes = p->get_info<info::program::binary_sizes>();
      this->get(p->prog.get());

      DoubleContainer ret;
      static const auto inner_type_size = sizeof(Contained_);
      ::size_t i = 0;
      for(auto bin_size : binary_sizes) {
        ret.emplace_back(
          this->param_value[i], this->param_value[i] + bin_size / inner_type_size);
        ++i;
      }
      return ret;
    }
  };

public:
  template <info::program param>
  typename param_traits<info::program, param>::type get_info() const {
    return traits<param_traits_t<info::program, param>, param>().get_info(this);
  }

  // TODO
  vector_class<vector_class<unsigned char>> get_binaries() const;
  vector_class<::size_t> get_binary_sizes() const;
  vector_class<device> get_devices() const;
  string_class get_build_options() const;

  cl_program get() const {
    return prog.get();
  }
};

} // namespace sycl
} // namespace cl
