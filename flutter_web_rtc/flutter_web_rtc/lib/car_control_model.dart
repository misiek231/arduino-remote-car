import 'dart:async';
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
  Timer? _commandTimer;

  int steerAngle = 90;
  String accHold = "c"; // 0 is default - stopped
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

  void startSendingCommands() {
    const duration = Duration(milliseconds: 20); // 20ms interval
    _commandTimer = Timer.periodic(duration, (timer) {
      _udpService.sendCommand(steerAngle, accHold, selectedGear);
    });
  }

  // Update the current gear and send the command to the car
  void updateGear(String gear) {
    selectedGear = gear;
  }

  // Change the steering angle based on joystick input
  void updateSteering(double x) {
    steerAngle = mapValue(-x, -1, 1, steerMinAngle, steerMaxAngle);
  }

  // Acceleration change
  void updateAcceleration(String acc) {
    accHold = acc;
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
    _commandTimer?.cancel();
    _udpService.close();
  }
}
