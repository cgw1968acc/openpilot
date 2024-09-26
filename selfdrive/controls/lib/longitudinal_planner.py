#!/usr/bin/env python3
import math
import numpy as np
from openpilot.common.numpy_fast import clip, interp

import cereal.messaging as messaging
from openpilot.common.conversions import Conversions as CV
from openpilot.common.filter_simple import FirstOrderFilter
from openpilot.common.simple_kalman import KF1D
from openpilot.common.realtime import DT_MDL
from openpilot.selfdrive.modeld.constants import ModelConstants
from openpilot.selfdrive.car.interfaces import ACCEL_MIN, ACCEL_MAX
from openpilot.selfdrive.controls.lib.longcontrol import LongCtrlState
from openpilot.selfdrive.controls.lib.longitudinal_mpc_lib.long_mpc import LongitudinalMpc
from openpilot.selfdrive.controls.lib.longitudinal_mpc_lib.long_mpc import T_IDXS as T_IDXS_MPC, LEAD_ACCEL_TAU
from openpilot.selfdrive.controls.lib.drive_helpers import V_CRUISE_MAX, CONTROL_N, get_speed_error
from openpilot.common.swaglog import cloudlog

LON_MPC_STEP = 0.2  # first step is 0.2s
A_CRUISE_MIN = -1.2
A_CRUISE_MAX_VALS =   [1.9, 2.0,  2.0,  1.83, 0.945, .588, .478,  .34,  .12]
A_CRUISE_MAX_BP =     [0.,  1.0,  6.1,  8.,   11.,   20.,  25.,   30.,  40.]
A_CRUISE_MIN_V =      [-0.2, -0.2, -0.36, -0.36, -0.76, -0.76, -1.0,  -1.02]
A_CRUISE_MIN_BP =     [0.,   8.33, 8.34,  11.11, 11.12, 17.49, 22.2,  30.]
CONTROL_N_T_IDX = ModelConstants.T_IDXS[:CONTROL_N]

# Lookup table for turns
_A_TOTAL_MAX_V = [1.6,  2.3, 3.2]
_A_TOTAL_MAX_BP = [14,  20., 40.]

# Kalman filter states enum
LEAD_KALMAN_SPEED, LEAD_KALMAN_ACCEL = 0, 1

def get_max_accel(v_ego):
  return interp(v_ego, A_CRUISE_MAX_BP, A_CRUISE_MAX_VALS)


def limit_accel_in_turns(v_ego, angle_steers, a_target, CP):
  """
  This function returns a limited long acceleration allowed, depending on the existing lateral acceleration
  this should avoid accelerating when losing the target in turns
  """
  # FIXME: This function to calculate lateral accel is incorrect and should use the VehicleModel
  # The lookup table for turns should also be updated if we do this
  a_total_max = interp(v_ego, _A_TOTAL_MAX_BP, _A_TOTAL_MAX_V)
  a_y = v_ego ** 2 * angle_steers * CV.DEG_TO_RAD / (CP.steerRatio * CP.wheelbase)
  a_x_allowed = math.sqrt(max(a_total_max ** 2 - a_y ** 2, 0.))

  return [a_target[0], min(a_target[1], a_x_allowed)]


def get_accel_from_plan(CP, speeds, accels):
    if len(speeds) == CONTROL_N:
      v_target_now = interp(DT_MDL, CONTROL_N_T_IDX, speeds)
      a_target_now = interp(DT_MDL, CONTROL_N_T_IDX, accels)

      v_target = interp(CP.longitudinalActuatorDelay + DT_MDL, CONTROL_N_T_IDX, speeds)
      a_target = 2 * (v_target - v_target_now) / CP.longitudinalActuatorDelay - a_target_now

      v_target_1sec = interp(CP.longitudinalActuatorDelay + DT_MDL + 1.0, CONTROL_N_T_IDX, speeds)
    else:
      v_target = 0.0
      v_target_now = 0.0
      v_target_1sec = 0.0
      a_target = 0.0
    should_stop = (v_target < CP.vEgoStopping and
                    v_target_1sec < CP.vEgoStopping)
    return a_target, should_stop


