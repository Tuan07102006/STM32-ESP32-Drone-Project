import 'package:flutter/material.dart';
import 'shared_components.dart';

class TelemetryPanel extends StatelessWidget {
  final Map<String, dynamic> telemetryData;

  const TelemetryPanel({super.key, required this.telemetryData});

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        buildSectionHeader("THÔNG SỐ HỆ THỐNG", Icons.memory),
        Expanded(
          child: GridView.count(
            crossAxisCount: 2,
            childAspectRatio: 2.5,
            mainAxisSpacing: 6,
            crossAxisSpacing: 6,
            children: [
              buildCompactCard("Roll", telemetryData["Roll"]?.toStringAsFixed(2) ?? "0.0", "°", Colors.indigo),
              buildCompactCard("Pitch", telemetryData["Pitch"]?.toStringAsFixed(2) ?? "0.0", "°", Colors.deepPurple),
              buildCompactCard("Yaw", telemetryData["Yaw"]?.toStringAsFixed(2) ?? "0.0", "°", Colors.pink),
              buildCompactCard("Độ cao (Khí áp)", telemetryData["Do_cao"]?.toStringAsFixed(2) ?? "0.0", "m", Colors.blue),
              buildCompactCard("Điện áp", telemetryData["Dien_ap"]?.toStringAsFixed(2) ?? "0.0", "V", Colors.green),
              buildCompactCard("Dòng điện", telemetryData["Dong_dien"]?.toStringAsFixed(2) ?? "0.0", "A", Colors.red),
              
              buildCompactCard("Vĩ độ (Lat)", telemetryData["gps_lat"]?.toStringAsFixed(6) ?? "0.0", "", Colors.teal),
              buildCompactCard("Kinh độ (Lng)", telemetryData["gps_lng"]?.toStringAsFixed(6) ?? "0.0", "", Colors.teal),
              buildCompactCard("Vệ tinh", telemetryData["gps_sat"]?.toString() ?? "0", "sat", Colors.amber),
              buildCompactCard("Độ chính xác (HDOP)", telemetryData["gps_hdop"]?.toStringAsFixed(1) ?? "0.0", "", Colors.purple),
              buildCompactCard("Vận tốc GPS", telemetryData["gps_speed"]?.toStringAsFixed(2) ?? "0.0", "m/s", Colors.cyan),
              buildCompactCard("Độ cao GPS", telemetryData["gps_alt"]?.toStringAsFixed(2) ?? "0.0", "m", Colors.brown),
              buildCompactCard("Hướng di chuyển", telemetryData["gps_course"]?.toStringAsFixed(1) ?? "0.0", "°", Colors.lime),
            ],
          ),
        ),
      ],
    );
  }
}