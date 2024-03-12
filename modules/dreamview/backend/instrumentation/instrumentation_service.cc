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
#include "modules/common/util/json_util.h"
#include "modules/map/hdmap/hdmap_util.h"
#include "modules/dreamview/backend/common/dreamview_gflags.h"

namespace apollo {
namespace dreamview {

using apollo::common::util::JsonUtil;
using apollo::common::PointENU;
using apollo::hdmap::HDMapUtil;
using apollo::hdmap::LaneInfoConstPtr;

using apollo::cyber::common::GetProtoFromBinaryFile;

using Json = nlohmann::json;
using google::protobuf::util::JsonStringToMessage;
using google::protobuf::util::MessageToJsonString;

using apollo::hdmap::Map;
using apollo::perception::TrafficLightDetection;
using apollo::planning::ADCTrajectory;
using apollo::planning::PlanningDebugMessage;
using apollo::planning::PadMessage;
using apollo::planning::DrivingAction;
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
        planning_debug_reader_ =
                node_->CreateReader<PlanningDebugMessage>(
                                FLAGS_planning_debug_data_topic);
        pad_msg_writer_ =
                node_->CreateWriter<PadMessage>(
                                FLAGS_planning_pad_topic);
}

void InstrumentationService::RegisterMessageHandlers()
{
        instrumentation_ws_->RegisterMessageHandler(
                "RequestInstrumentationData",
                [this](const Json &json, WebSocketHandler::Connection *conn) {
                        Update();
                        const auto instrumentation_json =
                                JsonUtil::ProtoToTypedJson(
                                                "Instrumentation",
                                                instrumentation_);
                        instrumentation_ws_->SendData(
                                        conn, instrumentation_json.dump());
                        //
                        // size_t size = instrumentation_.ByteSizeLong();
                        // void *data = malloc(size);
                        // instrumentation_.SerializeToArray(data, size);
                        // instrumentation_ws_->SendBinaryData(
                        //                 conn, (const std::string &) data);
                });

        instrumentation_ws_->RegisterMessageHandler(
                "RequestMapData",
                [this](const Json &json, WebSocketHandler::Connection *conn) {
                        Map hdmap_ = GetMapData();
                        const auto hdmap_json = JsonUtil::ProtoToTypedJson(
                                        "HDMap", hdmap_);
                        instrumentation_ws_->SendData(conn, hdmap_json.dump());
                });

        instrumentation_ws_->RegisterMessageHandler(
                "RequestNearestLane",
                [this](const Json &json, WebSocketHandler::Connection *conn) {
                        double x, y;
                        if (!json.contains("point")) {
                                AERROR << "Failed to parse point from json";
                                return;
                        }
                        x = json["point"]["x"];
                        y = json["point"]["y"];
 
                        double s, l;
                        LaneInfoConstPtr nearest_lane;
                        GetNearestLane(x, y, &nearest_lane, &s, &l);

                        const auto response_json = JsonUtil::ProtoToTypedJson(
                                        "Lane", nearest_lane->lane());
                        instrumentation_ws_->SendData(conn, response_json.dump());
                });

        instrumentation_ws_->RegisterMessageHandler(
                "RequestPullOver",
                [this](const Json &json, WebSocketHandler::Connection *conn) {
                        PadMessage pad_msg;
                        pad_msg.set_action(DrivingAction::PULL_OVER);
                        pad_msg_writer_->Write(pad_msg);
                });

        instrumentation_ws_->RegisterMessageHandler(
                "RequestEmergencyStop",
                [this](const Json &json, WebSocketHandler::Connection *conn) {
                        PadMessage pad_msg;
                        pad_msg.set_action(DrivingAction::STOP);
                        pad_msg_writer_->Write(pad_msg);
                });

        instrumentation_ws_->RegisterMessageHandler(
                "RequestResumeCruise",
                [this](const Json &json, WebSocketHandler::Connection *conn) {
                        PadMessage pad_msg;
                        pad_msg.set_action(DrivingAction::RESUME_CRUISE);
                        pad_msg_writer_->Write(pad_msg);
                });
}

void InstrumentationService::Update()
{
        instrumentation_.Clear();
        node_->Observe();
        UpdateWithLatestObserved(perception_traffic_light_reader_.get());
        UpdateWithLatestObserved(prediction_obstacle_reader_.get());
        UpdateWithLatestObserved(planning_reader_.get());
        UpdateWithLatestObserved(planning_debug_reader_.get());
}

apollo::hdmap::Map InstrumentationService::GetMapData()
{
        Map map_proto;
        std::string sim_map_path = apollo::hdmap::SimMapFile();
        GetProtoFromBinaryFile(sim_map_path, &map_proto);

        return map_proto;
}

bool InstrumentationService::GetNearestLane(const double x, const double y,
                                LaneInfoConstPtr *nearest_lane,
                                double *nearest_s, double *nearest_l)
{
        boost::shared_lock<boost::shared_mutex> reader_lock(mutex_);

        PointENU point;
        point.set_x(x);
        point.set_y(y);
        const apollo::hdmap::HDMap *hdmap = HDMapUtil::BaseMapPtr();
        if (hdmap->GetNearestLane(point, nearest_lane, nearest_s, nearest_l) < 0) {
                return false;
        }
        return true;
}

template<>
void InstrumentationService::UpdateData(const PredictionObstacles &obstacles)
{
        instrumentation_.mutable_prediction_obstacles()->CopyFrom(obstacles);
}

template<>
void InstrumentationService::UpdateData(
                const TrafficLightDetection &traffic_light_detection)
{
        instrumentation_
                .mutable_traffic_light_detection()
                ->CopyFrom(traffic_light_detection);
}

template<>
void InstrumentationService::UpdateData(const ADCTrajectory &trajectory)
{
        instrumentation_.mutable_trajectory()->CopyFrom(trajectory);
}

template<>
void InstrumentationService::UpdateData(const PlanningDebugMessage &planning_debug_msg)
{
        instrumentation_.mutable_planning_debug_message()->CopyFrom(planning_debug_msg);
}

}  // namespace dreamview
}  // namespace apollo
