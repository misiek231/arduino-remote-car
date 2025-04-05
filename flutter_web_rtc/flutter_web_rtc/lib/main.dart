import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:flutter_webrtc/flutter_webrtc.dart';
import 'package:web_socket_channel/web_socket_channel.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'WebRTC Video Streaming',
      home: VideoStreamingScreen(),
    );
  }
}

class VideoStreamingScreen extends StatefulWidget {
  const VideoStreamingScreen({super.key});

  @override
  VideoStreamingScreenState createState() => VideoStreamingScreenState();
}

class VideoStreamingScreenState extends State<VideoStreamingScreen> {
  late RTCPeerConnection _peerConnection;
  late MediaStream _localStream;
  late RTCVideoRenderer _localRenderer;

  late WebSocketChannel _channel; // Use the signaling server's IP

  @override
  void initState() {
    super.initState();
    _initializeWebRTC();
  }

  @override
  void dispose() {
    _peerConnection.close();
    _localRenderer.dispose();
    _localStream.dispose();
    super.dispose();
  }

  Future<void> _initializeWebRTC() async {
    _channel = WebSocketChannel.connect(Uri.parse("ws://192.168.0.213:8080"));
    _localRenderer = RTCVideoRenderer();
    _localRenderer.initialize();

    // No ICE servers since it's a local network
    final Map<String, dynamic> configuration = {};

    _peerConnection = await createPeerConnection(configuration);

    _localStream = await navigator.mediaDevices.getUserMedia({
      'video': {'facingMode': 'environment'},
      'audio': false, // Disable audio for now
    });

    _localRenderer.srcObject = _localStream;

    _localStream.getTracks().forEach((track) {
      _peerConnection.addTrack(track, _localStream);
    });

    _peerConnection.onIceCandidate = (RTCIceCandidate candidate) {
      _channel.sink.add(
        jsonEncode({'type': 'candidate', 'candidate': candidate.toMap()}),
      );
    };

    RTCSessionDescription offer = await _peerConnection.createOffer();
    await _peerConnection.setLocalDescription(offer);

    _channel.sink.add(
      jsonEncode({'type': 'offer', 'sdp': offer.sdp, 'sdpType': offer.type}),
    );

    _channel.stream.listen((message) async {
      Map<String, dynamic> data = jsonDecode(message);

      if (data['type'] == 'answer') {
        await _peerConnection.setRemoteDescription(
          RTCSessionDescription(data['sdp'], data['sdpType']),
        );
      } else if (data['type'] == 'offer') {
        print("recived offer");
        _peerConnection = await createPeerConnection(configuration);

        _peerConnection.onIceCandidate = (RTCIceCandidate candidate) {
          _channel.sink.add(
            jsonEncode({'type': 'candidate', 'candidate': candidate.toMap()}),
          );
        };

        _localStream.getTracks().forEach((track) {
          _peerConnection.addTrack(track, _localStream);
        });

        await _peerConnection.setRemoteDescription(
          RTCSessionDescription(data['sdp'], data['sdpType']),
        );

        RTCSessionDescription answer = await _peerConnection.createAnswer();
        _peerConnection.setLocalDescription(answer);

        _channel.sink.add(
          jsonEncode({
            'type': 'answer',
            'sdp': answer.sdp,
            'sdpType': answer.type,
          }),
        );
        print("end offer");
      } else if (data['type'] == 'candidate') {
        _peerConnection.addCandidate(
          RTCIceCandidate(
            data['candidate']['candidate'],
            data['candidate']['sdpMid'],
            data['candidate']['sdpMLineIndex'],
          ),
        );
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text("WebRTC Video Streaming")),
      body: Column(
        children: <Widget>[
          Expanded(
            child: RTCVideoView(
              _localRenderer,
              mirror: true, // Mirror the video for the sender
            ),
          ),
          SizedBox(height: 20),
          ElevatedButton(
            onPressed: _initializeWebRTC,
            child: Text("Start Streaming"),
          ),
        ],
      ),
    );
  }
}
