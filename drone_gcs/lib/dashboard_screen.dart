import 'package:flutter/material.dart';
import 'package:drone_gcs/udp_service.dart';
import 'package:drone_gcs/gamepad_service.dart'; 
import 'package:drone_gcs/data_logger_service.dart'; 

// Import các panels đã chia
import 'charts_panel.dart';
import 'telemetry_panel.dart';
import 'control_panel.dart';

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

  double currentRoll = 0.0;
  double currentPitch = 0.0;
  double currentYaw = 0.0;
  double currentThrottle = 0.0;
  int _isArmed = 0; 

  final ValueNotifier<Map<String, double>> _gamepadStateNotifier = ValueNotifier({});

  @override
  void initState() {
    super.initState();
    _udpService.startListening();
    _udpService.telemetryStream.listen((data) {
      if (mounted) {
        setState(() {
          _telemetryData = data;
        });
      }
      _loggerService.logData(data);
    });

    _initGamepadLogic();
  }

  void _initGamepadLogic() async {
    await _gamepadService.init();

    _gamepadService.onControlUpdated = (roll, pitch, yaw, throttle) {
      if (mounted) {
        setState(() {
          currentRoll = roll;
          currentPitch = pitch;
          currentYaw = yaw;
          currentThrottle = throttle; 
        });
      }
      double realThrottle = 1000.0 + (throttle * 10.0); 
      String command = "#${realThrottle.toStringAsFixed(1)},${roll.toStringAsFixed(1)},${pitch.toStringAsFixed(1)},${yaw.toStringAsFixed(1)},$_isArmed*";
      _udpService.sendCommand(command);
    };

    _gamepadService.onArmPressed = _armDrone;
    _gamepadService.onDisarmPressed = _disarmDrone;

    _gamepadService.onKeyStatesChanged = (states) {
      _gamepadStateNotifier.value = Map.from(states);
    };
  }
  
  // --- Các hàm Logic bay ---
  void _armDrone() {
    if (mounted) setState(() { _isArmed = 1; });
    double realThrottle = 1000.0 + (currentThrottle * 10.0);
    _udpService.sendCommand("#${realThrottle.toStringAsFixed(1)},0.0,0.0,0.0,1*");
  }

  void _disarmDrone() {
    if (mounted) setState(() { _isArmed = 0; });
    _udpService.sendCommand("#1000.0,0.0,0.0,0.0,0*");
  }

  Future<void> _toggleLog() async {
    if (_isRecording) {
      await _loggerService.stopLogging();
      if (mounted) setState(() { _isRecording = false; });
    } else {
      bool started = await _loggerService.startLogging();
      if (started && mounted) { setState(() { _isRecording = true; }); }
    }
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
            // CỘT 1
            Expanded(
              flex: 4,
              child: ChartsPanel(
                telemetryData: _telemetryData,
                currentRoll: currentRoll,
                currentPitch: currentPitch,
                currentYaw: currentYaw,
              ),
            ),
            const SizedBox(width: 12),
            
            // CỘT 2
            Expanded(
              flex: 3,
              child: TelemetryPanel(telemetryData: _telemetryData),
            ),
            const SizedBox(width: 12),

            // CỘT 3
            Expanded(
              flex: 3,
              child: ControlPanel(
                gamepadStateNotifier: _gamepadStateNotifier,
                currentThrottle: currentThrottle,
                currentRoll: currentRoll,
                currentPitch: currentPitch,
                currentYaw: currentYaw,
                isRecording: _isRecording,
                telemetryData: _telemetryData,
                onToggleRecord: _toggleLog,
                onArm: _armDrone,
                onDisarm: _disarmDrone,
                onSendPidCommand: (cmd) => _udpService.sendCommand(cmd),
              ),
            ),
          ],
        ),
      ),
    );
  }
}