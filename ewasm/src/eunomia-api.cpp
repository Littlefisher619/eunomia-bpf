
#include "ewasm/ewasm.hpp"
#include <cassert>

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <eunomia/eunomia-bpf.hpp>

using namespace eunomia;

int
ewasm_program::create_bpf_program(char *ebpf_json)
{
    int id = (int)bpf_program_map.size();
    bpf_program_map.emplace(id, std::make_unique<eunomia_ebpf_program>());
    int res = bpf_program_map[id]->load_json_config(ebpf_json);
    if (res < 0) {
        return res;
    }
    return (int)id;
}

int
ewasm_program::run_bpf_program(int id)
{
    auto bpf_program = bpf_program_map.find(id);
    if (bpf_program == bpf_program_map.end()) {
        return -1;
    }
    return bpf_program->second->run();
}

void
handle_eunomia_event(void *ctx, const char *e)
{
    assert("ctx is null" && ctx != nullptr);
    ewasm_program *program = (ewasm_program *)ctx;
    program->process_event(e);
}

int
ewasm_program::wait_and_poll_bpf_program(int id)
{
    auto bpf_program = bpf_program_map.find(id);
    if (bpf_program == bpf_program_map.end()) {
        return -1;
    }
    return bpf_program->second->wait_and_poll_to_handler(
        export_format_type::EXPORT_JSON, handle_eunomia_event, this);
}

extern "C" {
int
create_bpf(wasm_exec_env_t exec_env, char *ebpf_json, int str_len)
{
    ewasm_program *program =
        (ewasm_program *)wasm_runtime_get_user_data(exec_env);
    assert("program is null" && program != nullptr);
    return program->create_bpf_program(ebpf_json);
}

int
run_bpf(wasm_exec_env_t exec_env, int id)
{
    ewasm_program *program =
        (ewasm_program *)wasm_runtime_get_user_data(exec_env);
    assert("program is null" && program != nullptr);
    return program->run_bpf_program(id);
}

int
wait_and_poll_bpf(wasm_exec_env_t exec_env, int id)
{
    ewasm_program *program =
        (ewasm_program *)wasm_runtime_get_user_data(exec_env);
    assert("program is null" && program != nullptr);
    return program->wait_and_poll_bpf_program(id);
}
}
