from cereal import log, messaging
from openpilot.selfdrive.boardd.boardd import can_list_to_can_capnp
from openpilot.selfdrive.car import make_can_msg
from openpilot.selfdrive.manager.process_config import managed_processes

if __name__ == "__main__":
  pm = messaging.PubMaster(['deviceState', 'pandaStates', 'sendcan'])
  device_state_msg = messaging.new_message('deviceState')
  panda_states_msg = messaging.new_message('pandaStates', 1)

  device_state_msg.deviceState.started = True
  panda_states_msg.pandaStates[0].ignitionLine = True

  can_sends = make_can_msg(0x750, b'\x40\x05\x30\x11\x00\x80\x00\x00', 0)
  pm.send('sendcan', can_list_to_can_capnp(can_sends, msgtype='sendcan', valid=True))
