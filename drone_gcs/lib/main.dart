import 'package:flutter/material.dart';
import 'package:drone_gcs/udp_service.dart';
import 'package:drone_gcs/gamepad_service.dart'; 
import 'package:drone_gcs/data_logger_service.dart'; 

void main() {
  WidgetsFlutterBinding.ensureInitialized();
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
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.blueGrey, brightness: Brightness.dark),
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
  final UDPService _udpService = UDPService();
  final GamepadService _gamepadService = GamepadService(); 
  final DataLoggerService _loggerService = DataLoggerService(); 
  
  Map<String, dynamic> _telemetryData = {};
  bool _isRecording = false; 

  // Lưu trữ các giá trị lệnh điều khiển (Command)
  double currentRoll = 0.0;
  double currentPitch = 0.0;
  double currentYaw = 0.0;
  double currentThrottle = 0.0;
  int _isArmed = 0; // Biến lưu trạng thái Arm hiện tại

  final ValueNotifier<Map<String, double>> _gamepadStateNotifier = ValueNotifier({});

  @override
  void initState() {
    super.initState();
    _udpService.startListening();
    _udpService.telemetryStream.listen((data) {
      setState(() {
        _telemetryData = data;
      });
      _loggerService.logData(data);
    });

    _initGamepadLogic();
  }

  void _initGamepadLogic() async {
    await _gamepadService.init();

    _gamepadService.onControlUpdated = (roll, pitch, yaw, throttle) {
      setState(() {
        currentRoll = roll;
        currentPitch = pitch;
        currentYaw = yaw;
        currentThrottle = throttle; // Hiển thị 0-100% trên giao diện
      });

      // QUAN TRỌNG: Quy đổi ga từ phần trăm (0-100) sang PWM (1000-2000)
      double realThrottle = 1000.0 + (throttle * 10.0); 
      
      // Gửi đủ 5 tham số: Ga, Roll, Pitch, Yaw, và trạng thái ARM
      String command = "#${realThrottle.toStringAsFixed(1)},${roll.toStringAsFixed(1)},${pitch.toStringAsFixed(1)},${yaw.toStringAsFixed(1)},$_isArmed*";
      _udpService.sendCommand(command);
    };

    _gamepadService.onArmPressed = () {
      setState(() { _isArmed = 1; });
      double realThrottle = 1000.0 + (currentThrottle * 10.0);
      _udpService.sendCommand("#${realThrottle.toStringAsFixed(1)},0.0,0.0,0.0,1*");
    };

    _gamepadService.onDisarmPressed = () {
      setState(() { _isArmed = 0; });
      _udpService.sendCommand("#1000.0,0.0,0.0,0.0,0*");
    };

    _gamepadService.onKeyStatesChanged = (states) {
      _gamepadStateNotifier.value = Map.from(states);
    };
  }

  @override
  void dispose() {
    _gamepadStateNotifier.dispose();
    _udpService.dispose();
    _gamepadService.dispose(); 
    _loggerService.stopLogging(); 
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('GCS - Trạm Giám Sát Bay', style: TextStyle(fontWeight: FontWeight.bold, letterSpacing: 2)),
        backgroundColor: Colors.black87,
        centerTitle: true,
      ),
      body: Container(
        color: Colors.blueGrey.shade900,
        padding: const EdgeInsets.all(12.0),
        child: Row(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            // ================= CỘT 1 =================
            Expanded(
              flex: 3,
              child: Column(
                children: [
                  _buildSectionHeader("CẢM BIẾN & NGUỒN", Icons.sensors),
                  Expanded(
                    child: GridView.count(
                      crossAxisCount: 2,
                      childAspectRatio: 2.5,
                      mainAxisSpacing: 8,
                      crossAxisSpacing: 8,
                      children: [
                        _buildCompactCard("Roll", _telemetryData["Roll"]?.toStringAsFixed(2) ?? "0.0", "°", Colors.indigo),
                        _buildCompactCard("Pitch", _telemetryData["Pitch"]?.toStringAsFixed(2) ?? "0.0", "°", Colors.deepPurple),
                        _buildCompactCard("Yaw", _telemetryData["Yaw"]?.toStringAsFixed(2) ?? "0.0", "°", Colors.pink),
                        _buildCompactCard("Độ cao", _telemetryData["Do_cao"]?.toStringAsFixed(2) ?? "0.0", "m", Colors.blue),
                        _buildCompactCard("Điện áp", _telemetryData["Dien_ap"]?.toStringAsFixed(2) ?? "0.0", "V", Colors.green),
                        _buildCompactCard("Dòng điện", _telemetryData["Dong_dien"]?.toStringAsFixed(2) ?? "0.0", "A", Colors.red),
                        _buildCompactCard("Áp suất", _telemetryData["Ap_xuat"]?.toStringAsFixed(0) ?? "0", "Pa", Colors.grey),
                        _buildCompactCard("Nhiệt độ", _telemetryData["Nhiet_do"]?.toStringAsFixed(1) ?? "0.0", "°C", Colors.orange),
                      ],
                    ),
                  ),
                ],
              ),
            ),
            
            const SizedBox(width: 12),

            // ================= CỘT 2 =================
            Expanded(
              flex: 3,
              child: Column(
                children: [
                  _buildSectionHeader("ĐỊNH VỊ VỆ TINH", Icons.satellite_alt),
                  Expanded(
                    child: GridView.count(
                      crossAxisCount: 2,
                      childAspectRatio: 2.5,
                      mainAxisSpacing: 8,
                      crossAxisSpacing: 8,
                      children: [
                        _buildCompactCard("Vĩ độ (Lat)", _telemetryData["gps_lat"]?.toStringAsFixed(6) ?? "0.0", "", Colors.teal),
                        _buildCompactCard("Kinh độ (Lng)", _telemetryData["gps_lng"]?.toStringAsFixed(6) ?? "0.0", "", Colors.teal),
                        _buildCompactCard("Vệ tinh", _telemetryData["gps_sat"]?.toString() ?? "0", "", Colors.amber),
                        _buildCompactCard("Độ chính xác", _telemetryData["gps_hdop"]?.toStringAsFixed(1) ?? "0.0", "", Colors.purple),
                        _buildCompactCard("Vận tốc", _telemetryData["gps_speed"]?.toStringAsFixed(2) ?? "0.0", "m/s", Colors.cyan),
                        _buildCompactCard("Độ cao GPS", _telemetryData["gps_alt"]?.toStringAsFixed(2) ?? "0.0", "m", Colors.brown),
                        _buildCompactCard("Hướng đi", _telemetryData["gps_course"]?.toStringAsFixed(1) ?? "0.0", "°", Colors.lime),
                      ],
                    ),
                  ),
                ],
              ),
            ),

            const SizedBox(width: 12),

            // ================= CỘT 3 =================
            Expanded(
              flex: 2,
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  _buildSectionHeader("ĐIỀU KHIỂN & LỆNH", Icons.gamepad),
                  
                  Expanded(
                    child: SingleChildScrollView(
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.stretch,
                        children: [
                          GamepadVisualizer(stateNotifier: _gamepadStateNotifier),
                          _buildGamepadLegend(),
                          const SizedBox(height: 12),
                          _buildCommandedValuesPanel(),
                          const SizedBox(height: 12),

                          Card(
                            color: _isRecording ? Colors.red.shade900 : Colors.blueGrey.shade800,
                            margin: EdgeInsets.zero,
                            child: Padding(
                              padding: const EdgeInsets.all(12.0),
                              child: Column(
                                children: [
                                  Icon(_isRecording ? Icons.fiber_manual_record : Icons.save_alt, 
                                       color: _isRecording ? Colors.redAccent : Colors.white, size: 28),
                                  const SizedBox(height: 8),
                                  ElevatedButton(
                                    style: ElevatedButton.styleFrom(
                                      backgroundColor: _isRecording ? Colors.black54 : Colors.red,
                                      minimumSize: const Size.fromHeight(40),
                                    ),
                                    onPressed: () async {
                                      if (_isRecording) {
                                        await _loggerService.stopLogging();
                                        setState(() { _isRecording = false; });
                                      } else {
                                        bool started = await _loggerService.startLogging();
                                        if (started) { setState(() { _isRecording = true; }); }
                                      }
                                    },
                                    child: Text(_isRecording ? "DỪNG GHI LOG" : "GHI NHẬT KÝ BAY", style: const TextStyle(fontSize: 12)),
                                  ),
                                ],
                              ),
                            ),
                          ),
                          const SizedBox(height: 16),

                          ElevatedButton.icon(
                            onPressed: () {
                              setState(() { _isArmed = 1; });
                              double realThrottle = 1000.0 + (currentThrottle * 10.0);
                              _udpService.sendCommand("#${realThrottle.toStringAsFixed(1)},0.0,0.0,0.0,1*");
                            },
                            icon: const Icon(Icons.flight_takeoff),
                            label: const Text("ARM", style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold)),
                            style: ElevatedButton.styleFrom(
                              backgroundColor: Colors.redAccent,
                              foregroundColor: Colors.white,
                              padding: const EdgeInsets.symmetric(vertical: 16),
                            ),
                          ),
                          const SizedBox(height: 12),
                          ElevatedButton.icon(
                            onPressed: () {
                              setState(() { _isArmed = 0; });
                              _udpService.sendCommand("#1000.0,0.0,0.0,0.0,0*");
                            },
                            icon: const Icon(Icons.flight_land),
                            label: const Text("DISARM", style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold)),
                            style: ElevatedButton.styleFrom(
                              backgroundColor: Colors.orange,
                              foregroundColor: Colors.white,
                              padding: const EdgeInsets.symmetric(vertical: 16),
                            ),
                          ),
                          const SizedBox(height: 12),
                          ElevatedButton.icon(
                            onPressed: () => _showPidTuningDialog(context),
                            icon: const Icon(Icons.tune),
                            label: const Text("TÙY CHỈNH PID", style: TextStyle(fontSize: 14)),
                            style: ElevatedButton.styleFrom(
                              backgroundColor: Colors.teal,
                              foregroundColor: Colors.white,
                              padding: const EdgeInsets.symmetric(vertical: 16),
                            ),
                          ),
                        ],
                      ),
                    ),
                  ),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildSectionHeader(String title, IconData icon) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 12.0),
      child: Row(
        children: [
          Icon(icon, color: Colors.white70),
          const SizedBox(width: 8),
          Text(title, style: const TextStyle(color: Colors.white70, fontSize: 16, fontWeight: FontWeight.bold)),
        ],
      ),
    );
  }

  Widget _buildCompactCard(String title, String value, String unit, Color accentColor) {
    return Card(
      color: Colors.blueGrey.shade800,
      elevation: 2,
      shape: RoundedRectangleBorder(
        side: BorderSide(color: accentColor.withOpacity(0.3), width: 2),
        borderRadius: BorderRadius.circular(8),
      ),
      child: Padding(
        padding: const EdgeInsets.symmetric(horizontal: 12.0, vertical: 8.0),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(title, style: TextStyle(fontSize: 12, color: Colors.blueGrey.shade200)),
            const Spacer(),
            Row(
              crossAxisAlignment: CrossAxisAlignment.end,
              children: [
                Text(value, style: const TextStyle(fontSize: 22, fontWeight: FontWeight.bold, color: Colors.white)),
                const SizedBox(width: 4),
                Text(unit, style: TextStyle(fontSize: 14, color: accentColor)),
              ],
            )
          ],
        ),
      ),
    );
  }

  Widget _buildGamepadLegend() {
    return Container(
      padding: const EdgeInsets.all(10),
      decoration: BoxDecoration(
        color: Colors.black26,
        borderRadius: BorderRadius.circular(8),
      ),
      child: Column(
        children: [
          const Text("GÁN NÚT CHỨC NĂNG", style: TextStyle(fontSize: 11, fontWeight: FontWeight.bold, color: Colors.white54, letterSpacing: 1)),
          const SizedBox(height: 8),
          _buildLegendRow("🕹️ L-Stick", "Roll / Pitch"),
          _buildLegendRow("🕹️ R-Stick", "Yaw"),
          _buildLegendRow("🎯 LT / RT", "Giảm / Tăng Ga"),
          _buildLegendRow("🟢 Nút A", "ARM (Khởi động)"),
          _buildLegendRow("🔴 Nút B", "DISARM (Ngắt)"),
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
          Text(key, style: const TextStyle(fontSize: 11, color: Colors.cyan)),
          Text(func, style: const TextStyle(fontSize: 11, color: Colors.white)),
        ],
      ),
    );
  }

  Widget _buildCommandedValuesPanel() {
    return Container(
      padding: const EdgeInsets.all(12),
      decoration: BoxDecoration(
        color: Colors.blueGrey.shade800,
        borderRadius: BorderRadius.circular(8),
        border: Border.all(color: Colors.cyan.withOpacity(0.4), width: 1.5),
      ),
      child: Column(
        children: [
          const Text("TÍN HIỆU ĐANG GỬI (COMMAND)", style: TextStyle(fontSize: 11, fontWeight: FontWeight.bold, color: Colors.cyan, letterSpacing: 1)),
          const SizedBox(height: 10),
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
        const SizedBox(height: 4),
        Text("${val.toStringAsFixed(1)}$unit", style: const TextStyle(fontSize: 14, fontWeight: FontWeight.bold, color: Colors.white)),
      ],
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
            TextButton(child: const Text("Hủy"), onPressed: () => Navigator.of(context).pop()),
            ElevatedButton(
              child: const Text("Lưu"),
              onPressed: () {
                String command = "@${kpRollController.text},${kiRollController.text},${kdRollController.text},${kpPitchController.text},${kiPitchController.text},${kdPitchController.text},${kpYawController.text},${kiYawController.text},0.0*"; 
                _udpService.sendCommand(command);
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
      decoration: InputDecoration(labelText: label, border: const OutlineInputBorder()),
    );
  }
}

class GamepadVisualizer extends StatelessWidget {
  final ValueNotifier<Map<String, double>> stateNotifier;

  const GamepadVisualizer({super.key, required this.stateNotifier});

  @override
  Widget build(BuildContext context) {
    return AspectRatio(
      aspectRatio: 1.5,
      child: Container(
        decoration: BoxDecoration(
          color: Colors.transparent, 
          borderRadius: BorderRadius.circular(16),
        ),
        padding: const EdgeInsets.all(8.0),
        child: ValueListenableBuilder<Map<String, double>>(
          valueListenable: stateNotifier,
          builder: (context, states, child) {
            double lt = _getVal(states, "a_4");
            double rt = _getVal(states, "a_5");
            double lb = _getVal(states, "b_4"); 
            double rb = _getVal(states, "b_5");
            
            double stickLeftX = _getVal(states, "a_0");
            double stickLeftY = _getVal(states, "a_1");
            double stickRightX = _getVal(states, "a_2");
            double stickRightY = _getVal(states, "a_3"); 

            bool btnA = _getVal(states, "b_0") > 0.5; 
            bool btnB = _getVal(states, "b_1") > 0.5;
            bool btnX = _getVal(states, "b_2") > 0.5;
            bool btnY = _getVal(states, "b_3") > 0.5;

            bool dpadUp = _getVal(states, "b_12") > 0.5 || _getVal(states, "a_7") < -0.5;
            bool dpadDown = _getVal(states, "b_13") > 0.5 || _getVal(states, "a_7") > 0.5;
            bool dpadLeft = _getVal(states, "b_14") > 0.5 || _getVal(states, "a_6") < -0.5;
            bool dpadRight = _getVal(states, "b_15") > 0.5 || _getVal(states, "a_6") > 0.5;

            return LayoutBuilder(
              builder: (context, constraints) {
                final double w = constraints.maxWidth;
                final double h = constraints.maxHeight;

                return Stack(
                  children: [
                    _placeAt(w, h, 0.22, 0.15, _buildTriggerBox("LT", lt)),
                    _placeAt(w, h, 0.78, 0.15, _buildTriggerBox("RT", rt)),
                    _placeAt(w, h, 0.22, 0.30, _buildBumperBox("LB", lb > 0.5)),
                    _placeAt(w, h, 0.78, 0.30, _buildBumperBox("RB", rb > 0.5)),
                    _placeAt(w, h, 0.22, 0.55, _buildStick(w * 0.16, stickLeftX, stickLeftY)),
                    _placeAt(w, h, 0.78, 0.55, _buildActionButtons(w * 0.18, btnA, btnB, btnX, btnY)),
                    _placeAt(w, h, 0.38, 0.80, _buildDPad(w * 0.14, dpadUp, dpadDown, dpadLeft, dpadRight)),
                    _placeAt(w, h, 0.62, 0.80, _buildStick(w * 0.16, stickRightX, stickRightY)),
                    _placeAt(w, h, 0.45, 0.55, _buildCenterButton(_getVal(states, "b_8") > 0.5)),
                    _placeAt(w, h, 0.55, 0.55, _buildCenterButton(_getVal(states, "b_9") > 0.5)),
                  ],
                );
              },
            );
          },
        ),
      ),
    );
  }

  Widget _placeAt(double w, double h, double xPct, double yPct, Widget child) {
    return Positioned(
      left: w * xPct,
      top: h * yPct,
      child: FractionalTranslation(
        translation: const Offset(-0.5, -0.5), 
        child: child,
      ),
    );
  }

  double _getVal(Map<String, double> states, String key) {
    return states[key] ?? 0.0;
  }

  Widget _buildTriggerBox(String label, double intensity) {
    intensity = intensity.clamp(0.0, 1.0);
    return Container(
      width: 32,
      height: 18 + (intensity * 10), 
      decoration: BoxDecoration(
        color: intensity > 0.1 ? Colors.cyan.withOpacity(intensity) : Colors.transparent,
        border: Border.all(color: Colors.white70, width: 2),
        borderRadius: const BorderRadius.vertical(top: Radius.circular(8)),
        boxShadow: intensity > 0.1 ? [BoxShadow(color: Colors.cyan, blurRadius: 10 * intensity)] : [],
      ),
      alignment: Alignment.center,
      child: Text(label, style: const TextStyle(fontSize: 10, color: Colors.white, fontWeight: FontWeight.bold)),
    );
  }

  Widget _buildBumperBox(String label, bool isPressed) {
    return Container(
      width: 40,
      height: 14,
      decoration: BoxDecoration(
        color: isPressed ? Colors.cyan : Colors.transparent,
        border: Border.all(color: Colors.white70, width: 2),
        borderRadius: BorderRadius.circular(6),
        boxShadow: isPressed ? [const BoxShadow(color: Colors.cyan, blurRadius: 8)] : [],
      ),
    );
  }

  Widget _buildCenterButton(bool isPressed) {
    return Container(
      width: 10,
      height: 10,
      decoration: BoxDecoration(
        color: isPressed ? Colors.cyan : Colors.white24,
        shape: BoxShape.circle,
        boxShadow: isPressed ? [const BoxShadow(color: Colors.cyan, blurRadius: 8)] : [],
      ),
    );
  }

  Widget _buildStick(double size, double x, double y) {
    double maxOffset = size * 0.3; 
    bool isActive = x.abs() > 0.1 || y.abs() > 0.1;

    return Container(
      width: size,
      height: size,
      decoration: BoxDecoration(
        color: Colors.blueGrey.shade800,
        shape: BoxShape.circle,
        border: Border.all(color: Colors.white30, width: 2),
      ),
      child: Center(
        child: Transform.translate(
          offset: Offset(x * maxOffset, y * maxOffset),
          child: Container(
            width: size * 0.6,
            height: size * 0.6,
            decoration: BoxDecoration(
              color: isActive ? Colors.cyan : Colors.blueGrey.shade600,
              shape: BoxShape.circle,
              boxShadow: isActive ? [const BoxShadow(color: Colors.cyan, blurRadius: 10)] : [],
            ),
          ),
        ),
      ),
    );
  }

  Widget _buildDPad(double size, bool up, bool down, bool left, bool right) {
    return SizedBox(
      width: size,
      height: size,
      child: Stack(
        children: [
          Positioned(
            top: 0, bottom: 0, left: size * 0.35, right: size * 0.35,
            child: Container(color: (up || down) ? Colors.cyan : Colors.white54),
          ),
          Positioned(
            top: size * 0.35, bottom: size * 0.35, left: 0, right: 0,
            child: Container(color: (left || right) ? Colors.cyan : Colors.white54),
          ),
        ],
      ),
    );
  }

  Widget _buildActionButtons(double size, bool a, bool b, bool x, bool y) {
    double btnSize = size * 0.3;
    return SizedBox(
      width: size,
      height: size,
      child: Stack(
        children: [
          Positioned(top: 0, left: (size - btnSize) / 2, child: _buildCircleBtn("Y", y, Colors.amber)),
          Positioned(bottom: 0, left: (size - btnSize) / 2, child: _buildCircleBtn("A", a, Colors.green)),
          Positioned(top: (size - btnSize) / 2, left: 0, child: _buildCircleBtn("X", x, Colors.blue)),
          Positioned(top: (size - btnSize) / 2, right: 0, child: _buildCircleBtn("B", b, Colors.red)),
        ],
      ),
    );
  }

  Widget _buildCircleBtn(String label, bool isPressed, Color activeColor) {
    return Container(
      width: 18,
      height: 18,
      decoration: BoxDecoration(
        color: isPressed ? activeColor : Colors.transparent,
        shape: BoxShape.circle,
        border: Border.all(color: isPressed ? Colors.white : Colors.white54, width: 1.5),
        boxShadow: isPressed ? [BoxShadow(color: activeColor, blurRadius: 10)] : [],
      ),
      alignment: Alignment.center,
      child: Text(label, style: const TextStyle(fontSize: 8, color: Colors.white, fontWeight: FontWeight.bold)),
    );
  }
}