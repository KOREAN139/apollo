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

#include "modules/dreamview/backend/instrumentation/instrumentation_service.h"

#include "google/protobuf/util/json_util.h"

#include "cyber/common/file.h"
#include "modules/common/adapters/adapter_gflags.h"
#include "modules/dreamview/backend/common/dreamview_gflags.h"

namespace apollo {
namespace dreamview {

using apollo::cyber::common::GetProtoFromASCIIFile;
using apollo::cyber::common::SetProtoToASCIIFile;

using Json = nlohmann::json;
using google::protobuf::util::JsonStringToMessage;
using google::protobuf::util::MessageToJsonString;

using apollo::perception::PerceptionObstacle;
using apollo::perception::PerceptionObstacles;
using apollo::perception::SensorMeasurement;
using apollo::perception::TrafficLight;
using apollo::perception::TrafficLightDetection;
using apollo::perception::V2XInformation;
using apollo::planning::ADCTrajectory;
using apollo::planning::DecisionResult;
using apollo::planning::StopReasonCode;
using apollo::planning_internal::PlanningData;
using apollo::prediction::ObstacleInteractiveTag;
using apollo::prediction::ObstaclePriority;
using apollo::prediction::PredictionObstacle;
using apollo::prediction::PredictionObstacles;

namespace {
} //namespace

InstrumentationService::InstrumentationService(
                WebSocketHandler *instrumentation_ws)
        : node_(cyber::CreateNode("instrumentation")),
          instrumentation_ws_(instrumentation_ws)
{
        InitReaders();
        RegisterMessageHandlers();
}

void InstrumentationService::InitReaders()
{
        perception_traffic_light_reader_ =
                node_->CreateReader<TrafficLightDetection>(
                                FLAGS_traffic_light_detection_topic);
        prediction_obstacle_reader_ =
                node_->CreateReader<PredictionObstacles>(
                                FLAGS_prediction_topic);
        planning_reader_ =
                node_->CreateReader<ADCTrajectory>(
                                FLAGS_planning_trajectory_topic);
}

void InstrumentationService::RegisterMessageHandlers()
{
        instrumentation_ws_->RegisterMessageHandler(
                "RequestInstrumentationData",
                [this](const Json &json, WebSocketHandler::Connection *conn) {
                        Json response;
                        instrumentation_ws_->SendData(conn, response.dump());
                });
}

template<>
void InstrumentationService::UpdateData(const PredictionObstacles &obstacles)
{
}

template<>
void InstrumentationService::UpdateData(
                const TrafficLightDetection &traffic_light_detection)
{
}

template<>
void InstrumentationService::UpdateData(const ADCTrajectory &trajectory)
{
}

}  // namespace dreamview
}  // namespace apollo

