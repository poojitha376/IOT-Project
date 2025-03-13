const THINGSPEAK_CHANNEL_ID = " 2876560"; 
const THINGSPEAK_API_KEY = "RZDTNUJBBM942BGY"; 
const THINGSPEAK_URL = `https://thingspeak.mathworks.com/channels/2876560/feeds/last.json`;

const sensorMapping = {
    temp: "field1",
    heart: "field2",
    spo2: "field3",
    pulse: "field4",
    stress: "field5",
    breath: "field6",
    fall: "field7"
};

// Fetch ThingSpeak data once & update all sensors
async function fetchSensorData() {
    try {
        const response = await fetch(THINGSPEAK_URL);
        const data = await response.json();

        if (!data || !data.feeds) {
            console.error("Invalid response from ThingSpeak");
            return;
        }

        // Update all sensor values
        Object.keys(sensorMapping).forEach(sensorType => {
            const field = sensorMapping[sensorType];
            const sensorValue = data[field] || "No data";
            document.getElementById(`${sensorType}Value`).innerText = sensorValue;
        });

        // Make sure table is visible
        document.getElementById("sensorTable").style.display = "table";
    } catch (error) {
        console.error("Error fetching data:", error);
    }
}

// Show sensor data (Highlight instead of hiding)
function showSensor(sensorId) {
    document.querySelectorAll("#sensorTable tr").forEach(row => row.classList.remove("highlight"));
    document.getElementById(sensorId).classList.add("highlight");
}

// Fetch data when dashboard loads
document.addEventListener("DOMContentLoaded", fetchSensorData);
