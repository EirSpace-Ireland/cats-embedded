/*
 * CATS Flight Software
 * Copyright (C) 2021 Control and Telemetry Systems
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

#include "control/calibration.h"
#include "util/log.h"

void calibrate_imu(const vf32_t *accel_data, calibration_data_t *calibration) {
  /* first get the largest vector */
  if (fabsf(accel_data->x) >= fabsf(accel_data->y)) {
    if (fabsf(accel_data->x) >= fabsf(accel_data->z)) {
      calibration->axis = 0;
    } else {
      calibration->axis = 2;
    }
  } else {
    if (fabsf(accel_data->y) >= fabsf(accel_data->z)) {
      calibration->axis = 1;
    } else {
      calibration->axis = 2;
    }
  }

  /* Then get the angle (or here the cos(angle)) between vector and gravity for
   * further use */
  switch (calibration->axis) {
    case 0:
      calibration->angle = accel_data->x / GRAVITY;
      if (fabsf(calibration->angle) < 0.3f) calibration->angle = 0.3f;
      log_info("Calibration chose X Axis with invcos(alpha)*1000 = %ld", (int32_t)(1000 * calibration->angle));
      break;
    case 1:
      calibration->angle = accel_data->y / GRAVITY;
      if (fabsf(calibration->angle) < 0.3f) calibration->angle = 0.3f;
      log_info("Calibration chose Y Axis with invcos(alpha)*1000 = %ld", (int32_t)(1000 * calibration->angle));
      break;
    case 2:
      calibration->angle = accel_data->z / GRAVITY;
      if (fabsf(calibration->angle) < 0.3f) calibration->angle = 0.3f;
      log_info("Calibration chose Z Axis with invcos(alpha)*1000 = %ld", (int32_t)(1000 * calibration->angle));
      break;
    default:
      break;
  }
}

bool compute_gyro_calibration(const vf32_t *gyro_data, calibration_data_t *calibration) {
  static int16_t calibration_counter = 0;
  static vf32_t first_gyro_data = {.x = 0, .y = 0, .z = 0};
  static vf32_t averaged_gyro_data = {.x = 0, .y = 0, .z = 0};

  /* compute gyro error */
  vf32_t vector_error;
  vector_error.x = fabsf(first_gyro_data.x - gyro_data->x);
  vector_error.y = fabsf(first_gyro_data.y - gyro_data->y);
  vector_error.z = fabsf(first_gyro_data.z - gyro_data->z);

  /* check if the gyro error is inside the bounds
   * if yes, increase counter and compute averaged gyro data
   * if not, reset counter and reset averaged gyro data
   */
  if ((vector_error.x < GYRO_ALLOWED_ERROR_SI) && (vector_error.y < GYRO_ALLOWED_ERROR_SI) &&
      (vector_error.z < GYRO_ALLOWED_ERROR_SI)) {
    calibration_counter++;
    averaged_gyro_data.x += gyro_data->x / (float)GYRO_NUM_SAME_VALUE;
    averaged_gyro_data.y += gyro_data->y / (float)GYRO_NUM_SAME_VALUE;
    averaged_gyro_data.z += gyro_data->z / (float)GYRO_NUM_SAME_VALUE;
  } else {
    calibration_counter = 0;
    averaged_gyro_data.x = 0;
    averaged_gyro_data.y = 0;
    averaged_gyro_data.z = 0;
    first_gyro_data = *gyro_data;
  }

  /* if the counter achieved the defined value, calibrate gyro */
  if (calibration_counter > GYRO_NUM_SAME_VALUE) {
    memcpy(&calibration->gyro_calib, &averaged_gyro_data, sizeof(averaged_gyro_data));
    return true;
  }

  return false;
}

void calibrate_gyro(const calibration_data_t *calibration, vf32_t *gyro_data) {
  gyro_data->x = gyro_data->x - calibration->gyro_calib.x;
  gyro_data->y = gyro_data->y - calibration->gyro_calib.y;
  gyro_data->z = gyro_data->z - calibration->gyro_calib.z;
}

void calibrate_magneto(magneto_data_t *magneto_data, magneto_calibration_data_t *calibration_data) {
  float test_radii[10] = {2.0f, 2.1f, 2.2f, 2.3f, 2.4f, 2.5f, 2.6f, 2.7f, 2.8f, 2.9f};
  float test_bias[10] = {2.0f, 2.1f, 2.2f, 2.3f, 2.4f, 2.5f, 2.6f, 2.7f, 2.8f, 2.9f};
  int32_t smallest_indices[4] = {0};
  float smallest_value = 10000;
  float value;
  for (int radius_i = 0; radius_i < 10; radius_i++) {
    for (int bias_x_i = 0; bias_x_i < 10; bias_x_i++) {
      for (int bias_y_i = 0; bias_y_i < 10; bias_y_i++) {
        for (int bias_z_i = 0; bias_z_i < 10; bias_z_i++) {
          value = test_radii[radius_i] * test_radii[radius_i] - magneto_data->x * magneto_data->x -
                  magneto_data->y * magneto_data->y - magneto_data->z * magneto_data->z;
          value += -2.0f * magneto_data->x * test_bias[bias_x_i] - 2.0f * magneto_data->y * test_bias[bias_y_i] -
                   2.0f * magneto_data->z * test_bias[bias_z_i];
          value += -test_bias[bias_x_i] * test_bias[bias_x_i] - test_bias[bias_y_i] * test_bias[bias_y_i] -
                   test_bias[bias_z_i] * test_bias[bias_z_i];
          if (value < smallest_value) {
            smallest_value = value;
            smallest_indices[0] = radius_i;
            smallest_indices[1] = bias_x_i;
            smallest_indices[2] = bias_y_i;
            smallest_indices[3] = bias_z_i;
          }
        }
      }
    }
  }

  calibration_data->magneto_radius = test_radii[smallest_indices[0]];
  calibration_data->magneto_beta[0] = test_radii[smallest_indices[1]];
  calibration_data->magneto_beta[1] = test_radii[smallest_indices[2]];
  calibration_data->magneto_beta[2] = test_radii[smallest_indices[3]];
}