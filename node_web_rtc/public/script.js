"use strict";
const videoElement = document.getElementById('video');
if (videoElement == null)
    throw new Error('videoElement is null');
const signalingServer = new WebSocket('ws://192.168.0.213:8080'); // Replace with your signaling server IP
let peerConnection;
signalingServer.onmessage = (message) => {
    const data = JSON.parse(message.data);
    console.log(data);
    if (data.type === 'offer') {
        peerConnection = new RTCPeerConnection();
        peerConnection.setRemoteDescription(new RTCSessionDescription(data));
        peerConnection.createAnswer().then(answer => {
            peerConnection.setLocalDescription(answer);
            signalingServer.send(JSON.stringify({ type: 'answer', sdp: answer.sdp, sdpType: answer.type }));
        });
        peerConnection.ontrack = (event) => {
            console.log("ontrack");
            console.log(event);
            videoElement.srcObject = event.streams[0];
            videoElement.play().catch(e => console.error("Playback failed:", e));
        };
        peerConnection.onicecandidate = (event) => {
            if (event.candidate) {
                signalingServer.send(JSON.stringify({
                    type: 'candidate',
                    candidate: event.candidate,
                }));
            }
        };
    }
    else if (data.type === 'answer') {
        peerConnection.setRemoteDescription(new RTCSessionDescription(data));
    }
    else if (data.type === 'candidate') {
        peerConnection.addIceCandidate(new RTCIceCandidate(data.candidate));
    }
};
signalingServer.onopen = async () => {
    console.log('Connected to signaling server');
    peerConnection = new RTCPeerConnection();
    peerConnection.ontrack = (event) => {
        console.log("ontrack");
        console.log(event);
        const videoTrack = peerConnection.getReceivers().find(r => r.track.kind === "video")?.track;
        if (videoTrack) {
            videoTrack.applyConstraints({
                frameRate: { max: 30 }, // Lower FPS if needed
                width: { ideal: 1280 }, // Adjust resolution
                height: { ideal: 720 }
            });
        }
        videoElement.srcObject = event.streams[0];
        videoElement.play().catch(e => console.error("Playback failed:", e));
    };
    peerConnection.oniceconnectionstatechange = () => {
        console.log("ICE Connection State:", peerConnection.iceConnectionState);
    };
    peerConnection.onconnectionstatechange = () => {
        console.log("Connection State:", peerConnection.connectionState);
    };
    peerConnection.onicecandidate = (event) => {
        if (event.candidate) {
            console.log("ICE Candidate Found:", event.candidate);
            signalingServer.send(JSON.stringify({ type: 'candidate', candidate: event.candidate }));
        }
        else {
            console.log("ICE Candidate Gathering Completed.");
        }
    };
    peerConnection.onicegatheringstatechange = () => {
        console.log("ICE Gathering State Changed:", peerConnection.iceGatheringState);
    };
    try { // 💡 Create an Offer, explicitly requesting a video stream
        const offer = await peerConnection.createOffer({
            offerToReceiveVideo: true, // ✅ This ensures video is included in SDP
            offerToReceiveAudio: false
        });
        console.log("Offer Created:", offer);
        await peerConnection.setLocalDescription(offer);
        console.log("Local Description Set:", peerConnection.localDescription);
        signalingServer.send(JSON.stringify({
            type: 'offer',
            sdp: peerConnection.localDescription?.sdp,
            sdpType: peerConnection.localDescription?.type
        }));
    }
    catch (error) {
        console.error("Offer creation failed:", error);
    }
    console.log('Connected to signaling server');
};
document.addEventListener("click", () => {
    if (videoElement.paused) {
        videoElement.play().catch(e => console.error("Playback failed:", e));
    }
}, { once: true }); // Only runs once
