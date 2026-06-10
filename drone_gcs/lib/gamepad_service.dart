import 'dart:async';
import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart'; 
import 'package:win32_gamepad/win32_gamepad.dart';

class GamepadService {
  static final GamepadService _instance = GamepadService._internal();
  factory GamepadService() => _instance;
  GamepadService._internal();

  bool ignoreInput = false;
  Timer? _pollingTimer;
  Gamepad? _gamepad;
  
  double roll = 0.0;
  double pitch = 0.0;
  double yaw = 0.0;
  double throttle = 0.0;

  double _lastRoll = 0.0;
  double _lastPitch = 0.0;

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
        debugPrint("🎮 Đã kết nối tay cầm XInput (Win32)");
      } else {
        debugPrint("⚠️ Chưa tìm thấy tay cầm. Tự động chuyển sang dùng Bàn phím!");
      }
      
      // Đổi tên hàm thành _pollInputs để phản ánh việc đọc cả 2 thiết bị
      _pollingTimer = Timer.periodic(const Duration(milliseconds: 50), _pollInputs);
    } catch (e) {
      debugPrint("❌ Lỗi khởi tạo Gamepad: $e");
      // Vẫn chạy vòng lặp để bàn phím có thể hoạt động dù tay cầm bị lỗi
      _pollingTimer = Timer.periodic(const Duration(milliseconds: 50), _pollInputs);
    }
  }

  void _pollInputs(Timer timer) {
    // 1. ĐỌC TÍN HIỆU GAMEPAD (Nếu có kết nối)
    double stickLeftX = 0.0, stickLeftY = 0.0;
    double stickRightX = 0.0, stickRightY = 0.0;
    double ltValue = 0.0, rtValue = 0.0;
    bool isArmBtn = false, isDisarmBtn = false;

    if (!ignoreInput) {
      // 2. ĐỌC TÍN HIỆU GAMEPAD 
      if (_gamepad != null) {
        _gamepad!.updateState();
        if (_gamepad!.isConnected) {
          final state = _gamepad!.state;
          stickLeftX = state.leftThumbstickX / 32767.0;
          stickLeftY = state.leftThumbstickY / 32767.0; 
          stickRightX = state.rightThumbstickX / 32767.0;
          stickRightY = state.rightThumbstickY / 32767.0;
          ltValue = state.leftTrigger / 255.0;
          rtValue = state.rightTrigger / 255.0;

          if (stickLeftX.abs() < 0.1) stickLeftX = 0;
          if (stickLeftY.abs() < 0.1) stickLeftY = 0;
          if (stickRightX.abs() < 0.1) stickRightX = 0;
          if (stickRightY.abs() < 0.1) stickRightY = 0;

          isArmBtn = state.buttonA;
          isDisarmBtn = state.buttonB;
        }
      }

      // 3. ĐỌC TÍN HIỆU BÀN PHÍM 
      final keys = HardwareKeyboard.instance.logicalKeysPressed;
      if (keys.contains(LogicalKeyboardKey.keyW)) stickLeftY = 1.0; 
      if (keys.contains(LogicalKeyboardKey.keyS)) stickLeftY = -1.0; 
      if (keys.contains(LogicalKeyboardKey.keyA)) stickLeftX = -1.0; 
      if (keys.contains(LogicalKeyboardKey.keyD)) stickLeftX = 1.0;  
      if (keys.contains(LogicalKeyboardKey.arrowLeft)) stickRightX = -1.0;
      if (keys.contains(LogicalKeyboardKey.arrowRight)) stickRightX = 1.0;
      if (keys.contains(LogicalKeyboardKey.arrowUp)) rtValue = 1.0;   
      if (keys.contains(LogicalKeyboardKey.arrowDown)) ltValue = 1.0; 
      if (keys.contains(LogicalKeyboardKey.enter)) isArmBtn = true;
      if (keys.contains(LogicalKeyboardKey.escape) || keys.contains(LogicalKeyboardKey.backspace)) isDisarmBtn = true;
    }
    // 3. XỬ LÝ PITCH & ROLL VỚI SLEW RATE LIMITER 
    double targetRoll = stickLeftX * 50.0;
    double targetPitch = stickLeftY * -50.0; 

    const double maxStep = 5.0; // Tốc độ đáp ứng (độ trễ gạt cần)

    if (targetRoll > _lastRoll + maxStep) {
      roll = _lastRoll + maxStep;
    } else if (targetRoll < _lastRoll - maxStep) {
      roll = _lastRoll - maxStep;
    } else {
      roll = targetRoll;
    }

    if (targetPitch > _lastPitch + maxStep) {
      pitch = _lastPitch + maxStep;
    } else if (targetPitch < _lastPitch - maxStep) {
      pitch = _lastPitch - maxStep;
    } else {
      pitch = targetPitch;
    }

    // 4. XỬ LÝ YAW (Hướng đầu drone - CỘNG DỒN)
    if (stickRightX != 0) {
      yaw += stickRightX * 3.0; 
      if (yaw > 180.0) yaw -= 360.0;
      else if (yaw <= -180.0) yaw += 360.0;
    }

    // 5. XỬ LÝ MỨC GA (THROTTLE - CỘNG DỒN)
    if (rtValue > 0.1 && throttle < 100.0) {
      throttle += 1.5; 
      if (throttle > 100.0) throttle = 100.0;
    }
    if (ltValue > 0.1 && throttle > 0.0) {
      throttle -= 1.5; 
      if (throttle < 0.0) throttle = 0.0;
    }

    // 6. GỬI LỆNH ĐIỀU KHIỂN XUỐNG BỘ LỌC ĐỂ BẮN UDP 
    if (onControlUpdated != null) {
      onControlUpdated!(roll, pitch, yaw, throttle);
    }
    
    _lastRoll = roll;
    _lastPitch = pitch;

    // 7. XỬ LÝ NÚT BẤM (ARM / DISARM) 
    if (isArmBtn && !_wasArmPressed) {
      if (onArmPressed != null) onArmPressed!();
    }
    if (isDisarmBtn && !_wasDisarmPressed) {
      if (onDisarmPressed != null) onDisarmPressed!();
    }

    _wasArmPressed = isArmBtn;
    _wasDisarmPressed = isDisarmBtn;

    // 8. CẬP NHẬT GIAO DIỆN HÌNH TAY CẦM TRÊN MÀN HÌNH 
    keyStates["a_0"] = stickLeftX;
    keyStates["a_1"] = stickLeftY * -1;
    keyStates["a_2"] = stickRightX;
    keyStates["a_3"] = stickRightY * -1;
    keyStates["a_4"] = ltValue;
    keyStates["a_5"] = rtValue;
    keyStates["b_0"] = isArmBtn ? 1.0 : 0.0; 
    keyStates["b_1"] = isDisarmBtn ? 1.0 : 0.0; 

    if (onKeyStatesChanged != null) {
      onKeyStatesChanged!(keyStates);
    }
  }

  void dispose() {
    _pollingTimer?.cancel();
    debugPrint("🛑 Đã ngắt kết nối Input Service.");
  }
}