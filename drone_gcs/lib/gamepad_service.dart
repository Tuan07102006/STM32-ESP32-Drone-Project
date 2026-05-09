import 'dart:async';
import 'package:flutter/foundation.dart';
import 'package:win32_gamepad/win32_gamepad.dart';

class GamepadService {
  Timer? _pollingTimer;
  Gamepad? _gamepad;
  
  double roll = 0.0;
  double pitch = 0.0;
  double yaw = 0.0;
  double throttle = 0.0;

  // Biến lưu trạng thái của vòng lặp ngay trước đó
  double _lastRoll = 0.0;
  double _lastPitch = 0.0;
  double _lastYaw = 0.0;
  double _lastThrottle = 0.0;

  Map<String, double> keyStates = {};

  Function(double roll, double pitch, double yaw, double throttle)? onControlUpdated;
  Function()? onArmPressed;
  Function()? onDisarmPressed;
  Function(Map<String, double>)? onKeyStatesChanged; 

  bool _wasArmPressed = false;
  bool _wasDisarmPressed = false;

  Future<void> init() async {
    try {
      _gamepad = Gamepad(0);
      if (_gamepad!.isConnected) {
        debugPrint(" Đã kết nối tay cầm XInput (Win32)");
      } else {
        debugPrint(" Chưa tìm thấy tay cầm. Hãy cắm tay cầm vào và thử lại.");
      }
      _pollingTimer = Timer.periodic(const Duration(milliseconds: 50), _pollGamepad);
    } catch (e) {
      debugPrint("❌ Lỗi khởi tạo Gamepad: $e");
    }
  }

  void _pollGamepad(Timer timer) {
    if (_gamepad == null) return;

    _gamepad!.updateState();
    if (!_gamepad!.isConnected) return;

    final state = _gamepad!.state;

    // --- 1. CHUẨN HÓA TRỤC (Analog & Triggers) & LỌC NHIỄU ---
    double stickLeftX = state.leftThumbstickX / 32767.0;
    double stickLeftY = state.leftThumbstickY / 32767.0; 
    double stickRightX = state.rightThumbstickX / 32767.0;
    double stickRightY = state.rightThumbstickY / 32767.0;

    double ltValue = state.leftTrigger / 255.0;
    double rtValue = state.rightTrigger / 255.0;

    if (stickLeftX.abs() < 0.1) stickLeftX = 0;
    if (stickLeftY.abs() < 0.1) stickLeftY = 0;
    if (stickRightX.abs() < 0.1) stickRightX = 0;
    if (stickRightY.abs() < 0.1) stickRightY = 0;

    // --- 2. XỬ LÝ PITCH & ROLL (TỰ ĐỘNG TRẢ VỀ CÂN BẰNG) ---
    roll = stickLeftX * 100;
    pitch = stickLeftY * -100; 

    // --- 3. XỬ LÝ YAW (HƯỚNG ĐẦU DRONE - CỘNG DỒN) ---
    if (stickRightX != 0) {
      yaw += stickRightX * 2.0; 
      if (yaw > 180.0) yaw = 180.0;
      if (yaw < -180.0) yaw = -180.0;
    }

    // --- 4. XỬ LÝ MỨC GA (THROTTLE - CỘNG DỒN) ---
    if (rtValue > 0.1 && throttle < 100.0) {
      throttle += 1.5; 
      if (throttle > 100.0) throttle = 100.0;
    }
    if (ltValue > 0.1 && throttle > 0.0) {
      throttle -= 1.5; 
      if (throttle < 0.0) throttle = 0.0;
    }

    // --- 5. LỆNH GỬI LIÊN TỤC (TRÁNH FAILSAFE) ---
    // Đã xóa lệnh `if` kiểm tra sự thay đổi để đảm bảo luôn gửi dữ liệu
    if (onControlUpdated != null) {
      onControlUpdated!(roll, pitch, yaw, throttle);
    }
    
    _lastRoll = roll;
    _lastPitch = pitch;
    _lastYaw = yaw;
    _lastThrottle = throttle;

    // --- 6. XỬ LÝ NÚT BẤM (ARM / DISARM) ---
    bool isArmPressed = state.buttonA; 
    bool isDisarmPressed = state.buttonB; 

    if (isArmPressed && !_wasArmPressed) {
      if (onArmPressed != null) onArmPressed!();
    }
    if (isDisarmPressed && !_wasDisarmPressed) {
      if (onDisarmPressed != null) onDisarmPressed!();
    }

    _wasArmPressed = isArmPressed;
    _wasDisarmPressed = isDisarmPressed;

    // --- 7. CẬP NHẬT TRẠNG THÁI HIỂN THỊ ---
    keyStates["a_0"] = stickLeftX;
    keyStates["a_1"] = stickLeftY * -1;
    keyStates["a_2"] = stickRightX;
    keyStates["a_3"] = stickRightY * -1;
    keyStates["a_4"] = ltValue;
    keyStates["a_5"] = rtValue;

    keyStates["b_0"] = state.buttonA ? 1.0 : 0.0; 
    keyStates["b_1"] = state.buttonB ? 1.0 : 0.0; 
    keyStates["b_2"] = state.buttonX ? 1.0 : 0.0; 
    keyStates["b_3"] = state.buttonY ? 1.0 : 0.0; 
    keyStates["b_4"] = state.leftShoulder ? 1.0 : 0.0; 
    keyStates["b_5"] = state.rightShoulder ? 1.0 : 0.0; 
    keyStates["b_8"] = state.buttonBack ? 1.0 : 0.0; 
    keyStates["b_9"] = state.buttonStart ? 1.0 : 0.0; 
    
    keyStates["b_12"] = state.dpadUp ? 1.0 : 0.0;
    keyStates["b_13"] = state.dpadDown ? 1.0 : 0.0;
    keyStates["b_14"] = state.dpadLeft ? 1.0 : 0.0;
    keyStates["b_15"] = state.dpadRight ? 1.0 : 0.0;

    if (onKeyStatesChanged != null) {
      onKeyStatesChanged!(keyStates);
    }
  }

  void dispose() {
    _pollingTimer?.cancel();
    debugPrint("🛑 Đã ngắt kết nối Gamepad Service.");
  }
}