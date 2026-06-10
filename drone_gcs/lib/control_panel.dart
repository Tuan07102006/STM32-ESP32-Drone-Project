import 'package:flutter/material.dart';
import 'shared_components.dart';
import 'gamepad_visualizer.dart';
import 'gamepad_service.dart';

class ControlPanel extends StatelessWidget {
  final ValueNotifier<Map<String, double>> gamepadStateNotifier;
  final double currentThrottle;
  final double currentRoll;
  final double currentPitch;
  final double currentYaw;
  final bool isRecording;
  final Map<String, dynamic> telemetryData;
  
  // Callbacks truyền từ DashboardScreen xuống
  final VoidCallback onToggleRecord;
  final VoidCallback onArm;
  final VoidCallback onDisarm;
  final Function(String command) onSendPidCommand;

  const ControlPanel({
    super.key,
    required this.gamepadStateNotifier,
    required this.currentThrottle,
    required this.currentRoll,
    required this.currentPitch,
    required this.currentYaw,
    required this.isRecording,
    required this.telemetryData,
    required this.onToggleRecord,
    required this.onArm,
    required this.onDisarm,
    required this.onSendPidCommand,
  });

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        buildSectionHeader("ĐIỀU KHIỂN & LỆNH", Icons.gamepad),
        Expanded(
          child: SingleChildScrollView(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: [
                GamepadVisualizer(stateNotifier: gamepadStateNotifier),
                const SizedBox(height: 8),
                _buildGamepadLegend(),
                const SizedBox(height: 12),
                _buildCommandedValuesPanel(),
                const SizedBox(height: 12),

                // Nút Ghi Log
                Card(
                  color: isRecording ? Colors.red.shade900 : Colors.blueGrey.shade800,
                  margin: EdgeInsets.zero,
                  child: Padding(
                    padding: const EdgeInsets.all(12.0),
                    child: Column(
                      children: [
                        Icon(isRecording ? Icons.fiber_manual_record : Icons.save_alt, 
                             color: isRecording ? Colors.redAccent : Colors.white, size: 24),
                        const SizedBox(height: 8),
                        ElevatedButton(
                          style: ElevatedButton.styleFrom(
                            backgroundColor: isRecording ? Colors.black54 : Colors.red,
                            minimumSize: const Size.fromHeight(40),
                          ),
                          onPressed: onToggleRecord,
                          child: Text(isRecording ? "DỪNG GHI LOG" : "GHI NHẬT KÝ BAY", style: const TextStyle(fontSize: 12)),
                        ),
                      ],
                    ),
                  ),
                ),
                const SizedBox(height: 12),

                // Các nút điều khiển bay
                ElevatedButton.icon(
                  onPressed: onArm,
                  icon: const Icon(Icons.flight_takeoff),
                  label: const Text("ARM", style: TextStyle(fontSize: 14, fontWeight: FontWeight.bold)),
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Colors.redAccent,
                    foregroundColor: Colors.white,
                    padding: const EdgeInsets.symmetric(vertical: 14),
                  ),
                ),
                const SizedBox(height: 10),
                ElevatedButton.icon(
                  onPressed: onDisarm,
                  icon: const Icon(Icons.flight_land),
                  label: const Text("DISARM", style: TextStyle(fontSize: 14, fontWeight: FontWeight.bold)),
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Colors.orange,
                    foregroundColor: Colors.white,
                    padding: const EdgeInsets.symmetric(vertical: 14),
                  ),
                ),
                const SizedBox(height: 10),
                ElevatedButton.icon(
                  onPressed: () => _showPidTuningDialog(context),
                  icon: const Icon(Icons.tune),
                  label: const Text("TÙY CHỈNH PID", style: TextStyle(fontSize: 13)),
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Colors.teal,
                    foregroundColor: Colors.white,
                    padding: const EdgeInsets.symmetric(vertical: 14),
                  ),
                ),
              ],
            ),
          ),
        ),
      ],
    );
  }

  // --- Các hàm hỗ trợ cho Control Panel ---
  Widget _buildGamepadLegend() {
    return Container(
      padding: const EdgeInsets.all(8),
      decoration: BoxDecoration(color: Colors.black26, borderRadius: BorderRadius.circular(8)),
      child: Column(
        children: [
          const Text("GÁN PHÍM CHỨC NĂNG (GAMEPAD & BÀN PHÍM)", style: TextStyle(fontSize: 10, fontWeight: FontWeight.bold, color: Colors.white54, letterSpacing: 1)),
          const SizedBox(height: 6),
          _buildLegendRow("🕹️ L-Stick | Phím W A S D", "Pitch (Tiến/Lùi) & Roll (Trái/Phải)"),
          _buildLegendRow("🕹️ R-Stick | Phím ⬅️ ➡️", "Yaw (Xoay đầu)"),
          _buildLegendRow("🎯 LT / RT  | Phím ⬇️ ⬆️", "Giảm / Tăng Ga"),
          _buildLegendRow("🟢 Nút A     | Phím Enter", "ARM (Khởi động)"),
          _buildLegendRow("🔴 Nút B     | Phím Esc", "DISARM (Ngắt)"),
        ],
      ),
    );
  }

  Widget _buildLegendRow(String key, String func) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 2.0),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(key, style: const TextStyle(fontSize: 10, color: Colors.cyan)),
          Text(func, style: const TextStyle(fontSize: 10, color: Colors.white)),
        ],
      ),
    );
  }

  Widget _buildCommandedValuesPanel() {
    return Container(
      padding: const EdgeInsets.all(10),
      decoration: BoxDecoration(
        color: Colors.blueGrey.shade800,
        borderRadius: BorderRadius.circular(8),
        border: Border.all(color: Colors.cyan.withOpacity(0.4), width: 1.5),
      ),
      child: Column(
        children: [
          const Text("TÍN HIỆU ĐANG GỬI (COMMAND)", style: TextStyle(fontSize: 10, fontWeight: FontWeight.bold, color: Colors.cyan, letterSpacing: 1)),
          const SizedBox(height: 8),
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceAround,
            children: [
              _buildCommandText("Ga (Thr)", currentThrottle, "%"),
              _buildCommandText("Roll", currentRoll, "°"),
              _buildCommandText("Pitch", currentPitch, "°"),
              _buildCommandText("Yaw", currentYaw, "°"),
            ],
          )
        ],
      ),
    );
  }

  Widget _buildCommandText(String label, double val, String unit) {
    return Column(
      children: [
        Text(label, style: const TextStyle(fontSize: 10, color: Colors.white70)),
        const SizedBox(height: 2),
        Text("${val.toStringAsFixed(1)}$unit", style: const TextStyle(fontSize: 13, fontWeight: FontWeight.bold, color: Colors.white)),
      ],
    );
  }

  void _showPidTuningDialog(BuildContext context) {
    int isArmed = telemetryData["Trang_thai_Arm"] ?? 0;
    if (isArmed == 1) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(
          content: Text("⚠️ AN TOÀN: Vui lòng DISARM (Khóa) drone trước khi cài đặt PID!"),
          backgroundColor: Colors.redAccent,
          duration: Duration(seconds: 3),
        ),
      );
      return;
    }

    // KHÓA BÀN PHÍM / GAMEPAD KHI BẮT ĐẦU MỞ BẢNG PID
    GamepadService().ignoreInput = true;

    final TextEditingController kpRollController = TextEditingController(text: telemetryData["Kp_roll_moi"]?.toString() ?? "0.0");
    final TextEditingController kdRollController = TextEditingController(text: telemetryData["Kd_roll_moi"]?.toString() ?? "0.0");
    final TextEditingController kpPitchController = TextEditingController(text: telemetryData["Kp_pitch_moi"]?.toString() ?? "0.0");
    final TextEditingController kdPitchController = TextEditingController(text: telemetryData["Kd_pitch_moi"]?.toString() ?? "0.0");
    final TextEditingController kpYawController = TextEditingController(text: telemetryData["Kp_yaw_moi"]?.toString() ?? "0.0");
    final TextEditingController kiYawController = TextEditingController(text: telemetryData["Ki_yaw_moi"]?.toString() ?? "0.0");

    showDialog(
      context: context,
      barrierDismissible: false, // Bắt buộc người dùng phải bấm nút để đóng, tránh thoát vô tình
      builder: (BuildContext context) {
        return AlertDialog(
          title: const Text("Cài đặt PID"),
          content: SingleChildScrollView(
            child: Column(
              mainAxisSize: MainAxisSize.min,
              children: [
                _buildPidInputField("Kp Roll", kpRollController),
                _buildPidInputField("Kd Roll", kdRollController),
                const SizedBox(height: 16),
                _buildPidInputField("Kp Pitch", kpPitchController),
                _buildPidInputField("Kd Pitch", kdPitchController),
                const SizedBox(height: 16),
                _buildPidInputField("Kp Yaw", kpYawController),
                _buildPidInputField("Ki Yaw", kiYawController),
              ],
            ),
          ),
          actions: [
            TextButton(
              child: const Text("Hủy"), 
              onPressed: () {
                GamepadService().ignoreInput = false; // MỞ LẠI BÀN PHÍM
                Navigator.of(context).pop();
              }
            ),
            ElevatedButton(
              child: const Text("Lưu"),
              onPressed: () {
                GamepadService().ignoreInput = false; // MỞ LẠI BÀN PHÍM

                int currentArmedState = telemetryData["Trang_thai_Arm"] ?? 0;
                if (currentArmedState == 1) {
                  Navigator.of(context).pop(); 
                  ScaffoldMessenger.of(context).showSnackBar(
                    const SnackBar(
                      content: Text("⚠️ ĐÃ HỦY LƯU: Drone vừa bị Arm, không thể đổi PID!"),
                      backgroundColor: Colors.redAccent,
                    ),
                  );
                  return;
                }

                String command = "@${kpRollController.text},${kdRollController.text},${kpPitchController.text},${kdPitchController.text},${kpYawController.text},${kiYawController.text},0.0*"; 
                onSendPidCommand(command); 
                
                ScaffoldMessenger.of(context).showSnackBar(
                  const SnackBar(
                    content: Text("✅ Đã gửi lệnh lưu PID!"),
                    backgroundColor: Colors.green,
                    duration: Duration(seconds: 2),
                  ),
                );
                Navigator.of(context).pop();
              },
            ),
          ],
        );
      },
    ).then((_) {
      // Đề phòng trường hợp lỗi HĐH hoặc user bấm Esc tắt ngang dialog
      GamepadService().ignoreInput = false; 
    });
  }

  Widget _buildPidInputField(String label, TextEditingController controller) {
    return TextField(
      controller: controller,
      keyboardType: TextInputType.number, 
      decoration: InputDecoration(labelText: label, border: const OutlineInputBorder()),
    );
  }
}