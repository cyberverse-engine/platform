#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <thread>

#include "cyberverse/platform.hpp"

using namespace cyberverse::platform;

TEST_CASE("timer reports deltas and frame counts", "[timer]") {
  auto host = make_platform(backend_type::headless);
  auto frame_timer = host->create_timer();

  REQUIRE(frame_timer != nullptr);
  CHECK(frame_timer->frame_count() == 0);
  CHECK(frame_timer->delta_seconds() == Catch::Approx(0.0));
  CHECK(frame_timer->total_seconds() == Catch::Approx(0.0));

  frame_timer->tick();
  CHECK(frame_timer->frame_count() == 1);
  CHECK(frame_timer->delta_seconds() == Catch::Approx(0.0));
  CHECK(frame_timer->total_seconds() >= 0.0);

  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  frame_timer->tick();
  CHECK(frame_timer->frame_count() == 2);
  CHECK(frame_timer->delta_seconds() > 0.001);
  CHECK(frame_timer->total_seconds() >= frame_timer->delta_seconds());

  const auto previous_total = frame_timer->total_seconds();
  std::this_thread::sleep_for(std::chrono::milliseconds(2));
  frame_timer->tick();
  CHECK(frame_timer->frame_count() == 3);
  CHECK(frame_timer->total_seconds() > previous_total);
}

TEST_CASE("timers keep their platform alive", "[timer][lifetime]") {
  auto host = make_platform(backend_type::headless);
  auto frame_timer = host->create_timer();

  host.reset();
  REQUIRE(frame_timer != nullptr);
  frame_timer->tick();
  CHECK(frame_timer->frame_count() == 1);
}
