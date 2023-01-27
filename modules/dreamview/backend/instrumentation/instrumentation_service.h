/******************************************************************************
 * Copyright 2023 Sanggu Han. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

/**
 * @file
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "absl/strings/str_cat.h"

#include "modules/dreamview/proto/instrumentation.pb.h"
#include "modules/map/proto/map.pb.h"

#include "cyber/common/log.h"
#include "cyber/cyber.h"

#include "modules/dreamview/backend/handlers/websocket_handler.h"

/**
 * @namespace apollo::dreamview
 * @brief apollo::dreamview
 */
namespace apollo {
namespace dreamview {

/**
 * @class InstrumentationService
 * @brief A wrapper around SimulationWorldService and WebSocketHandler to keep
 * pushing SimulationWorld to frontend via websocket while handling the response
 * from frontend.
 */
class InstrumentationService {
 public:
  /**
   * @brief Constructor with the websocket handler.
   * @param websocket Pointer of the websocket handler that has been attached to
   * the server.
   */
  InstrumentationService(WebSocketHandler *instrumentation_ws);

  double LastAdcTimestampSec() { return last_pushed_adc_timestamp_sec_; }

 private:
  void InitReaders();
  void RegisterMessageHandlers();
  void Update();
  apollo::hdmap::Map GetMapData();

  /**
   * @brief Update simulation world with incoming data, e.g., chassis,
   * localization, planning, perception, etc.
   */
  template <typename DataType>
  void UpdateData(const DataType &data);

  /**
   * @brief Get the latest observed data from reader to update the
   * SimulationWorld object.
   */
  template <typename MessageT>
  void UpdateWithLatestObserved(cyber::Reader<MessageT> *reader) {
    if (reader->Empty()) {
      AINFO << "Has not received any data from "
              << reader->GetChannelName();
      return;
    }

    const std::shared_ptr<MessageT> msg = reader->GetLatestObserved();
    UpdateData(*msg);
  }

  std::unique_ptr<cyber::Node> node_;

  // Instrumentation data object
  Instrumentation instrumentation_;

  // Websocket handler
  WebSocketHandler *instrumentation_ws_ = nullptr;

  // Readers
  std::shared_ptr<cyber::Reader<apollo::perception::TrafficLightDetection>>
      perception_traffic_light_reader_;
  std::shared_ptr<cyber::Reader<apollo::prediction::PredictionObstacles>>
      prediction_obstacle_reader_;
  std::shared_ptr<cyber::Reader<apollo::planning::ADCTrajectory>>
      planning_reader_;

  // Mutex to protect concurrent access to simulation_world_json_.
  // NOTE: Use boost until we have std version of rwlock support.
  boost::shared_mutex mutex_;

  std::unique_ptr<cyber::Timer> timer_;

  volatile double last_pushed_adc_timestamp_sec_ = 0.0f;
};

}  // namespace dreamview
}  // namespace apollo

