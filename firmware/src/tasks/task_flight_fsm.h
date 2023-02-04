/*
 * CATS Flight Software
 * Copyright (C) 2023 Control and Telemetry Systems
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "task.h"
#include "task_preprocessing.h"
#include "task_state_est.h"

namespace task {

class FlightFsm final : public Task<FlightFsm, 512> {
 public:
  explicit FlightFsm(const Preprocessing& task_preprocessing, const StateEstimation& task_state_estimation)
      : m_task_preprocessing{task_preprocessing}, m_task_state_estimation{task_state_estimation} {}

 private:
  const Preprocessing& m_task_preprocessing;
  const StateEstimation& m_task_state_estimation;
  [[noreturn]] void Run() noexcept override;
};

}  // namespace task