def lead_kf(v_lead: float, dt: float = 0.05):
  # Lead Kalman Filter params, calculating K from A, C, Q, R requires the control library.
  # hardcoding a lookup table to compute K for values of radar_ts between 0.01s and 0.2s
  assert dt > .01 and dt < .2, "Radar time step must be between .01s and 0.2s"
  A = [[1.0, dt], [0.0, 1.0]]
  C = [1.0, 0.0]
  #Q = np.matrix([[10., 0.0], [0.0, 100.]])
  #R = 1e3
  #K = np.matrix([[ 0.05705578], [ 0.03073241]])
  dts = [dt * 0.01 for dt in range(1, 21)]
  K0 = [0.12287673, 0.14556536, 0.16522756, 0.18281627, 0.1988689,  0.21372394,
        0.22761098, 0.24069424, 0.253096,   0.26491023, 0.27621103, 0.28705801,
        0.29750003, 0.30757767, 0.31732515, 0.32677158, 0.33594201, 0.34485814,
        0.35353899, 0.36200124]
  K1 = [0.29666309, 0.29330885, 0.29042818, 0.28787125, 0.28555364, 0.28342219,
        0.28144091, 0.27958406, 0.27783249, 0.27617149, 0.27458948, 0.27307714,
        0.27162685, 0.27023228, 0.26888809, 0.26758976, 0.26633338, 0.26511557,
        0.26393339, 0.26278425]
  K = [[interp(dt, dts, K0)], [interp(dt, dts, K1)]]

  kf = KF1D([[v_lead], [0.0]], A, C, K)
  return kf


class Lead:
  def __init__(self):
    self.dRel = 0.0
    self.yRel = 0.0
    self.vLead = 0.0
    self.aLead = 0.0
    self.vLeadK = 0.0
    self.aLeadK = 0.0
    self.aLeadTau = LEAD_ACCEL_TAU
    self.prob = 0.0
    self.status = False

    self.kf: KF1D | None = None

  def reset(self):
    self.status = False
    self.kf = None
    self.aLeadTau = LEAD_ACCEL_TAU

  def update(self, dRel: float, yRel: float, vLead: float, aLead: float, prob: float):
    self.dRel = dRel
    self.yRel = yRel
    self.vLead = vLead
    self.aLead = aLead
    self.prob = prob
    self.status = True

    if self.kf is None:
      self.kf = lead_kf(self.vLead)
    else:
      self.kf.update(self.vLead)

    self.vLeadK = float(self.kf.x[LEAD_KALMAN_SPEED][0])
    self.aLeadK = float(self.kf.x[LEAD_KALMAN_ACCEL][0])

    # Learn if constant acceleration
    if abs(self.aLeadK) < 0.5:
      self.aLeadTau = LEAD_ACCEL_TAU
    else:
      self.aLeadTau *= 0.9


