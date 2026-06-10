import 'package:flutter/material.dart';

Widget buildSectionHeader(String title, IconData icon) {
  return Padding(
    padding: const EdgeInsets.only(bottom: 12.0),
    child: Row(
      children: [
        Icon(icon, color: Colors.white70, size: 20),
        const SizedBox(width: 8),
        Text(title, style: const TextStyle(color: Colors.white70, fontSize: 14, fontWeight: FontWeight.bold)),
      ],
    ),
  );
}

Widget buildCompactCard(String title, String value, String unit, Color accentColor) {
  return Card(
    color: Colors.blueGrey.shade800,
    elevation: 2,
    margin: EdgeInsets.zero, 
    shape: RoundedRectangleBorder(
      side: BorderSide(color: accentColor.withOpacity(0.5), width: 1.2), 
      borderRadius: BorderRadius.circular(6), 
    ),
    child: Padding(
      padding: const EdgeInsets.all(6.0), 
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        // SỬA Ở ĐÂY: Đổi từ .start sang .center để kéo mọi thứ vào giữa thẻ
        crossAxisAlignment: CrossAxisAlignment.center, 
        children: [
          FittedBox(
            fit: BoxFit.scaleDown, 
            child: Text(title, style: TextStyle(fontSize: 11, color: Colors.blueGrey.shade200)) 
          ),
          const Spacer(),
          FittedBox(
            fit: BoxFit.scaleDown,
            child: Row(
              mainAxisAlignment: MainAxisAlignment.center, 
              crossAxisAlignment: CrossAxisAlignment.center,
             
              textBaseline: TextBaseline.alphabetic, 
              children: [
                Text(value, style: const TextStyle(fontSize: 18, fontWeight: FontWeight.bold, color: Colors.white)), 
                const SizedBox(width: 3),
                Text(unit, style: TextStyle(fontSize: 12, color: accentColor)),
              ],
            ),
          )
        ],
      ),
    ),
  );
}