import 'dart:async';
import 'package:flutter/foundation.dart';
import 'package:gamepads/gamepads.dart';

class GamepadService {
  StreamSubscription<GamepadEvent>? _subscription;
  
  Function(String key, double value)? onAnalogMoved;
  Function(String key)? onButtonPressed;

  Future<void> init() async {
    try {
      final gamepads = await Gamepads.list();
      if (gamepads.isNotEmpty) {
        debugPrint("🎮 Đã tìm thấy tay cầm: ${gamepads.first.name}");
      } else {
        debugPrint("⚠️ Chưa tìm thấy tay cầm nào. Vui lòng kết nối tay cầm.");
      }

      _subscription = Gamepads.events.listen((GamepadEvent event) {
        if (event.type == KeyType.analog) {
          if (onAnalogMoved != null) {
            onAnalogMoved!(event.key, event.value);
          }
        } 
        else if (event.type == KeyType.button) {
          if (onButtonPressed != null && event.value == 1.0) {
            onButtonPressed!(event.key);
          }
        }
      });
    } catch (e) {
      debugPrint("❌ Lỗi khởi tạo Gamepad: $e");
    }
  }

  void dispose() {
    _subscription?.cancel();
    debugPrint("🛑 Đã ngắt kết nối Gamepad Service.");
  }
}