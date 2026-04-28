import 'package:flutter/material.dart';

void main() {
  runApp(const DroneDashboardApp());
}

class DroneDashboardApp extends StatelessWidget {
  const DroneDashboardApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      title: 'Drone Control Station',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.blueGrey),
        useMaterial3: true,
      ),
      home: const DashboardScreen(),
    );
  }
}

class DashboardScreen extends StatefulWidget {
  const DashboardScreen({super.key});

  @override
  State<DashboardScreen> createState() => _DashboardScreenState();
}

class _DashboardScreenState extends State<DashboardScreen> {
  // Biến lưu trữ dữ liệu giả lập (Sau này sẽ nhận thực tế từ vi điều khiển)
  String batteryVoltage = "11.8 V"; // Đo qua INA219 từ pin 3S
  String altitude = "1.2 m";        // Đo từ cảm biến VL53L0X
  String gpsStatus = "3D Fix";      // Trạng thái từ NEO-8M

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Bảng Giám Sát Bay', style: TextStyle(fontWeight: FontWeight.bold)),
        backgroundColor: Colors.blueGrey,
        foregroundColor: Colors.white,
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            _buildInfoCard("Điện áp Pin (INA219)", batteryVoltage, Icons.battery_charging_full, Colors.green),
            const SizedBox(height: 16),
            _buildInfoCard("Độ cao (VL53L0X)", altitude, Icons.height, Colors.blue),
            const SizedBox(height: 16),
            _buildInfoCard("Định vị (NEO-8M)", gpsStatus, Icons.satellite_alt, Colors.orange),
            
            const Spacer(),
            
            // Nút bấm ví dụ để gửi lệnh
            ElevatedButton.icon(
              onPressed: () {
                // Logic gửi lệnh Arm động cơ sẽ viết ở đây
                print("Đã gửi lệnh ARM động cơ!");
              },
              icon: const Icon(Icons.flight_takeoff),
              label: const Padding(
                padding: EdgeInsets.all(12.0),
                child: Text("ARM ĐỘNG CƠ", style: TextStyle(fontSize: 18)),
              ),
              style: ElevatedButton.styleFrom(
                backgroundColor: Colors.redAccent,
                foregroundColor: Colors.white,
              ),
            )
          ],
        ),
      ),
    );
  }

  // Hàm tiện ích để vẽ các thẻ thông tin cho gọn code
  Widget _buildInfoCard(String title, String value, IconData icon, Color iconColor) {
    return Card(
      elevation: 4,
      child: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Row(
          children: [
            Icon(icon, size: 40, color: iconColor),
            const SizedBox(width: 16),
            Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(title, style: const TextStyle(fontSize: 16, color: Colors.grey)),
                Text(value, style: const TextStyle(fontSize: 24, fontWeight: FontWeight.bold)),
              ],
            )
          ],
        ),
      ),
    );
  }
}