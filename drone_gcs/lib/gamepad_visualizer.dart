import 'package:flutter/material.dart';
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
        padding: const EdgeInsets.all(4.0),
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
                    // Cập nhật: Truyền theo kích thước tham chiếu (ví dụ: w * 0.08 cho chiều rộng trigger)
                    _placeAt(w, h, 0.22, 0.15, _buildTriggerBox("LT", lt, w * 0.1)),
                    _placeAt(w, h, 0.78, 0.15, _buildTriggerBox("RT", rt, w * 0.1)),
                    _placeAt(w, h, 0.22, 0.30, _buildBumperBox("LB", lb > 0.5, w * 0.15)),
                    _placeAt(w, h, 0.78, 0.30, _buildBumperBox("RB", rb > 0.5, w * 0.15)),
                    _placeAt(w, h, 0.22, 0.55, _buildStick(w * 0.16, stickLeftX, stickLeftY)),
                    _placeAt(w, h, 0.78, 0.55, _buildActionButtons(w * 0.2, btnA, btnB, btnX, btnY)),
                    _placeAt(w, h, 0.38, 0.80, _buildDPad(w * 0.14, dpadUp, dpadDown, dpadLeft, dpadRight)),
                    _placeAt(w, h, 0.62, 0.80, _buildStick(w * 0.16, stickRightX, stickRightY)),
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

  Widget _buildTriggerBox(String label, double intensity, double baseW) {
    intensity = intensity.clamp(0.0, 1.0);
    double baseH = baseW * 0.6; // Chiều cao cơ bản
    return Container(
      width: baseW,
      height: baseH + (intensity * baseW * 0.4), // Độ phản hồi tỷ lệ thuận với chiều rộng
      decoration: BoxDecoration(
        color: intensity > 0.1 ? Colors.cyan.withOpacity(intensity) : Colors.transparent,
        border: Border.all(color: Colors.white70, width: 2),
        borderRadius: const BorderRadius.vertical(top: Radius.circular(8)),
        boxShadow: intensity > 0.1 ? [BoxShadow(color: Colors.cyan, blurRadius: 10 * intensity)] : [],
      ),
      alignment: Alignment.center,
      child: Text(label, style: TextStyle(fontSize: baseW * 0.3, color: Colors.white, fontWeight: FontWeight.bold)),
    );
  }

  Widget _buildBumperBox(String label, bool isPressed, double width) {
    return Container(
      width: width,
      height: width * 0.35, 
      decoration: BoxDecoration(
        color: isPressed ? Colors.cyan : Colors.transparent,
        border: Border.all(color: Colors.white70, width: 2),
        borderRadius: BorderRadius.circular(6),
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
    double btnSize = size * 0.4;
    return SizedBox(
      width: size,
      height: size,
      child: Stack(
        children: [
          Positioned(top: 0, left: (size - btnSize) / 2, child: _buildCircleBtn("Y", y, Colors.amber, btnSize)),
          Positioned(bottom: 0, left: (size - btnSize) / 2, child: _buildCircleBtn("A", a, Colors.green, btnSize)),
          Positioned(top: (size - btnSize) / 2, left: 0, child: _buildCircleBtn("X", x, Colors.blue, btnSize)),
          Positioned(top: (size - btnSize) / 2, right: 0, child: _buildCircleBtn("B", b, Colors.red, btnSize)),
        ],
      ),
    );
  }

  Widget _buildCircleBtn(String label, bool isPressed, Color activeColor, double btnSize) {
    return Container(
      width: btnSize,
      height: btnSize,
      decoration: BoxDecoration(
        color: isPressed ? activeColor : Colors.transparent,
        shape: BoxShape.circle,
        border: Border.all(color: isPressed ? Colors.white : Colors.white54, width: 1.5),
        boxShadow: isPressed ? [BoxShadow(color: activeColor, blurRadius: 10)] : [],
      ),
      alignment: Alignment.center,
      child: Text(label, style: TextStyle(fontSize: btnSize * 0.5, color: Colors.white, fontWeight: FontWeight.bold)),
    );
  }
}