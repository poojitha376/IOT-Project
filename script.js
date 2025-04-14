const THINGSPEAK_CHANNEL_ID = "2876560";
const THINGSPEAK_API_KEY = "RZDTNUJBBM942BGY";
const THINGSPEAK_URL = `https://api.thingspeak.com/channels/${THINGSPEAK_CHANNEL_ID}/feeds/last.json?api_key=${THINGSPEAK_API_KEY}`;

const sensorMapping = {
    temp: "field1",
    spo2: "field2",
    heart: "field3",
    pulse: "field4",
    fall: "field5",
    touch: "field6",
};

// Fetch ThingSpeak data & update sensors
async function fetchSensorData(sensorType) {

    document.getElementById("reportSection").style.display = "block"; // Make it visible

    try {
        console.log("Fetching data from:", THINGSPEAK_URL);

        const response = await fetch(THINGSPEAK_URL);
        const data = await response.json();

        console.log("Fetched Data:", data);

        if (!data || !data.created_at) {
            console.error("Invalid response from ThingSpeak");
            return;
        }

        // Find the correct field from the mapping
        const field = sensorMapping[sensorType];
        if (!field) {
            console.error("Invalid sensor type:", sensorType);
            return;
        }

        // Get the field value
        const sensorValue = data[field];

        console.log(`Sensor (${sensorType}):`, sensorValue);

        // Show "No data" if value is null
        document.getElementById(`${sensorType}Value`).innerText = sensorValue ?? "No data";

        // Highlight the selected sensor row
        showSensor(sensorType);

        // Re-run analysis after fetching new data
        analyzeHealthStatus();
        sendDataToServer();
    } catch (error) {
        console.error("Error fetching data:", error);
    }
}

// Show sensor data (Highlight instead of hiding)
function showSensor(sensorId) {
    // Make sure the table is visible
    document.getElementById("sensorTable").style.display = "table";

    // Remove highlighting from all rows
    document.querySelectorAll("#sensorTable tr").forEach(row => row.classList.remove("highlight"));

    // Highlight the selected sensor row
    const selectedRow = document.getElementById(sensorId);
    if (selectedRow) {
        selectedRow.classList.add("highlight");
    } else {
        console.error("Invalid sensor ID:", sensorId);
    }
}

// Analyze patient health based on sensor data
function analyzeHealthStatus() {
    let temp = parseFloat(document.getElementById("tempValue").textContent) || 0;
    let spo2 = parseFloat(document.getElementById("spo2Value").textContent) || 0;
    let heart = parseFloat(document.getElementById("heartValue").textContent) || 0;
    let fall = parseFloat(document.getElementById("fallValue").textContent) || 0;
    let touch = parseFloat(document.getElementById("touchValue").textContent) || 0; 

    let statusClass = "normal"; // Default status
    let count = { mild: 0, moderate: 0, severe: 0, critical: 0 };
    let issues = []; // Collect all out-of-range conditions

    function addIssue(icon, level, message, category) {
        issues.push({ icon, level, message });
        count[category]++;
        if (["critical", "severe", "moderate", "mild"].includes(level.toLowerCase())) {
            statusClass = level.toLowerCase();
        }
    }

    // ⚫ CRITICAL CONDITIONS (BLACK)
    if (temp > 40) addIssue("⚫", "CRITICAL", `Temperature extreme (${temp.toFixed(1)}°C)!`, "critical");
    if (spo2 < 85) addIssue("⚫", "CRITICAL", `Oxygen dangerously low (${spo2}%)!`, "critical");
    if (heart > 150 || heart < 40) addIssue("⚫", "CRITICAL", `Abnormal heart rate (${heart} BPM)!`, "critical");
    if (touch < 8 || touch > 40) addIssue("⚫", "CRITICAL", `Irregular touching (${touch} BPM)!`, "critical");
    if (fall >= 8) addIssue("⚫", "CRITICAL", "Fall detected! Immediate medical attention required.", "critical");

    // 🔴 SEVERE CONDITIONS (RED)
    if (temp >= 39 && temp <= 40) addIssue("🔴", "SEVERE", `High fever detected (${temp.toFixed(1)}°C).`, "severe");
    if (spo2 >= 85 && spo2 < 88) addIssue("🔴", "SEVERE", `Very low oxygen level (${spo2}%).`, "severe");
    if (heart >= 130 && heart < 150) addIssue("🔴", "SEVERE", `Rapid heart rate (${heart} BPM).`, "severe");
    if (touch >= 30 && touch < 40) addIssue("🔴", "SEVERE", `Fast touching (${touch} BPM).`, "severe");

    // 🟠 MODERATE CONDITIONS (ORANGE)
    if (temp >= 38 && temp < 39) addIssue("🟠", "MODERATE", `Fever detected (${temp.toFixed(1)}°C).`, "moderate");
    if (spo2 >= 88 && spo2 < 91) addIssue("🟠", "MODERATE", `Oxygen level slightly low (${spo2}%).`, "moderate");
    if (heart >= 110 && heart < 130) addIssue("🟠", "MODERATE", `Increased heart rate (${heart} BPM).`, "moderate");
    if (touch >= 24 && touch < 30) addIssue("🟠", "MODERATE", `touching rate elevated (${touch} BPM).`, "moderate");

    // 🟡 MILD CONDITIONS (YELLOW)
    if (temp >= 37.3 && temp < 38) addIssue("🟡", "MILD", `Slight fever (${temp.toFixed(1)}°C).`, "mild");
    if (spo2 >= 92 && spo2 < 95) addIssue("🟡", "MILD", `Oxygen level slightly reduced (${spo2}%).`, "mild");
    if (heart >= 100 && heart < 110) addIssue("🟡", "MILD", `Mild tachycardia (${heart} BPM).`, "mild");
    if (touch >= 20 && touch < 24) addIssue("🟡", "MILD", `Slightly high touching rate (${touch} BPM).`, "mild");

    // 🟢 NORMAL CONDITION (GREEN)
    if (issues.length === 0) {
        issues.push({ icon: "🟢", level: "NORMAL", message: "Patient is stable. No medical concern." });
    }

    // **Update Report Table**
    let reportTable = document.getElementById("reportTableBody");
    reportTable.innerHTML = ""; // Clear previous table rows

    issues.forEach(issue => {
        let row = document.createElement("tr");
        row.innerHTML = `<td>${issue.icon}</td><td>${issue.level}</td><td>${issue.message}</td>`;
        reportTable.appendChild(row);
    });

    // Update Status Box
    let statusBox = document.getElementById("statusBox");
    statusBox.textContent = statusClass.toUpperCase();
    statusBox.className = `status-box ${statusClass}`;

    // **Possible Causes Section**
    let possibleCauses = document.getElementById("possibleCausesSection");
    let possibleCausesList = document.getElementById("possibleCausesList");
    possibleCausesList.innerHTML = ""; // Clear previous entries

    let causes = [];
    if (count.critical > count.severe && count.critical > count.moderate && count.critical > count.mild) {
        causes = [
            "Severe infections (Sepsis, Septic Shock)",
            "Heart attack, Stroke, Organ failure",
            "Severe asthma attack, Respiratory arrest",
            "Heat stroke, Brain damage risk"
        ];
    } else if (count.severe > count.moderate && count.severe > count.mild) {
        causes = [
            "Severe infections (pneumonia, COVID-19, sepsis)",
            "Heart issues (hypertensive crisis, heart failure)",
            "Respiratory distress (asthma attack, severe hypoxia)",
            "Extreme stress response (panic attack, medical emergency)"
        ];
    } else if (count.moderate > count.mild) {
        causes = [
            "Infection (flu, mild pneumonia, fever)",
            "Respiratory Issues (asthma, mild COVID-19)",
            "Hypertension (rising BP can cause long-term damage)",
            "High stress or anxiety attacks"
        ];
    } else if (count.mild > 0) {
        causes = [
            "Dehydration",
            "Anxiety or stress",
            "Mild fever (common cold, flu)",
            "Lack of sleep",
            "High caffeine intake"
        ];
    }

    if (causes.length > 0) {
        possibleCauses.style.display = "block";
        causes.forEach(cause => {
            let div = document.createElement("div"); // Create a div instead of <li>
            div.innerHTML = cause;
            div.classList.add("cause-item"); // Apply CSS styling
            possibleCausesList.appendChild(div);
        });
    } else {
        possibleCauses.style.display = "none";
    }
}

