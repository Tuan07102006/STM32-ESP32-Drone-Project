import 'dart:io';
import 'package:flutter/foundation.dart';
import 'package:file_selector/file_selector.dart'; // Dùng thư viện chính chủ Google
import 'package:intl/intl.dart';

class DataLoggerService {
  bool isLogging = false;
  IOSink? _sink;
  String? currentFilePath;

  Future<bool> startLogging() async {
    if (isLogging) return false;

    String defaultName = "FlightLog_${DateFormat('yyyyMMdd_HHmmss').format(DateTime.now())}.csv";

    // Hiện bảng Save As để người dùng chọn vị trí lưu và sửa tên file
    final FileSaveLocation? result = await getSaveLocation(
      suggestedName: defaultName,
      acceptedTypeGroups: [
        const XTypeGroup(label: 'CSV Data', extensions: ['csv'])
      ],
    );

    // Nếu người dùng ấn Hủy (Cancel) không lưu nữa
    if (result == null) {
      return false; 
    }

    currentFilePath = result.path;
    File file = File(currentFilePath!);
    
    _sink = file.openWrite(mode: FileMode.write);

    // Ghi tiêu đề (Header)
    _sink?.writeln("Timestamp(ms),Time_Readable,Roll,Pitch,Yaw,Muc_Ga_Cmd,Diem_Dat_Roll,Diem_Dat_Pitch,Do_Cao,Dien_Ap");

    isLogging = true;
    debugPrint("🔴 Đã bắt đầu ghi log tại: $currentFilePath");
    return true;
  }

  void logData(Map<String, dynamic> data) {
    if (!isLogging || _sink == null) return;

    int timestampMs = DateTime.now().millisecondsSinceEpoch;
    String timeReadable = DateFormat('HH:mm:ss.SSS').format(DateTime.now());

    String logLine = "$timestampMs,$timeReadable,${data['Roll']},${data['Pitch']},${data['Yaw']},${data['Muc_Ga']},${data['Diem_dat_Roll']},${data['Diem_dat_Pitch']},${data['Do_cao']},${data['Dien_ap']}";

    _sink?.writeln(logLine);
  }

  Future<void> stopLogging() async {
    if (!isLogging) return;
    isLogging = false;
    
    await _sink?.flush(); 
    await _sink?.close(); 
    _sink = null;
    
    debugPrint("🛑 Đã dừng ghi log. File lưu tại: $currentFilePath");
  }
}