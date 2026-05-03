import 'dart:io';
import 'package:flutter/foundation.dart';
import 'package:file_picker/file_picker.dart';
import 'package:intl/intl.dart';

class DataLoggerService {
  bool isLogging = false;
  IOSink? _sink;
  String? currentFilePath;

  Future<bool> startLogging() async {
    if (isLogging) return false;

    String defaultName = "FlightLog_${DateFormat('yyyyMMdd_HHmmss').format(DateTime.now())}.csv";

    String? outputFile = await FilePicker.platform.saveFile(
      dialogTitle: 'Chọn nơi lưu nhật ký bay (Blackbox)',
      fileName: defaultName,
      type: FileType.custom,
      allowedExtensions: ['csv'],
    );

    if (outputFile == null) {
      return false; 
    }

    currentFilePath = outputFile;
    File file = File(outputFile);
    
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