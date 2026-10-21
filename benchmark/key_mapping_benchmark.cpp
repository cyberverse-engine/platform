#include <benchmark/benchmark.h>

#include "cyberverse/platform/key_mapping.hpp"

using namespace cyberverse::platform;

namespace {

void setup_mapping(key_mapping& mapping) {
  mapping.bind("move_forward", key_id::key_w);
  mapping.bind("move_forward", key_id::key_up);
  mapping.bind("move_back", key_id::key_s);
  mapping.bind("move_left", key_id::key_a);
  mapping.bind("move_right", key_id::key_d);
  mapping.bind("jump", key_id::key_space);
  mapping.bind("sprint", key_id::key_left_shift);
  mapping.bind("fire", mouse_button::left);
  mapping.bind("save", key_id::key_s, mod_keys::control);
}

} // namespace

static void BM_KeyMappingUpdate(benchmark::State& state) {
  raw_input_state input;
  key_mapping mapping;
  setup_mapping(mapping);

  for (auto _ : state) {
    input.begin_frame();
    input.set_mod_keys(mod_keys::shift);
    input.key_down(key_id::key_w);
    input.mouse_button_down(mouse_button::left);
    mapping.update(input);
    input.key_up(key_id::key_w);
    input.mouse_button_up(mouse_button::left);

    benchmark::DoNotOptimize(mapping.state("move_forward"));
    benchmark::DoNotOptimize(mapping.state("fire"));
  }
}

static void BM_KeyMappingQueries(benchmark::State& state) {
  raw_input_state input;
  key_mapping mapping;
  setup_mapping(mapping);
  input.key_down(key_id::key_w);
  mapping.update(input);

  for (auto _ : state) {
    bool down = mapping.is_down("move_forward");
    down = down || mapping.is_pressed("jump");
    benchmark::DoNotOptimize(down);
  }
}

BENCHMARK(BM_KeyMappingUpdate);
BENCHMARK(BM_KeyMappingQueries);

BENCHMARK_MAIN();
