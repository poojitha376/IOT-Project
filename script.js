const THINGSPEAK_CHANNEL_ID = "2876560"; 
const THINGSPEAK_API_KEY = "RZDTNUJBBM942BGY"; 
const THINGSPEAK_URL = `https://api.thingspeak.com/channels/${THINGSPEAK_CHANNEL_ID}/feeds/last.json?api_key=${THINGSPEAK_API_KEY}`;

const sensorMapping = {
    temp: "field1",
    heart: "field2",
    spo2: "field3",
    pulse: "field4",
    breath: "field5",
    fall: "field6"
};

// Fetch ThingSpeak data once & update all sensors
async function fetchSensorData(sensorType) {
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

        // ✅ Correct way to access sensor data
        const sensorValue = data[field]; // Get the field value

        console.log(`Sensor (${sensorType}):`, sensorValue);

        // Show "No data" if value is null
        document.getElementById(`${sensorType}Value`).innerText = sensorValue ?? "No data";

        // Highlight the selected sensor row
        showSensor(sensorType);
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

// Fetch data when dashboard loads
document.addEventListener("DOMContentLoaded", () => {
    Object.keys(sensorMapping).forEach(fetchSensorData);
});
