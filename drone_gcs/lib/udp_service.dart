import 'dart:async';
import 'dart:io';
import 'package:flutter/foundation.dart'; 

class UDPService {
  RawDatagramSocket? _udpSocket;
  final int _localPort = 12345;
  final String _targetIP = "192.168.10.55"; // Địa chỉ IP của ESP32
  final int _targetPort = 12345;

  final StreamController<Map<String, dynamic>> _telemetryController = StreamController<Map<String, dynamic>>.broadcast();
  Stream<Map<String, dynamic>> get telemetryStream => _telemetryController.stream;

  Future<void> startListening() async {
    _udpSocket = await RawDatagramSocket.bind(InternetAddress.anyIPv4, _localPort);
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

      if (values.length >= 29) { 
        try {
          _telemetryController.add({
            'Roll': double.parse(values[0]),
            'Pitch': double.parse(values[1]),
            'Yaw': double.parse(values[2]),
            'Dien_ap': double.parse(values[3]),
            'Dong_dien': double.parse(values[4]),
            'Do_cao': double.parse(values[5]),
            'Ap_xuat': double.parse(values[6]),
            'Nhiet_do': double.parse(values[7]),
            'gps_lat': double.parse(values[8]),
            'gps_lng': double.parse(values[9]),
            'gps_sat': int.parse(values[10]),
            'gps_hdop': double.parse(values[11]),
            'gps_speed': double.parse(values[12]),
            'gps_alt': double.parse(values[13]),
            'gps_course': double.parse(values[14]),
            'Muc_Ga': double.parse(values[15]),
            'Diem_dat_Roll': double.parse(values[16]),
            'Diem_dat_Pitch': double.parse(values[17]),
            'Diem_dat_Yaw': double.parse(values[18]),
            'Kp_roll_moi': double.parse(values[19]),
            'Ki_roll_moi': double.parse(values[20]),
            'Kd_roll_moi': double.parse(values[21]),
            'Kp_pitch_moi': double.parse(values[22]),
            'Ki_pitch_moi': double.parse(values[23]),
            'Kd_pitch_moi': double.parse(values[24]),
            'Kp_yaw_moi': double.parse(values[25]),
            'Ki_yaw_moi': double.parse(values[26]),
            'Trang_thai_Arm': int.parse(values[28]),
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