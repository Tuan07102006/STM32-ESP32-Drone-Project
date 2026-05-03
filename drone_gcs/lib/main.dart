import 'package:flutter/material.dart';
import 'package:drone_gcs/udp_service.dart';
import 'package:drone_gcs/gamepad_service.dart'; 
import 'package:drone_gcs/data_logger_service.dart'; 

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
  // KHAI BÁO CÁC SERVICE CHUẨN Ở ĐÂY
  final UDPService _udpService = UDPService();
  final GamepadService _gamepadService = GamepadService(); 
  final DataLoggerService _loggerService = DataLoggerService(); 
  
  Map<String, dynamic> _telemetryData = {};
  bool _isRecording = false; 

  double currentRoll = 0.0;
  double currentPitch = 0.0;
  double currentYaw = 0.0;
  double currentThrottle = 0.0;

  @override
  void initState() {
    super.initState();
    // 1. Khởi động mạng UDP
    _udpService.startListening();
    _udpService.telemetryStream.listen((data) {
      setState(() {
        _telemetryData = data;
      });
      // Đẩy dữ liệu vào hộp đen
      _loggerService.logData(data);
    });

    // 2. Khởi động tay cầm
    _initGamepadLogic();
  }

  void _initGamepadLogic() async {
    await _gamepadService.init();

    _gamepadService.onAnalogMoved = (key, value) {
      bool valueChanged = false;

      if (key == "0") { 
        currentYaw = value * 100; 
        valueChanged = true;
      } else if (key == "1") { 
        currentThrottle = value * -100; 
        valueChanged = true;
      } else if (key == "2") { 
        currentRoll = value * 100;
        valueChanged = true;
      } else if (key == "3") { 
        currentPitch = value * -100;
        valueChanged = true;
      }

      if (valueChanged) {
         _sendControlCommand();
      }
    };

    _gamepadService.onButtonPressed = (key) {
       if (key == "0") {
         _udpService.sendCommand("#0,0,0,0,1*");
         debugPrint("Đã gửi lệnh ARM động cơ (từ Tay cầm)!");
       } else if (key == "1") {
         _udpService.sendCommand("#0,0,0,0,0*");
         debugPrint("Đã gửi lệnh DISARM động cơ (từ Tay cầm)!");
       }
    };
  }

  void _sendControlCommand() {
     String command = "#${currentRoll.toStringAsFixed(1)},${currentPitch.toStringAsFixed(1)},${currentYaw.toStringAsFixed(1)},${currentThrottle.toStringAsFixed(1)}*";
     _udpService.sendCommand(command);
  }

  @override
  void dispose() {
    _udpService.dispose();
    _gamepadService.dispose(); 
    _loggerService.stopLogging(); 
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Bảng Giám Sát Bay', style: TextStyle(fontWeight: FontWeight.bold)),
        backgroundColor: Colors.blueGrey,
        foregroundColor: Colors.white,
      ),
      body: ListView(
        padding: const EdgeInsets.all(16.0),
        children: [
          Card(
            color: _isRecording ? Colors.red.shade50 : Colors.white,
            elevation: 4,
            child: ListTile(
              leading: Icon(
                _isRecording ? Icons.fiber_manual_record : Icons.save_alt,
                color: _isRecording ? Colors.red : Colors.blueGrey,
                size: 40,
              ),
              title: Text(
                _isRecording ? "Đang ghi dữ liệu chuyến bay..." : "Nhật ký bay (Blackbox)",
                style: TextStyle(
                    fontWeight: FontWeight.bold,
                    color: _isRecording ? Colors.red : Colors.black),
              ),
              subtitle: Text(_isRecording 
                  ? "Dữ liệu đang được đồng bộ hóa liên tục." 
                  : "Bấm để chọn nơi lưu và bắt đầu ghi."),
              trailing: ElevatedButton(
                style: ElevatedButton.styleFrom(
                  backgroundColor: _isRecording ? Colors.grey : Colors.red,
                  foregroundColor: Colors.white,
                ),
                onPressed: () async {
                  if (_isRecording) {
                    await _loggerService.stopLogging();
                    setState(() { _isRecording = false; });
                  } else {
                    bool started = await _loggerService.startLogging();
                    if (started) {
                      setState(() { _isRecording = true; });
                    }
                  }
                },
                child: Text(_isRecording ? "DỪNG GHI" : "BẮT ĐẦU GHI"),
              ),
            ),
          ),
          const SizedBox(height: 16),

          _buildInfoCard("Roll", _telemetryData["Roll"]?.toStringAsFixed(2) ?? "N/A", Icons.rotate_right, Colors.indigo),
          const SizedBox(height: 16),
          _buildInfoCard("Pitch", _telemetryData["Pitch"]?.toStringAsFixed(2) ?? "N/A", Icons.rotate_left, Colors.deepPurple),
          const SizedBox(height: 16),
          _buildInfoCard("Yaw", _telemetryData["Yaw"]?.toStringAsFixed(2) ?? "N/A", Icons.compass_calibration, Colors.pink),
          const SizedBox(height: 16),
          _buildInfoCard("Điện áp Pin (V)", _telemetryData["Dien_ap"]?.toStringAsFixed(2) ?? "N/A", Icons.battery_charging_full, Colors.green),
          const SizedBox(height: 16),
          _buildInfoCard("Dòng điện (A)", _telemetryData["Dong_dien"]?.toStringAsFixed(2) ?? "N/A", Icons.flash_on, Colors.red),
          const SizedBox(height: 16),
          _buildInfoCard("Độ cao (m)", _telemetryData["Do_cao"]?.toStringAsFixed(2) ?? "N/A", Icons.straighten, Colors.blue),
          const SizedBox(height: 16),
          _buildInfoCard("Áp suất (Pa)", _telemetryData["Ap_xuat"]?.toStringAsFixed(2) ?? "N/A", Icons.compress, Colors.grey),
          const SizedBox(height: 16),
          _buildInfoCard("Nhiệt độ (°C)", _telemetryData["Nhiet_do"]?.toStringAsFixed(2) ?? "N/A", Icons.device_thermostat, Colors.deepOrange),
          const SizedBox(height: 16),
          _buildInfoCard("GPS Lat", _telemetryData["gps_lat"]?.toStringAsFixed(6) ?? "N/A", Icons.location_on, Colors.teal),
          const SizedBox(height: 16),
          _buildInfoCard("GPS Lng", _telemetryData["gps_lng"]?.toStringAsFixed(6) ?? "N/A", Icons.location_on, Colors.teal),
          const SizedBox(height: 16),
          _buildInfoCard("GPS Satellites", _telemetryData["gps_sat"]?.toString() ?? "N/A", Icons.satellite_alt, Colors.orange),
          const SizedBox(height: 16),
          _buildInfoCard("GPS HDOP", _telemetryData["gps_hdop"]?.toStringAsFixed(1) ?? "N/A", Icons.gps_fixed, Colors.purple),
          const SizedBox(height: 16),
          _buildInfoCard("GPS Speed (m/s)", _telemetryData["gps_speed"]?.toStringAsFixed(2) ?? "N/A", Icons.speed, Colors.cyan),
          const SizedBox(height: 16),
          _buildInfoCard("GPS Altitude (m)", _telemetryData["gps_alt"]?.toStringAsFixed(2) ?? "N/A", Icons.height, Colors.brown),
          const SizedBox(height: 16),
          _buildInfoCard("GPS Course (°)", _telemetryData["gps_course"]?.toStringAsFixed(2) ?? "N/A", Icons.alt_route, Colors.lime),
          
          const SizedBox(height: 32),
          
          ElevatedButton.icon(
            onPressed: () {
              _udpService.sendCommand("#0,0,0,0,1*");
              debugPrint("Đã gửi lệnh ARM động cơ (Màn hình)!");
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
          ),
          const SizedBox(height: 16),
          ElevatedButton.icon(
            onPressed: () {
              _udpService.sendCommand("#0,0,0,0,0*");
              debugPrint("Đã gửi lệnh DISARM động cơ (Màn hình)!");
            },
            icon: const Icon(Icons.flight_land),
            label: const Padding(
              padding: EdgeInsets.all(12.0),
              child: Text("DISARM ĐỘNG CƠ", style: TextStyle(fontSize: 18)),
            ),
            style: ElevatedButton.styleFrom(
              backgroundColor: Colors.blueGrey,
              foregroundColor: Colors.white,
            ),
          ),
          const SizedBox(height: 16),
          ElevatedButton.icon(
            onPressed: () {
              _showPidTuningDialog(context);
            },
            icon: const Icon(Icons.tune),
            label: const Padding(
              padding: EdgeInsets.all(12.0),
              child: Text("CÀI ĐẶT PID", style: TextStyle(fontSize: 18)),
            ),
            style: ElevatedButton.styleFrom(
              backgroundColor: Colors.teal,
              foregroundColor: Colors.white,
            ),
          )
        ],
      ),
    );
  }

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

  void _showPidTuningDialog(BuildContext context) {
    final TextEditingController kpRollController = TextEditingController(text: _telemetryData["Kp_roll_moi"]?.toString() ?? "0.0");
    final TextEditingController kiRollController = TextEditingController(text: _telemetryData["Ki_roll_moi"]?.toString() ?? "0.0");
    final TextEditingController kdRollController = TextEditingController(text: _telemetryData["Kd_roll_moi"]?.toString() ?? "0.0");

    final TextEditingController kpPitchController = TextEditingController(text: _telemetryData["Kp_pitch_moi"]?.toString() ?? "0.0");
    final TextEditingController kiPitchController = TextEditingController(text: _telemetryData["Ki_pitch_moi"]?.toString() ?? "0.0");
    final TextEditingController kdPitchController = TextEditingController(text: _telemetryData["Kd_pitch_moi"]?.toString() ?? "0.0");

    final TextEditingController kpYawController = TextEditingController(text: _telemetryData["Kp_yaw_moi"]?.toString() ?? "0.0");
    final TextEditingController kiYawController = TextEditingController(text: _telemetryData["Ki_yaw_moi"]?.toString() ?? "0.0");

    showDialog(
      context: context,
      builder: (BuildContext context) {
        return AlertDialog(
          title: const Text("Cài đặt PID"),
          content: SingleChildScrollView(
            child: Column(
              mainAxisSize: MainAxisSize.min,
              children: [
                _buildPidInputField("Kp Roll", kpRollController),
                _buildPidInputField("Ki Roll", kiRollController),
                _buildPidInputField("Kd Roll", kdRollController),
                const SizedBox(height: 16),
                _buildPidInputField("Kp Pitch", kpPitchController),
                _buildPidInputField("Ki Pitch", kiPitchController),
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
                Navigator.of(context).pop();
              },
            ),
            ElevatedButton(
              child: const Text("Lưu"),
              onPressed: () {
                String command = "@";
                command += "${kpRollController.text},${kiRollController.text},${kdRollController.text},";
                command += "${kpPitchController.text},${kiPitchController.text},${kdPitchController.text},";
                command += "${kpYawController.text},${kiYawController.text},0.0*"; 
                _udpService.sendCommand(command);
                debugPrint("Đã gửi lệnh PID: $command");
                Navigator.of(context).pop();
              },
            ),
          ],
        );
      },
    );
  }

  Widget _buildPidInputField(String label, TextEditingController controller) {
    return TextField(
      controller: controller,
      keyboardType: TextInputType.number, 
      decoration: InputDecoration(
        labelText: label,
        border: const OutlineInputBorder(),
      ),
    );
  }
}