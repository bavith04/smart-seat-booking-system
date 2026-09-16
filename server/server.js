const express = require('express');
const http = require('http');
const { Server } = require('socket.io');
const cors = require('cors');
const path = require('path');

const app = express();
const server = http.createServer(app);
const io = new Server(server, {
  cors: {
    origin: '*',
    methods: ['GET', 'POST']
  }
});

const PORT = process.env.PORT || 3000;

app.use(cors());
app.use(express.json());
// Serve static frontend dashboard from public folder
app.use(express.static(path.join(__dirname, '../public')));

// --- REAL-TIME SYSTEM STATE ---
let seatData = {
  seats: [
    { id: 1, name: 'Seat 1', status: 'available', pressure: 120 },
    { id: 2, name: 'Seat 2', status: 'available', pressure: 90 },
    { id: 3, name: 'Seat 3', status: 'available', pressure: 110 },
    { id: 4, name: 'Seat 4', status: 'available', pressure: 105 }
  ],
  availableCount: 4,
  totalSeats: 4,
  lastUpdated: new Date().toISOString()
};

// Activity log array
let activityLogs = [
  { id: 1, type: 'info', message: 'System initialized and connected.', timestamp: new Date().toLocaleTimeString() }
];

// Helper to calculate total available seats
function calculateAvailable() {
  return seatData.seats.filter(s => s.status === 'available').length;
}

// Add event to activity log & broadcast to web
function addLog(type, message) {
  const logEntry = {
    id: Date.now(),
    type, // 'info', 'success', 'warning', 'danger'
    message,
    timestamp: new Date().toLocaleTimeString()
  };
  activityLogs.unshift(logEntry);
  if (activityLogs.length > 30) activityLogs.pop(); // Keep last 30 logs

  io.emit('new_log', logEntry);
}

// --- REST API ENDPOINTS ---

// GET current seat state
app.get('/api/seats', (req, res) => {
  res.json({
    success: true,
    data: seatData,
    logs: activityLogs
  });
});

// POST seat updates from ESP32 telemetry
app.post('/api/seats/update', (req, res) => {
  const { s1, s2, s3, s4 } = req.body;
  
  if (s1 !== undefined) seatData.seats[0].status = s1 ? 'occupied' : 'available';
  if (s2 !== undefined) seatData.seats[1].status = s2 ? 'occupied' : 'available';
  if (s3 !== undefined) seatData.seats[2].status = s3 ? 'occupied' : 'available';
  if (s4 !== undefined) seatData.seats[3].status = s4 ? 'occupied' : 'available';

  seatData.availableCount = calculateAvailable();
  seatData.lastUpdated = new Date().toISOString();

  // Broadcast real-time seat status to all connected web dashboards
  io.emit('seat_update', seatData);
  addLog('info', `ESP32 Sensor Telemetry Updated. Seats Available: ${seatData.availableCount}/4`);

  res.json({ success: true, available: seatData.availableCount });
});

// POST keypad booking request from ESP32
app.post('/api/seats/keypad', (req, res) => {
  const { requestedSeats } = req.body;
  const currentAvailable = calculateAvailable();

  let success = false;
  let msg = '';

  if (currentAvailable >= requestedSeats) {
    success = true;
    msg = `Keypad Request Approved: ${requestedSeats} seat(s) booked successfully.`;
    addLog('success', msg);
  } else {
    success = false;
    msg = `Keypad Request Rejected: Requested ${requestedSeats} seat(s), but only ${currentAvailable} available.`;
    addLog('danger', msg);
  }

  // Broadcast event to web dashboard
  io.emit('booking_result', { source: 'Keypad', requestedSeats, success, message: msg });
  res.json({ success, message: msg, available: currentAvailable });
});

// POST booking request from Web Dashboard
app.post('/api/book', (req, res) => {
  const { requestedSeats } = req.body;
  const currentAvailable = calculateAvailable();

  if (requestedSeats < 1 || requestedSeats > 4) {
    return res.status(400).json({ success: false, message: 'Invalid seat request number.' });
  }

  if (currentAvailable >= requestedSeats) {
    const msg = `Web Booking Confirmed: Reserved ${requestedSeats} seat(s).`;
    addLog('success', msg);
    io.emit('booking_result', { source: 'Web Dashboard', requestedSeats, success: true, message: msg });
    return res.json({ success: true, message: msg, available: currentAvailable });
  } else {
    const msg = `Web Booking Failed: Only ${currentAvailable} seat(s) available.`;
    addLog('danger', msg);
    io.emit('booking_result', { source: 'Web Dashboard', requestedSeats, success: false, message: msg });
    return res.status(400).json({ success: false, message: msg, available: currentAvailable });
  }
});

// --- SOCKET.IO REALTIME CONNECTIONS ---
io.on('connection', (socket) => {
  console.log(`🟢 New Web Dashboard Client Connected: ${socket.id}`);
  
  // Send initial state to newly connected web client
  socket.emit('initial_state', { seatData, logs: activityLogs });

  socket.on('disconnect', () => {
    console.log(`🔴 Web Dashboard Client Disconnected: ${socket.id}`);
  });
});

server.listen(PORT, () => {
  console.log(`=================================================`);
  console.log(`🚀 Smart Seat Booking Server Running on Port ${PORT}`);
  console.log(`🌐 Dashboard URL: http://localhost:${PORT}`);
  console.log(`=================================================`);
});
