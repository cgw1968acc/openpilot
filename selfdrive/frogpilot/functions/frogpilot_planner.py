import cereal.messaging as messaging

from openpilot.common.conversions import Conversions as CV
from openpilot.selfdrive.controls.lib.desire_helper import LANE_CHANGE_SPEED_MIN
from openpilot.selfdrive.controls.lib.longitudinal_planner import A_CRUISE_MIN, get_max_accel

from openpilot.selfdrive.frogpilot.functions.frogpilot_functions import FrogPilotFunctions

class FrogPilotPlanner:
  def __init__(self, params, params_memory):
    self.fpf = FrogPilotFunctions()

    self.v_cruise = 0

    self.accel_limits = [A_CRUISE_MIN, get_max_accel(0)]

    self.update_frogpilot_params(params, params_memory)

  def update(self, carState, controlsState, modelData, mpc, sm, v_ego):
    enabled = controlsState.enabled

    v_cruise_kph = controlsState.vCruiseCluster if controlsState.vCruiseCluster != 0.0 else controlsState.vCruise
    v_cruise = v_cruise_kph * CV.KPH_TO_MS

    # Acceleration profiles
    if self.acceleration_profile == 1:
      self.accel_limits = [self.fpf.get_min_accel_eco(v_ego), self.fpf.get_max_accel_eco(v_ego)]
    elif self.acceleration_profile in (2, 3):
      self.accel_limits = [self.fpf.get_min_accel_sport(v_ego), self.fpf.get_max_accel_sport(v_ego)]
    else:
      self.accel_limits = [A_CRUISE_MIN, get_max_accel(v_ego)]

    # Update the max allowed speed
    self.v_cruise = self.update_v_cruise(carState, controlsState, enabled, modelData, v_cruise, v_ego)

    # Lane detection
    check_lane_width = self.adjacent_lanes or self.blind_spot_path
    if check_lane_width and v_ego >= LANE_CHANGE_SPEED_MIN:
      self.lane_width_left = float(self.fpf.calculate_lane_width(modelData.laneLines[0], modelData.laneLines[1], modelData.roadEdges[0]))
      self.lane_width_right = float(self.fpf.calculate_lane_width(modelData.laneLines[3], modelData.laneLines[2], modelData.roadEdges[1]))
    else:
      self.lane_width_left = 0
      self.lane_width_right = 0

  def update_v_cruise(self, carState, controlsState, enabled, modelData, v_cruise, v_ego):
    return min(v_cruise)

  def publish(self, sm, pm, mpc):
    frogpilot_plan_send = messaging.new_message('frogpilotPlan')
    frogpilot_plan_send.valid = sm.all_checks(service_list=['carState', 'controlsState'])
    frogpilotPlan = frogpilot_plan_send.frogpilotPlan

    frogpilotPlan.laneWidthLeft = self.lane_width_left
    frogpilotPlan.laneWidthRight = self.lane_width_right

    pm.send('frogpilotPlan', frogpilot_plan_send)

  def update_frogpilot_params(self, params, params_memory):
    self.is_metric = params.get_bool("IsMetric")

    custom_ui = params.get_bool("CustomUI")
    self.blind_spot_path = params.get_bool("BlindSpotPath") and custom_ui

    longitudinal_tune = params.get_bool("LongitudinalTune")
    self.acceleration_profile = params.get_int("AccelerationProfile") if longitudinal_tune else 0
    self.aggressive_acceleration = params.get_bool("AggressiveAcceleration") and longitudinal_tune
