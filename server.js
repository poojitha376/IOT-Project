const express = require("express");
const cors = require("cors");

const app = express();
app.use(cors());
app.use(express.json());

// Store patient details & sensor data
let patients = {};
let sensorData = {};

// ✅ API to register a new patient
app.post("/register", (req, res) => {
    const { name, email, password, height, weight, address, doctor, hospital, emergency } = req.body;

    if (!email || !password) {
        return res.status(400).json({ error: "Email and password are required!" });
    }

    if (patients[email]) {
        return res.status(400).json({ error: "Patient already registered!" });
    }

    patients[email] = { name, email, password, height, weight, address, doctor, hospital, emergency };
    sensorData[email] = { temperature: 36.5, heartRate: 72, alert: "", medication: "" };

    res.status(201).json({ message: "Patient registered successfully!", patient: patients[email] });
});

// ✅ API to login a patient
app.post("/login", (req, res) => {
    const { email, password } = req.body;

    if (!patients[email] || patients[email].password !== password) {
        return res.status(401).json({ error: "Invalid email or password!" });
    }

    res.json({ message: "Login successful!", patient: patients[email] });
});

// ✅ API to fetch sensor data for a specific patient
app.get("/data/:email", (req, res) => {
    const email = req.params.email;
    
    if (!sensorData[email]) {
        return res.status(404).json({ error: "Patient not found!" });
    }

    res.json(sensorData[email]);
});

// ✅ API to update sensor data (from ESP32)
app.post("/update/:email", (req, res) => {
    const email = req.params.email;
    const { temperature, heartRate, alert, medication } = req.body;

    if (!sensorData[email]) {
        return res.status(404).json({ error: "Patient not found!" });
    }

    sensorData[email] = { temperature, heartRate, alert, medication };
    res.send("Data updated successfully");
});

// ✅ Debugging: Get all patients
app.get("/patients", (req, res) => {
    res.json(patients);
});

app.listen(3000, () => console.log("✅ Server running on port 3000"));
