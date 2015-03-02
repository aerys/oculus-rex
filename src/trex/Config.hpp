/*
Copyright (c) 2014 Aerys

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#define DINO_DIST 7.f

#define WALKING_TO_FOLLOWING_TIME                           1.0f
#define FOLLOWING_TO_WALKING_TIME                           1.0f

#define TREX_ROAD_MAP                                       "model/map_road.scene"
#define TREX_ROAD_CHUNK_LENGTH                              50
#define TREX_ROAD_WIDTH                                     10
#define TREX_ROAD_CHUNK_MAX_PROPS                           10
#define TREX_ROAD_CHUNK_MIN_PROPS                           7
#define TREX_FRONT_VIEW_DISTANCE                            200
#define TREX_BACK_VIEW_DISTANCE                             200
#define TREX_CHUNK_POOL_SIZE                                10
#define TREX_FOG_COLOR                                      minko::math::Vector4::create(5.0f/ 255.0f, 5.0f/ 255.0f, 14.0f/ 255.0f, 1.0f)
#define NUM_LANES                                           3

#define CAR_BASE_SPEED                                      60.f
#define CAR_INTRO_SPEED                                     0.0f
#define CAR_WIDTH                                           1.75f
#define CAR_HEIGHT                                          1.75f
#define CAR_LENGTH                                          3.9f
#define CAR_SCORE_ENABLE

#define CAR_RUMBLE_ENABLE
#define CAR_RUMBLE_LVL                                      50
#define CAR_RUMBLE_THRESHOLD                                0.025f
#define CAR_RUMBLE_DELAY                                    5

#define LANE_WIDTH                                          (CAR_WIDTH * 1.333f)

#define AXIS_LX                                             0
#define AXIS_LY                                             1
#define AXIS_RX                                             2
#define AXIS_RY                                             3

#define MOVE_THRESHOLD                                      0.4f
#define CAMERA_THRESHOLD                                    0.3f
#define CAMERA_V_LIMIT                                      (M_PI_2 / 3.f)

#define LANE_CHANGE_DURATION                                450

#define ROAD_COLLISION_ENABLE
#define ROAD_COLLISION_SLOWDOWN                             20
#define ROAD_COLLISION_ACCELERATION                         2

#define TREX_DINO_IDLE_STATE_DURATION                       5.0f

#define TREX_DINO_INTRO_SPEED                               35.f
#define TREX_DINO_BASE_SPEED                                60.f
#define TREX_DINO_LENGTH                                    13.f
#define TREX_DINO_STARTING_DIST                             30.f
#define TREX_DINO_DIST                                      7.f
#define TREX_DINO_DIST_WHEN_CAR_IS_SLOWN                    3.5f
#define TREX_DINO_ATTACKING_DIST                            -0.8f

#define TREX_DINO_ACCELERATING_STATE_SPEED                  (TREX_DINO_BASE_SPEED + 15.0f)
#define TREX_DINO_RECOVERING_STATE_SPEED                    (TREX_DINO_BASE_SPEED - 15.0f)

#define TREX_DINO_FOLLOWING_STATE_DELAY                     1.0f
#define TREX_DINO_SCREAMING_STATE_DURATION                  2.0f
#define TREX_DINO_ATTACKING_STATE_DURATION                  3.0f
#define TREX_DINO_WALKING_TO_SCREAMING_STATE_DELAY          1.0f
#define TREX_DINO_AFTER_ATTACKING_WALKING_STATE_DURATION    2.0f

#define TREX_GOD_MODE                                       false

#define TREX_ENABLE_LIGHTWELL
#define TREX_ENABLE_PARTICLES                               true