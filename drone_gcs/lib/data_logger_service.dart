import 'dart:io';
import 'package:flutter/foundation.dart';
import 'package:file_selector/file_selector.dart'; // Dùng thư viện chính chủ Google
import 'package:intl/intl.dart';

class DataLoggerService {
  bool isLogging = false;
  IOSink? _sink;
  String? currentFilePath;
  
  // MỚI: Biến đếm số thứ tự mẫu (thay cho mốc thời gian ms)
  int _sampleCount = 0;

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

    if (result == null) {
      return false; 
    }

    currentFilePath = result.path;
    File file = File(currentFilePath!);
    
    _sink = file.openWrite(mode: FileMode.write);

    // GHI TIÊU ĐỀ (Đổi Time_ms thành Sample_Index)
    _sink?.writeln("Sample_Index,Roll,Pitch,Yaw,Muc_Ga_Cmd,Diem_Dat_Roll,Diem_Dat_Pitch,Do_Cao,Dien_Ap");

    // Reset bộ đếm về 0 mỗi khi bắt đầu ghi file mới
    _sampleCount = 0;

    isLogging = true;
    debugPrint("🔴 Đã bắt đầu ghi log tại: $currentFilePath");
    return true;
  }

  void logData(Map<String, dynamic> data) {
    if (!isLogging || _sink == null) return;

    // MỚI: Tăng giá trị bộ đếm lên 1 mỗi khi có một dòng dữ liệu mới được đẩy tới
    _sampleCount++;

    // CHUỖI LOG: Ghi số thứ tự mẫu + dữ liệu cảm biến
    String logLine = "$_sampleCount,${data['Roll']},${data['Pitch']},${data['Yaw']},${data['Muc_Ga']},${data['Diem_dat_Roll']},${data['Diem_dat_Pitch']},${data['Do_cao']},${data['Dien_ap']}";

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