async function sendDataToServer() {
    let sensorData = {
        temperature: parseFloat(document.getElementById("tempValue").textContent) || 0,
        spo2: parseFloat(document.getElementById("spo2Value").textContent) || 0,
        heartRate: parseFloat(document.getElementById("heartValue").textContent) || 0,
        pulse: parseFloat(document.getElementById("pulseValue").textContent) || 0,
        fall: parseFloat(document.getElementById("fallValue").textContent) || 0,
        touch: parseFloat(document.getElementById("touchValue").textContent) || 0,
        analysis: document.getElementById("reportText").innerText,
    };

    try {
        const response = await fetch("http://localhost:5001/data", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(sensorData),
        });

        const result = await response.json();
        console.log("Data sent to server:", result);
    } catch (error) {
        console.error("Error sending data:", error);
    }
}

async function addMedication() {
    const name = document.getElementById("medName").value.trim();
    const dosage = parseInt(document.getElementById("dosage").value.trim());

    if (!name || dosage < 1) {
        alert("Enter valid medication details!");
        return;
    }

    let times = [];
    for (let i = 0; i < dosage; i++) {
        const time = prompt(`Enter time ${i + 1} (HH:MM 24-hour format)`);
        if (time) times.push(time);
    }

    const medication = { name, dosage, times };

    // Store in MongoDB
    await fetch("http://localhost:5001/medications", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(medication)
    });

    alert("Medication Added!");
    loadMedications();
}

async function loadMedications() {
    const res = await fetch("http://localhost:5001/medications");
    const meds = await res.json();
    document.getElementById("medList").innerHTML = meds.map(med =>
        `<li>${med.name}`).join("");
}

function checkReminders() {
    const now = new Date();
    const currentTime = now.getHours().toString().padStart(2, "0") + ":" + now.getMinutes().toString().padStart(2, "0");

    fetch("http://localhost:5001/medications")
        .then(res => res.json())
        .then(meds => {
            meds.forEach(med => {
                if (med.times.includes(currentTime)) {
                    new Notification("Medication Reminder", { body: `Time to take ${med.name}` });
                }
            });
        });
}

if (Notification.permission !== "granted") Notification.requestPermission();

setInterval(checkReminders, 60000); // Check every minute
document.addEventListener("DOMContentLoaded", loadMedications);


// Fetch data when dashboard loads
document.addEventListener("DOMContentLoaded", () => {
    Object.keys(sensorMapping).forEach(fetchSensorData);
});