class LongitudinalPlanner:
  def __init__(self, CP, init_v=0.0, init_a=0.0, dt=DT_MDL):
    self.CP = CP
    self.mpc = LongitudinalMpc(dt=dt)
    self.fcw = False
    self.dt = dt

    self.a_desired = init_a
    self.v_desired_filter = FirstOrderFilter(init_v, 2.0, self.dt)
    self.v_model_error = 0.0

    self.lead_one = Lead()
    self.lead_two = Lead()

    self.v_desired_trajectory = np.zeros(CONTROL_N)
    self.a_desired_trajectory = np.zeros(CONTROL_N)
    self.j_desired_trajectory = np.zeros(CONTROL_N)
    self.solverExecutionTime = 0.0

  @staticmethod
  def parse_model(model_msg, model_error, v_ego, taco_tune):
    if (len(model_msg.position.x) == ModelConstants.IDX_N and
       len(model_msg.velocity.x) == ModelConstants.IDX_N and
       len(model_msg.acceleration.x) == ModelConstants.IDX_N):
      x = np.interp(T_IDXS_MPC, ModelConstants.T_IDXS, model_msg.position.x) - model_error * T_IDXS_MPC
      v = np.interp(T_IDXS_MPC, ModelConstants.T_IDXS, model_msg.velocity.x) - model_error
      a = np.interp(T_IDXS_MPC, ModelConstants.T_IDXS, model_msg.acceleration.x)
      j = np.zeros(len(T_IDXS_MPC))
    else:
      x = np.zeros(len(T_IDXS_MPC))
      v = np.zeros(len(T_IDXS_MPC))
      a = np.zeros(len(T_IDXS_MPC))
      j = np.zeros(len(T_IDXS_MPC))

    if taco_tune:
      max_lat_accel = interp(v_ego, [5, 10, 20], [1.5, 2.0, 3.0])
      curvatures = np.interp(T_IDXS_MPC, ModelConstants.T_IDXS, model_msg.orientationRate.z) / np.clip(v, 0.3, 100.0)
      max_v = np.sqrt(max_lat_accel / (np.abs(curvatures) + 1e-3)) - 2.0
      v = np.minimum(max_v, v)

    return x, v, a, j

  def update(self, clairvoyant_model, e2e_longitudinal_model, sm, frogpilot_toggles):
    self.mpc.mode = 'blended' if sm['controlsState'].experimentalMode and not clairvoyant_model else 'acc'

    v_ego = sm['carState'].vEgo
    v_cruise_kph = min(sm['controlsState'].vCruise, V_CRUISE_MAX)
    v_cruise = v_cruise_kph * CV.KPH_TO_MS

    long_control_off = sm['controlsState'].longControlState == LongCtrlState.off
    force_slow_decel = sm['controlsState'].forceDecel

    # Reset current state when not engaged, or user is controlling the speed
    reset_state = long_control_off if self.CP.openpilotLongitudinalControl else not sm['controlsState'].enabled

    # No change cost when user is controlling the speed, or when standstill
    prev_accel_constraint = not (reset_state or sm['carState'].standstill)

    accel_limits = [sm['frogpilotPlan'].minAcceleration, sm['frogpilotPlan'].maxAcceleration]
    if self.mpc.mode == 'acc':
      accel_limits_turns = limit_accel_in_turns(v_ego, sm['carState'].steeringAngleDeg, accel_limits, self.CP)
    else:
      accel_limits_turns = [ACCEL_MIN, ACCEL_MAX]

    if reset_state:
      self.v_desired_filter.x = v_ego
      # Clip aEgo to cruise limits to prevent large accelerations when becoming active
      self.a_desired = clip(sm['carState'].aEgo, accel_limits[0], accel_limits[1])

    # Prevent divergence, smooth in current v_ego
    self.v_desired_filter.x = max(0.0, self.v_desired_filter.update(v_ego))
    # Compute model v_ego error
    self.v_model_error = 0.0 if e2e_longitudinal_model else get_speed_error(sm['modelV2'], v_ego)

    if force_slow_decel:
      v_cruise = 0.0
    # clip limits, cannot init MPC outside of bounds
    accel_limits_turns[0] = min(accel_limits_turns[0], self.a_desired + 0.05)
    accel_limits_turns[1] = max(accel_limits_turns[1], self.a_desired - 0.05)

    if frogpilot_toggles.radarless_model:
      model_leads = list(sm['modelV2'].leadsV3)
      # TODO lead state should be invalidated if its different point than the previous one
      lead_states = [self.lead_one, self.lead_two]
      for index in range(len(lead_states)):
        if len(model_leads) > index:
          model_lead = model_leads[index]
          lead_states[index].update(model_lead.x[0], model_lead.y[0], model_lead.v[0], model_lead.a[0], model_lead.prob)
        else:
          lead_states[index].reset()
    else:
      self.lead_one = sm['radarState'].leadOne
      self.lead_two = sm['radarState'].leadTwo

    self.mpc.set_weights(sm['frogpilotPlan'].accelerationJerk, sm['frogpilotPlan'].dangerJerk, sm['frogpilotPlan'].speedJerk, prev_accel_constraint, personality=sm['controlsState'].personality)
    self.mpc.set_accel_limits(accel_limits_turns[0], accel_limits_turns[1])
    self.mpc.set_cur_state(self.v_desired_filter.x, self.a_desired)
    x, v, a, j = self.parse_model(sm['modelV2'], self.v_model_error, v_ego, frogpilot_toggles.taco_tune)
    self.mpc.update(self.lead_one, self.lead_two, sm['frogpilotPlan'].vCruise, x, v, a, j, sm['frogpilotPlan'].tFollow,
                    sm['frogpilotCarState'].trafficModeActive, frogpilot_toggles, personality=sm['controlsState'].personality)

    self.a_desired_trajectory_full = np.interp(CONTROL_N_T_IDX, T_IDXS_MPC, self.mpc.a_solution)
    self.v_desired_trajectory = np.interp(CONTROL_N_T_IDX, T_IDXS_MPC, self.mpc.v_solution)
    self.a_desired_trajectory = np.interp(CONTROL_N_T_IDX, T_IDXS_MPC, self.mpc.a_solution)
    self.j_desired_trajectory = np.interp(CONTROL_N_T_IDX, T_IDXS_MPC[:-1], self.mpc.j_solution)

    # TODO counter is only needed because radar is glitchy, remove once radar is gone
    self.fcw = self.mpc.crash_cnt > 2 and not sm['carState'].standstill
    if self.fcw:
      cloudlog.info("FCW triggered")

    # Interpolate 0.05 seconds and save as starting point for next iteration
    a_prev = self.a_desired
    self.a_desired = float(interp(self.dt, CONTROL_N_T_IDX, self.a_desired_trajectory))
    self.v_desired_filter.x = self.v_desired_filter.x + self.dt * (self.a_desired + a_prev) / 2.0

  def publish(self, e2e_longitudinal_model, sm, pm):
    plan_send = messaging.new_message('longitudinalPlan')

    plan_send.valid = sm.all_checks(service_list=['carState', 'controlsState'])


    longitudinalPlan = plan_send.longitudinalPlan
    longitudinalPlan.modelMonoTime = sm.logMonoTime['modelV2']
    longitudinalPlan.processingDelay = (plan_send.logMonoTime / 1e9) - sm.logMonoTime['modelV2']
    longitudinalPlan.solverExecutionTime = self.mpc.solve_time

    longitudinalPlan.allowBrake = True
    longitudinalPlan.allowThrottle = True

    longitudinalPlan.speeds = self.v_desired_trajectory.tolist()
    longitudinalPlan.accels = self.a_desired_trajectory.tolist()
    longitudinalPlan.jerks = self.j_desired_trajectory.tolist()

    longitudinalPlan.hasLead = self.lead_one.status
    longitudinalPlan.longitudinalPlanSource = self.mpc.source
    longitudinalPlan.fcw = self.fcw

    a_target_mpc, should_stop_mpc = get_accel_from_plan(self.CP, longitudinalPlan.speeds, longitudinalPlan.accels)

    if e2e_longitudinal_model and sm['controlsState'].experimentalMode:
      model_speeds = np.interp(CONTROL_N_T_IDX, ModelConstants.T_IDXS, sm['modelV2'].velocity.x)
      model_accels = np.interp(CONTROL_N_T_IDX, ModelConstants.T_IDXS, sm['modelV2'].acceleration.x)
      a_target_model, should_stop_model = get_accel_from_plan(self.CP, model_speeds, model_accels)
      a_target = min(a_target_mpc, a_target_model)
      should_stop = should_stop_mpc or should_stop_model
    else:
      a_target = a_target_mpc
      should_stop = should_stop_mpc

    longitudinalPlan.aTarget = float(a_target)
    longitudinalPlan.shouldStop = bool(should_stop)

    longitudinalPlan.allowBrake = True
    longitudinalPlan.allowThrottle = True

    pm.send('longitudinalPlan', plan_send)
