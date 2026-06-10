import 'package:flutter/material.dart';
import 'package:drone_gcs/telemetry_widgets.dart'; // Đảm bảo import đúng đường dẫn package của bạn
import 'shared_components.dart';

class ChartsPanel extends StatelessWidget {
  final Map<String, dynamic> telemetryData;
  final double currentRoll;
  final double currentPitch;
  final double currentYaw;

  const ChartsPanel({
    super.key,
    required this.telemetryData,
    required this.currentRoll,
    required this.currentPitch,
    required this.currentYaw,
  });

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        buildSectionHeader("BIỂU ĐỒ TRẠNG THÁI", Icons.show_chart),
        Expanded(
          child: Column(
            children: [
              Expanded(
                flex: 2,
                child: RealtimeLineChart(
                  title: "Biểu đồ ROLL (°)",
                  actualValue: telemetryData["Roll"] ?? 0.0,
                  targetValue: currentRoll,
                  actualColor: Colors.indigoAccent,
                ),
              ),
              const SizedBox(height: 8),
              Expanded(
                flex: 2,
                child: RealtimeLineChart(
                  title: "Biểu đồ PITCH (°)",
                  actualValue: telemetryData["Pitch"] ?? 0.0,
                  targetValue: currentPitch,
                  actualColor: Colors.deepPurpleAccent,
                ),
              ),
              const SizedBox(height: 8),
              Expanded(
                flex: 3,
                child: Container(
                  padding: const EdgeInsets.all(12),
                  decoration: BoxDecoration(
                    color: Colors.blueGrey.shade800,
                    borderRadius: BorderRadius.circular(8),
                  ),
                  child: CompassWidget(
                    yaw: telemetryData["Yaw"] ?? 0.0,
                    targetYaw: currentYaw,
                  ),
                ),
              ),
            ],
          ),
        ),
      ],
    );
  }
}