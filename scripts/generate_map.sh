#!/usr/bin/env bash
# ref: https://github.com/lgsvl/apollo-5.0/blob/simulator/scripts/generate_map.sh

# HOW-TO USE
# 1. make map directory under $APOLLO_ROOT_DIR/modules/map/data
# 2. put base_map.bin in map directory
# 3. put this script under $APOLLO_ROOT_DIR/scripts
# 4. run this script inside apollo docker with command `generate_map.sh MAP_DIRECTORY`

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$DIR/apollo_base.sh"

if [ $# -eq 0 ]; then
  echo "Please specify name of your map directory."
else
  dir_name=modules/map/data/$1
  bazel-bin/modules/map/tools/sim_map_generator --map_dir=${dir_name} --output_dir=${dir_name}
  bash scripts/generate_routing_topo_graph.sh --map_dir ${dir_name}
fi
