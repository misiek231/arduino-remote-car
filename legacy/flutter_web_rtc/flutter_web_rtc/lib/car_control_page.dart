import 'package:car_steer/car_control_model.dart';
import 'package:flutter/material.dart';
import 'package:flutter_joystick/flutter_joystick.dart';
import 'package:flutter_webrtc/flutter_webrtc.dart';

class CarControlPage extends StatefulWidget {
  const CarControlPage({super.key});

  @override
  CarControlPageState createState() => CarControlPageState();
}

class CarControlPageState extends State<CarControlPage> {
  late CarControlModel _carControlModel;

  @override
  void initState() {
    super.initState();
    _carControlModel = CarControlModel();
    _carControlModel.initialize();
    _carControlModel.startSendingCommands();
  }

  @override
  void dispose() {
    _carControlModel.dispose();
    super.dispose();
  }

  Widget _buildGearButton(String label, String gearChar) {
    final isSelected = _carControlModel.selectedGear == gearChar;
    return ElevatedButton(
      onPressed:
          () => setState(() {
            _carControlModel.updateGear(gearChar);
          }),
      style: ElevatedButton.styleFrom(
        backgroundColor: isSelected ? Colors.blue : Colors.grey[700],
        padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
      ),
      child: Text(
        label,
        style: const TextStyle(fontSize: 18, color: Colors.white),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Stack(
        children: [
          // Video stream view
          Positioned.fill(child: RTCVideoView(_carControlModel.localRenderer)),

          // Joystick and control areas
          Row(
            children: [
              Expanded(
                flex: 1,
                child: Container(
                  color: Colors.grey[300]?.withValues(alpha: 0.1),
                  child: JoystickArea(
                    mode: JoystickMode.horizontal,
                    listener: (d) => _carControlModel.updateSteering(d.x),
                  ),
                ),
              ),
              Expanded(
                flex: 1,
                child: Column(
                  children: [
                    Expanded(
                      flex: 1,
                      child: GestureDetector(
                        onTapDown:
                            (_) => _carControlModel.updateAcceleration(
                              'w',
                            ), // Decelerate
                        onTapUp:
                            (_) => _carControlModel.updateAcceleration(
                              'c',
                            ), // Stop acceleration
                        child: Container(
                          color: Colors.white.withValues(alpha: 0.1),
                          child: Center(
                            child: Text(
                              "Decelerate",
                              style: TextStyle(fontSize: 16),
                            ),
                          ),
                        ),
                      ),
                    ),
                    Expanded(
                      flex: 1,
                      child: GestureDetector(
                        onTapDown:
                            (_) => _carControlModel.updateAcceleration(
                              "s",
                            ), // Accelerate
                        onTapUp:
                            (_) => _carControlModel.updateAcceleration(
                              'c',
                            ), // Stop acceleration
                        child: Container(
                          color: Colors.white.withValues(alpha: 0.1),
                          child: Center(
                            child: Text(
                              "Accelerate",
                              style: TextStyle(fontSize: 16),
                            ),
                          ),
                        ),
                      ),
                    ),
                  ],
                ),
              ),
            ],
          ),

          // Gear buttons (Positioned in top-left corner)
          Positioned(
            top: 20,
            left: 20,
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                _buildGearButton('1', 'v'),
                SizedBox(height: 8),
                _buildGearButton('2', 'b'),
                SizedBox(height: 8),
                _buildGearButton('N', 'n'),
              ],
            ),
          ),
        ],
      ),
    );
  }
}
