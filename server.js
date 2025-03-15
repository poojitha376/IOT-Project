require("dotenv").config();
const express = require("express");
const mongoose = require("mongoose");
const cors = require("cors");

// Initialize Express
const app = express();
app.use(cors());
app.use(express.json());

// Connect to MongoDB
mongoose.connect(process.env.MONGO_URI, {
    useNewUrlParser: true,
    useUnifiedTopology: true
}).then(() => console.log("MongoDB Connected"))
  .catch(err => console.log(err));

// Define Patient Schema
const patientSchema = new mongoose.Schema({
    name: String,
    height: Number,
    weight: Number,
    address: String,
    doctor: String,
    hospital: String,
    emergency: String,
    email: String,
    password: String
});

const Patient = mongoose.model("Patient", patientSchema);

// API Routes

// Add a new patient
app.post("/patients", async (req, res) => {
    try {
        const newPatient = new Patient(req.body);
        await newPatient.save();
        res.status(201).json(newPatient);
    } catch (err) {
        res.status(400).json({ error: err.message });
    }
});

// Get all patients
app.get("/patients", async (req, res) => {
    try {
        const patients = await Patient.find();
        res.json(patients);
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

//Handle logins
app.post("/login", async (req, res) => {
    const { name, password } = req.body;

    try {
        const patient = await Patient.findOne({ name });

        if (!patient || patient.password !== password) {
            return res.status(401).json({ error: "Invalid name or password" });
        }

        res.json(patient); // Send patient data on successful login
    } catch (err) {
        res.status(500).json({ error: "Server error" });
    }
});

// Start server
const PORT = process.env.PORT || 5000;
app.listen(PORT, () => console.log(`Server running on port ${PORT}`));
