require("dotenv").config(); // Load environment variables from .env

const express = require("express");
const mongoose = require("mongoose");
const cors = require("cors");

const app = express();
app.use(express.json());
app.use(cors());

// Connect to MongoDB using .env variable
mongoose.connect(process.env.MONGO_URI, {
    useNewUrlParser: true,
    useUnifiedTopology: true,
}).then(() => console.log("Connected to MongoDB"))
  .catch((err) => console.error("MongoDB Connection Error:", err));


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

// Route to register a patient
app.post("/patients", async (req, res) => {
    try {
        const patient = new Patient(req.body);
        await patient.save();
        res.json({ message: "Patient Registered!" });
    } catch (error) {
        res.status(500).json({ error: "Error saving patient" });
    }
});

// Route to login a patient
app.post("/login", async (req, res) => {
    try {
        const { name, password } = req.body;
        const patient = await Patient.findOne({ name, password });

        if (!patient) {
            return res.status(401).json({ error: "Invalid credentials" });
        }

        res.json(patient);
    } catch (error) {
        res.status(500).json({ error: "Login failed" });
    }
});

// ==========================
// Medication Reminder Schema
// ==========================
const medicationSchema = new mongoose.Schema({
    name: String,
    dosage: Number,
    times: [String]
});

const Medication = mongoose.model("Medication", medicationSchema);

// Route to add medication
app.post("/medications", async (req, res) => {
    try {
        const med = new Medication(req.body);
        await med.save();
        res.json({ message: "Medication Added!" });
    } catch (error) {
        res.status(500).json({ error: "Error saving medication" });
    }
});

// Route to fetch medications
app.get("/medications", async (req, res) => {
    try {
        const meds = await Medication.find();
        res.json(meds);
    } catch (error) {
        res.status(500).json({ error: "Error fetching medications" });
    }
});


app.delete("/medications/:name", async (req, res) => {
    try {
        const result = await Medication.findOneAndDelete({ name: req.params.name });

        if (!result) {
            return res.status(404).json({ error: "Medication not found" });
        }

        res.json({ message: `Medication '${req.params.name}' deleted successfully!` });
    } catch (error) {
        res.status(500).json({ error: "Error deleting medication" });
    }
});

// ==========================
// Start Server
// ==========================
const PORT = process.env.PORT || 5001;
app.listen(PORT, () => console.log(`Server running on port ${PORT}`));

