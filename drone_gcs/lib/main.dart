import 'package:flutter/material.dart';
import 'dashboard_screen.dart';

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