const THINGSPEAK_CHANNEL_ID = "2876560";
const THINGSPEAK_API_KEY = "RZDTNUJBBM942BGY";
const THINGSPEAK_URL = `https://api.thingspeak.com/channels/${THINGSPEAK_CHANNEL_ID}/feeds/last.json?api_key=${THINGSPEAK_API_KEY}`;

const sensorMapping = {
    temp: "field1",
    spo2: "field2",
    heart: "field3",
    pulse: "field4",
    fall: "field5",
    breath: "field6",
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
    let breath = parseFloat(document.getElementById("breathValue").textContent) || 0;

    let issues = []; // Collect all out-of-range conditions
    let statusClass = "normal"; // Default to normal (green)

    // ⚫ CRITICAL CONDITIONS (BLACK)
    if (temp > 40) {
        issues.push(`⚠️ CRITICAL: Temperature (${temp}°F) extreme!`);
        statusClass = "critical";
    }
    if (spo2 < 85) {
        issues.push(`⚠️ CRITICAL: Oxygen dangerously low (${spo2}%)!`);
        statusClass = "critical";
    }
    if (heart > 150 || heart < 40) {
        issues.push(`⚠️ CRITICAL: Abnormal heart rate (${heart} BPM)!`);
        statusClass = "critical";
    }
    if (breath < 8 || breath > 40) {
        issues.push(`⚠️ CRITICAL: Irregular breathing (${breath} BPM)!`);
        statusClass = "critical";
    }
    if (fall >= 8) {
        issues.push("🚨 CRITICAL: Fall detected! Immediate medical attention required.");
        statusClass = "critical";
    }

    // 🔴 SEVERE CONDITIONS (RED)
    if (temp >= 39 && temp <= 40) {
        issues.push(`🔴 SEVERE: High fever detected (${temp}°C).`);
        if (statusClass !== "critical") statusClass = "severe";
    }
    if (spo2 >= 85 && spo2 < 88) {
        issues.push(`🔴 SEVERE: Very low oxygen level (${spo2}%).`);
        if (statusClass !== "critical") statusClass = "severe";
    }
    if (heart >= 130 && heart < 150) {
        issues.push(`🔴 SEVERE: Rapid heart rate (${heart} BPM).`);
        if (statusClass !== "critical") statusClass = "severe";
    }
    if (breath >= 30 && breath < 40) {
        issues.push(`🔴 SEVERE: Fast breathing (${breath} BPM).`);
        if (statusClass !== "critical") statusClass = "severe";
    }

    // 🟠 MODERATE CONDITIONS (ORANGE)
    if (temp >= 38 && temp < 39) {
        issues.push(`🟠 MODERATE: Fever detected (${temp}°F).`);
        if (statusClass !== "critical" && statusClass !== "severe") statusClass = "moderate";
    }
    if (spo2 >= 88 && spo2 < 91) {
        issues.push(`🟠 MODERATE: Oxygen level slightly low (${spo2}%).`);
        if (statusClass !== "critical" && statusClass !== "severe") statusClass = "moderate";
    }
    if (heart >= 110 && heart < 130) {
        issues.push(`🟠 MODERATE: Increased heart rate (${heart} BPM).`);
        if (statusClass !== "critical" && statusClass !== "severe") statusClass = "moderate";
    }
    if (breath >= 24 && breath < 30) {
        issues.push(`🟠 MODERATE: Breathing rate elevated (${breath} BPM).`);
        if (statusClass !== "critical" && statusClass !== "severe") statusClass = "moderate";
    }

    // 🟡 MILD CONDITIONS (YELLOW)
    if (temp >= 37.3 && temp < 38) {
        issues.push(`🟡 MILD: Slight fever (${temp}°F).`);
        if (statusClass !== "critical" && statusClass !== "severe" && statusClass !== "moderate") statusClass = "mild";
    }
    if (spo2 >= 92 && spo2 < 95) {
        issues.push(`🟡 MILD: Oxygen level slightly reduced (${spo2}%).`);
        if (statusClass !== "critical" && statusClass !== "severe" && statusClass !== "moderate") statusClass = "mild";
    }
    if (heart >= 100 && heart < 110) {
        issues.push(`🟡 MILD: Mild tachycardia (${heart} BPM).`);
        if (statusClass !== "critical" && statusClass !== "severe" && statusClass !== "moderate") statusClass = "mild";
    }
    if (breath >= 20 && breath < 24) {
        issues.push(`🟡 MILD: Slightly high breathing rate (${breath} BPM).`);
        if (statusClass !== "critical" && statusClass !== "severe" && statusClass !== "moderate") statusClass = "mild";
    }

    // 🟢 NORMAL CONDITION (GREEN)
    if (issues.length === 0) {
        issues.push("🟢 Patient is stable. No medical concern.");
    }

    // Update Report Text
    document.getElementById("reportText").innerHTML = issues.join("<br>");

    // Update Status Box
    let statusBox = document.getElementById("statusBox");
    statusBox.textContent = statusClass.toUpperCase();
    statusBox.className = `status-box ${statusClass}`;
}

// Fetch data when dashboard loads
document.addEventListener("DOMContentLoaded", () => {
    Object.keys(sensorMapping).forEach(fetchSensorData);
});
