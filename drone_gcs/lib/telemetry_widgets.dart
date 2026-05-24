import 'package:flutter/material.dart';
import 'package:fl_chart/fl_chart.dart';
import 'dart:math' as math;
import 'dart:async'; 

// ==========================================
// 1. WIDGET LA BÀN CHO GÓC YAW (GY-271)
// ==========================================
class CompassWidget extends StatelessWidget {
  final double yaw;
  final double targetYaw;

  const CompassWidget({super.key, required this.yaw, required this.targetYaw});

  @override
  Widget build(BuildContext context) {
    return Column(
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        const Text("LA BÀN YAW", style: TextStyle(color: Colors.white70, fontWeight: FontWeight.bold, fontSize: 12)),
        const SizedBox(height: 8),
        Expanded(
          child: FittedBox(
            fit: BoxFit.contain, 
            child: SizedBox(
              width: 150, 
              height: 150,
              child: Stack(
                alignment: Alignment.center,
                children: [
                  Container(
                    width: 140,
                    height: 140,
                    decoration: BoxDecoration(
                      shape: BoxShape.circle,
                      border: Border.all(color: Colors.cyan.withOpacity(0.5), width: 2),
                      color: Colors.blueGrey.shade900,
                    ),
                    child: const Stack(
                      children: [
                        Positioned(top: 5, left: 65, child: Text("N", style: TextStyle(color: Colors.red, fontWeight: FontWeight.bold))),
                        Positioned(bottom: 5, left: 65, child: Text("S", style: TextStyle(color: Colors.white54))),
                        Positioned(left: 8, top: 60, child: Text("W", style: TextStyle(color: Colors.white54))),
                        Positioned(right: 8, top: 60, child: Text("E", style: TextStyle(color: Colors.white54))),
                      ],
                    ),
                  ),
                  Transform.rotate(
                    angle: targetYaw * (math.pi / 180),
                    child: const Icon(Icons.navigation_outlined, color: Colors.white30, size: 120),
                  ),
                  Transform.rotate(
                    angle: yaw * (math.pi / 180),
                    child: const Icon(Icons.navigation, color: Colors.pinkAccent, size: 100),
                  ),
                ],
              ),
            ),
          ),
        ),
        const SizedBox(height: 8),
        Text("Thực tế: ${yaw.toStringAsFixed(1)}°", style: const TextStyle(color: Colors.pinkAccent, fontWeight: FontWeight.bold, fontSize: 12)),
        Text("Mục tiêu: ${targetYaw.toStringAsFixed(1)}°", style: const TextStyle(color: Colors.white54, fontSize: 11)),
      ],
    );
  }
}

// ==========================================
// 2. WIDGET BIỂU ĐỒ SO SÁNH (ROLL / PITCH)
// ==========================================
class RealtimeLineChart extends StatefulWidget {
  final String title;
  final double actualValue;
  final double targetValue;
  final Color actualColor;
  final Color targetColor;

  const RealtimeLineChart({
    super.key,
    required this.title,
    required this.actualValue,
    required this.targetValue,
    this.actualColor = Colors.greenAccent,
    this.targetColor = Colors.white, // ĐÃ ĐỔI: Màu mặc định thành trắng tinh
  });

  @override
  State<RealtimeLineChart> createState() => _RealtimeLineChartState();
}

class _RealtimeLineChartState extends State<RealtimeLineChart> {
  final int maxDataPoints = 50; 
  List<FlSpot> actualSpots = [];
  List<FlSpot> targetSpots = [];
  double time = 0;
  Timer? _timer;

  @override
  void initState() {
    super.initState();
    actualSpots.add(const FlSpot(0, 0));
    targetSpots.add(const FlSpot(0, 0));

    _timer = Timer.periodic(const Duration(milliseconds: 100), (timer) {
      if (!mounted) return;

      double aVal = widget.actualValue.isNaN || widget.actualValue.isInfinite ? 0.0 : widget.actualValue;
      double tVal = widget.targetValue.isNaN || widget.targetValue.isInfinite ? 0.0 : widget.targetValue;

      setState(() {
        time += 1;
        actualSpots.add(FlSpot(time, aVal));
        targetSpots.add(FlSpot(time, tVal));

        if (actualSpots.length > maxDataPoints) {
          actualSpots.removeAt(0);
          targetSpots.removeAt(0);
        }
      });
    });
  }

  @override
  void dispose() {
    _timer?.cancel(); 
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    double currentMinX = actualSpots.isNotEmpty ? actualSpots.first.x : 0;
    double currentMaxX = actualSpots.isNotEmpty ? actualSpots.last.x : maxDataPoints.toDouble();

    return Container(
      padding: const EdgeInsets.all(8),
      decoration: BoxDecoration(
        color: Colors.blueGrey.shade900,
        borderRadius: BorderRadius.circular(8),
        border: Border.all(color: Colors.white12),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Text(widget.title, style: const TextStyle(color: Colors.white, fontWeight: FontWeight.bold, fontSize: 11)),
              Row(
                children: [
                  _buildLegend("Đo được", widget.actualColor),
                  const SizedBox(width: 8),
                  _buildLegend("Điểm đặt", widget.targetColor),
                ],
              )
            ],
          ),
          const SizedBox(height: 10),
          Expanded(
            child: LineChart(
              LineChartData(
                clipData: const FlClipData.all(), 
                minY: -90, 
                maxY: 90,
                minX: currentMinX, 
                maxX: currentMaxX,  
                gridData: const FlGridData(
                  show: true, 
                  drawVerticalLine: false,
                  horizontalInterval: 30, 
                ),
                titlesData: FlTitlesData(
                  show: true,
                  topTitles: const AxisTitles(sideTitles: SideTitles(showTitles: false)),
                  rightTitles: const AxisTitles(sideTitles: SideTitles(showTitles: false)),
                  bottomTitles: const AxisTitles(sideTitles: SideTitles(showTitles: false)),
                  leftTitles: AxisTitles(
                    sideTitles: SideTitles(
                      showTitles: true,
                      reservedSize: 28,
                      getTitlesWidget: (double value, TitleMeta meta) { 
                        if (value == -90 || value == -45 || value == 0 || value == 45 || value == 90) {
                          return SideTitleWidget(
                            axisSide: meta.axisSide,
                            child: Text("${value.toInt()}°", style: const TextStyle(color: Colors.white30, fontSize: 9)),
                          );
                        }
                        return const SizedBox.shrink();
                      },
                    ),
                  ),
                ),
                borderData: FlBorderData(show: false),
                lineBarsData: [
                  // DẢI ĐIỂM ĐẶT (TARGET)
                  LineChartBarData(
                    spots: targetSpots,
                    isCurved: true, // Đường cong
                    color: widget.targetColor, // Màu trắng (theo constructor)
                    barWidth: 2,
                    isStrokeCapRound: true,
                    dotData: const FlDotData(show: false),
                    // dashArray: [5, 5], // ĐÃ XÓA: Để trở thành nét liền
                  ),
                  // DẢI ĐO ĐƯỢC (ACTUAL)
                  LineChartBarData(
                    spots: actualSpots,
                    isCurved: true,
                    color: widget.actualColor,
                    barWidth: 2.5,
                    isStrokeCapRound: true,
                    dotData: const FlDotData(show: false),
                  ),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildLegend(String text, Color color) {
    return Row(
      children: [
        Container(width: 8, height: 8, decoration: BoxDecoration(color: color, shape: BoxShape.circle)),
        const SizedBox(width: 4),
        Text(text, style: const TextStyle(color: Colors.white70, fontSize: 10)),
      ],
    );
  }
}