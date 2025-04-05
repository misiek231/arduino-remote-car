import 'dart:convert';
import 'package:flutter_webrtc/flutter_webrtc.dart';
import 'package:web_socket_channel/web_socket_channel.dart';
import 'config.dart';
import 'car_control_service.dart';

class CarControlModel {
  final UdpService _udpService = UdpService();
  late RTCVideoRenderer localRenderer;
  late RTCPeerConnection _peerConnection;
  late WebSocketChannel _channel;

  int accHold = 0; // 0 is default - stopped
  int steerAngle = 90;
  String selectedGear = 'n'; // default to Neutral

  // Initialize WebRTC and UDP connections
  Future<void> initialize() async {
    await _initializeWebRTC();
    await _udpService.initialize();
  }

  // Initialize WebRTC and signaling
  Future<void> _initializeWebRTC() async {
    _channel = WebSocketChannel.connect(Uri.parse(webSocketUrl));
    localRenderer = RTCVideoRenderer();
    await localRenderer.initialize();

    final Map<String, dynamic> configuration = {};

    _peerConnection = await createPeerConnection(configuration);

    _peerConnection.onIceCandidate = (RTCIceCandidate candidate) {
      _channel.sink.add(
        jsonEncode({'type': 'candidate', 'candidate': candidate.toMap()}),
      );
    };

    _peerConnection.onTrack = (RTCTrackEvent event) {
      // We can update the renderer directly here to handle the video stream.
    };

    RTCSessionDescription offer = await _peerConnection.createOffer({
      'offerToReceiveVideo': true,
    });

    await _peerConnection.setLocalDescription(offer);

    _channel.sink.add(
      jsonEncode({'type': 'offer', 'sdp': offer.sdp, 'sdpType': offer.type}),
    );
  }

  // Update the current gear and send the command to the car
  void updateGear(String gear) {
    selectedGear = gear;
    _sendCommand();
  }

  // Send the control command to the car
  void _sendCommand() {
    String acc = 'c'; // Default to 'c' for neutral
    if (accHold == 1) {
      acc = 's'; // Accelerate
    } else if (accHold == -1) {
      acc = 'w'; // Decelerate
    }
    _udpService.sendCommand(steerAngle, acc, selectedGear);
  }

  // Change the steering angle based on joystick input
  void updateSteering(double x) {
    steerAngle = mapValue(-x, -1, 1, steerMinAngle, steerMaxAngle);
    _sendCommand();
  }

  // Acceleration change
  void updateAcceleration(int clicked) {
    accHold = clicked;
    _sendCommand();
  }

  int mapValue(
    double x,
    double inMin,
    double inMax,
    double outMin,
    double outMax,
  ) {
    return ((x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin).toInt();
  }

  void dispose() {
    _udpService.close();
  }
}
