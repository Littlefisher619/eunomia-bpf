/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, Yusheng Zheng
 * All rights reserved.
 */
#ifndef EUNOMIA_BPF_HPP_
#define EUNOMIA_BPF_HPP_

#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

#include "eunomia-meta.hpp"
#include "eunomia-config.h"

struct bpf_map;
struct bpf_program;
struct bpf_link;
struct bpf_object_skeleton;
struct ring_buffer;
struct bpf_object;
struct perf_buffer;

namespace eunomia
{
  class eunomia_ebpf_program
  {
   private:
    /// create an ebpf skeleton
    int create_prog_skeleton(void);

    /// check for types and create export format

    /// check the types from ebpf source code and export header
    /// create export formats for correctly print the data,
    /// and used by user space.
    int check_for_meta_types_and_create_export_format(ebpf_export_types_meta_data &types);
    void check_and_add_export_type(ebpf_rb_export_field_meta_data &field, std::size_t width);

    /// wait and polling the ring buffer map
    int wait_and_poll_from_rb(std::size_t id);

    /// wait and polling from perf event
    int wait_and_poll_from_perf_event(std::size_t id);

    /// simply wait for the program to exit
    /// use in no export data mode
    int wait_for_no_export_program(void);

    int check_export_maps(void);

    /// a default printer to print event data
    void print_default_export_event_with_time(const char *event) const;

   private:
    /// is the polling ring buffer loop exiting?
    std::mutex exit_mutex;
    volatile bool exiting = false;
    /// meta data storage
    eunomia_ebpf_meta_data meta_data;
    eunomia_config config_data;

    /// buffer to base 64 decode
    bpf_object *obj = nullptr;
    std::vector<char> base64_decode_buffer = {};
    std::vector<bpf_map *> maps = {};
    std::vector<bpf_program *> progs = {};
    std::vector<bpf_link *> links = {};
    bpf_object_skeleton *skeleton = nullptr;

    /// user define handler to process export data
    void (*user_export_event_handler)(const char *event) = nullptr;

    /// used for processing maps and free them
    // FIXME: use smart pointer instead of raw pointer
    ring_buffer *ring_buffer_map = nullptr;
    perf_buffer *perf_buffer_map = nullptr;

   public:
    /// create a ebpf program from json config str
    eunomia_ebpf_program(const std::string &json_str);
    eunomia_ebpf_program(const eunomia_ebpf_program &) = delete;
    eunomia_ebpf_program(eunomia_ebpf_program &&) = delete;
    ~eunomia_ebpf_program()
    {
      stop_and_clean();
    }
    /// start running the ebpf program

    /// load and attach the ebpf program to the kernel to run the ebpf program
    /// if the ebpf program has maps to export to user space, you need to call
    /// the wait and export.
    int run(void);

    /// wait for the program to exit

    /// if the program has a ring buffer or perf event to export data
    /// to user space, the program will help load the map info and poll the
    /// events automatically.
    int wait_and_export(void);

    /// stop, detach, and clean up memory

    /// This is thread safe with wait_and_export.
    /// it will notify the wait_and_export to exit and
    /// wait until it exits.
    void stop_and_clean(void);

    /// get the name id of the ebpf program
    const std::string &get_program_name(void) const;

    /// print event with meta data;
    /// used for export call backs: ring buffer and perf events
    /// provide a common interface to print the event data
    void handler_export_events(const char *event) const;
  };
}  // namespace eunomia

#endif