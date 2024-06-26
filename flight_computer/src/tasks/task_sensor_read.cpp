/// Copyright (C) 2020, 2024 Control and Telemetry Systems GmbH
///
/// SPDX-License-Identifier: GPL-3.0-or-later

#include "tasks/task_sensor_read.hpp"
#include "cmsis_os.h"
#include "config/globals.hpp"
#include "flash/recorder.hpp"

#include "sensors/ms5607.hpp"
#include "util/log.h"
#include "util/task_util.hpp"

/** Private Function Declarations **/

namespace task {

baro_data_t SensorRead::GetBaro(uint8_t index) const noexcept { return m_baro_data[index]; }

imu_data_t SensorRead::GetImu(uint8_t index) const noexcept { return m_imu_data[index]; }

/** Exported Function Definitions **/

/**
 * @brief Function implementing the task_sens_read thread.
 * @param argument: Not used
 * @retval None
 */
[[noreturn]] void SensorRead::Run() noexcept {
  /* Initialize IMU data variables */
  m_barometer->Prepare(sensor::Ms5607::Request::kTemperature);
  osDelay(5);

  uint32_t tick_count = osKernelGetTickCount();
  /* This task is sampled with 2 times the control sampling frequency to maximize speed of the barometer. In one
   * timestep the Baro pressure is read out and then the Baro Temperature. The other sensors are only read out one in
   * two times. */
  constexpr uint32_t tick_update = sysGetTickFreq() / static_cast<uint32_t>(2 * CONTROL_SAMPLING_FREQ);
  while (true) {
    // Readout the baro register
    m_barometer->Read();

    // Prepare new readout for the baro
    if (m_current_readout == SensorRead::BaroReadoutType::kReadBaroPressure) {
      m_barometer->Prepare(sensor::Ms5607::Request::kPressure);
      m_current_readout = SensorRead::BaroReadoutType::kReadBaroTemperature;
    } else {
      m_barometer->Prepare(sensor::Ms5607::Request::kTemperature);
      m_current_readout = SensorRead::BaroReadoutType::kReadBaroPressure;
      /* For Simulator */
      if (simulation_started) {
        for (int i = 0; i < NUM_BARO; i++) {
          m_baro_data[i].pressure = global_baro_sim[i].pressure;
        }
      } else {
        m_barometer->GetMeasurement(m_baro_data[0].pressure, m_baro_data[0].temperature);
      }

      /* Save Barometric Data */
      for (int i = 0; i < NUM_BARO; i++) {
        record(tick_count, add_id_to_record_type(BARO, i), &(m_baro_data[0]));
      }

      /* Read and Save IMU Data */
      for (int i = 0; i < NUM_IMU; i++) {
        if (simulation_started) {
          m_imu_data[i].acc = global_imu_sim[i].acc;
        } else {
          if (imu_initialized[i]) {
            // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
            m_imu->ReadGyroRaw(reinterpret_cast<int16_t *>(&m_imu_data[i].gyro));
            m_imu->ReadAccelRaw(reinterpret_cast<int16_t *>(&m_imu_data[i].acc));
            // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
          }
        }
        record(tick_count, add_id_to_record_type(IMU, i), &(m_imu_data[i]));
      }
    }

    tick_count += tick_update;
    osDelayUntil(tick_count);
  }
}

}  // namespace task
