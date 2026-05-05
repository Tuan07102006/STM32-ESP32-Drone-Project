import 'dart:async';
import 'dart:io';
import 'package:flutter/foundation.dart'; 

class UDPService {
  RawDatagramSocket? _udpSocket;
  final int _localPort = 34542;
  final String _targetIP = "192.168.137.116"; // Địa chỉ IP của ESP32
  final int _targetPort = 34542;

  final StreamController<Map<String, dynamic>> _telemetryController = StreamController<Map<String, dynamic>>.broadcast();
  Stream<Map<String, dynamic>> get telemetryStream => _telemetryController.stream;

  Future<void> startListening() async {
    _udpSocket = await RawDatagramSocket.bind(InternetAddress("192.168.137.79"), _localPort);
    debugPrint('UDP listening on port: ${_udpSocket!.port}'); 

    _udpSocket!.listen((RawSocketEvent event) {
      if (event == RawSocketEvent.read) {
        Datagram? datagram = _udpSocket!.receive();
        if (datagram != null) {
          String message = String.fromCharCodes(datagram.data);
          _parseTelemetryData(message);
        }
      }
    });
  }

  void _parseTelemetryData(String message) {
    if (message.startsWith("\$") && message.endsWith("*")) {
      String data = message.substring(1, message.length - 1);
      List<String> values = data.split(",");

      // SỬA 1: Đổi 29 thành 28
      if (values.length >= 28) { 
        try {
          _telemetryController.add({
            'Roll': double.tryParse(values[0]) ?? 0.0,
            'Pitch': double.tryParse(values[1]) ?? 0.0,
            'Yaw': double.tryParse(values[2]) ?? 0.0,
            'Dien_ap': double.tryParse(values[3]) ?? 0.0,
            'Dong_dien': double.tryParse(values[4]) ?? 0.0,
            'Do_cao': double.tryParse(values[5]) ?? 0.0,
            'Ap_xuat': double.tryParse(values[6]) ?? 0.0,
            'Nhiet_do': double.tryParse(values[7]) ?? 0.0,
            'gps_lat': double.tryParse(values[8]) ?? 0.0,
            'gps_lng': double.tryParse(values[9]) ?? 0.0,
            'gps_sat': int.tryParse(values[10]) ?? 0,
            'gps_hdop': double.tryParse(values[11]) ?? 0.0,
            'gps_speed': double.tryParse(values[12]) ?? 0.0,
            'gps_alt': double.tryParse(values[13]) ?? 0.0,
            'gps_course': double.tryParse(values[14]) ?? 0.0,
            'Muc_Ga': double.tryParse(values[15]) ?? 0.0,
            'Diem_dat_Roll': double.tryParse(values[16]) ?? 0.0,
            'Diem_dat_Pitch': double.tryParse(values[17]) ?? 0.0,
            'Diem_dat_Yaw': double.tryParse(values[18]) ?? 0.0,
            'Kp_roll_moi': double.tryParse(values[19]) ?? 0.0,
            'Ki_roll_moi': double.tryParse(values[20]) ?? 0.0,
            'Kd_roll_moi': double.tryParse(values[21]) ?? 0.0,
            'Kp_pitch_moi': double.tryParse(values[22]) ?? 0.0,
            'Ki_pitch_moi': double.tryParse(values[23]) ?? 0.0,
            'Kd_pitch_moi': double.tryParse(values[24]) ?? 0.0,
            'Kp_yaw_moi': double.tryParse(values[25]) ?? 0.0,
            'Ki_yaw_moi': double.tryParse(values[26]) ?? 0.0,
            'Trang_thai_Arm': int.tryParse(values[27]) ?? 0,
          });
        } catch (e) {
          debugPrint('Error parsing telemetry data: $e'); 
        }
      } else {
        debugPrint('Telemetry data format error: Invalid number of values (${values.length})'); 
      }
    } else {
      debugPrint('Telemetry data format error: Missing start/end markers.'); 
    }
  }

  void sendCommand(String command) {
    _udpSocket?.send(command.codeUnits, InternetAddress(_targetIP), _targetPort);
  }

  void dispose() {
    _udpSocket?.close();
    _telemetryController.close();
  }
}