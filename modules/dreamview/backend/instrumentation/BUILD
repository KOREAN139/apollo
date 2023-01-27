load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
load("//tools:cpplint.bzl", "cpplint")

package(default_visibility = ["//visibility:public"])

DREAMVIEW_COPTS = ['-DMODULE_NAME=\\"dreamview\\"']

cc_library(
    name = "instrumentation_service",
    srcs = ["instrumentation_service.cc"],
    hdrs = ["instrumentation_service.h"],
    copts = DREAMVIEW_COPTS,
    deps = [
        "//cyber",
        "//modules/common/adapters:adapter_gflags",
        "//modules/common/util",
        "//modules/common/util:json_util",
        "//modules/dreamview/proto:instrumentation_cc_proto",
        "//modules/dreamview/backend/common:dreamview_gflags",
        "//modules/dreamview/backend/handlers:websocket_handler",
        "//modules/map/hdmap:hdmap_util",
        "//modules/map/proto:map_cc_proto",
        "//modules/perception/proto:traffic_light_detection_cc_proto",
        "//modules/planning/proto:planning_cc_proto",
        "//modules/prediction/proto:prediction_obstacle_cc_proto",
        "@boost",
        "@com_github_nlohmann_json//:json",
    ],
)

cpplint()
