import 'package:car_steer/car_control_page.dart';
import 'package:flutter/material.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Car Control',
      theme: ThemeData(primarySwatch: Colors.blue),
      home: const CarControlPage(),
    );
  }
}
