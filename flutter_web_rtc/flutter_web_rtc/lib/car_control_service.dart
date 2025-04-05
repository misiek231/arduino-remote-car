import 'dart:io';
import 'package:udp/udp.dart';
import 'config.dart';

class UdpService {
  UDP? _udp;

  Future<void> initialize() async {
    _udp = await UDP.bind(Endpoint.any(port: Port(udpPort)));
    print("UDP Receiver is ready");
  }

  Future<void> sendCommand(
    int steerAngle,
    String accHold,
    String selectedGear,
  ) async {
    String command = "$steerAngle:$accHold:$selectedGear\n";
    print("Command: $command");

    final data = command.codeUnits;
    final address = Endpoint.unicast(
      InternetAddress(serverIp),
      port: Port(udpPort),
    );
    await _udp?.send(data, address);
  }

  void close() {
    _udp?.close();
  }
}
