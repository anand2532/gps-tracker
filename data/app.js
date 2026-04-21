// Delhi — matches firmware dummy origin (central Delhi / Connaught Place area).
const DELHI_LAT = 28.6139;
const DELHI_LON = 77.209;

const map = L.map("map", {
  zoomControl: true,
  attributionControl: true,
}).setView([DELHI_LAT, DELHI_LON], 13);

L.tileLayer("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png", {
  maxZoom: 19,
  attribution: "&copy; OpenStreetMap contributors",
}).addTo(map);

const marker = L.marker([DELHI_LAT, DELHI_LON], { title: "Device location" }).addTo(map);
const pathLine = L.polyline([], { color: "#55a6ff", weight: 3, opacity: 0.7 }).addTo(map);

const signalValue = document.getElementById("signalValue");
const satValue = document.getElementById("satValue");
const speedValue = document.getElementById("speedValue");
const batteryValue = document.getElementById("batteryValue");
const updatedValue = document.getElementById("updatedValue");
const gpsState = document.getElementById("gpsState");
const centerBtn = document.getElementById("centerBtn");

let lastCoords = marker.getLatLng();

function animateMarker(nextCoords) {
  const start = performance.now();
  const from = { lat: lastCoords.lat, lon: lastCoords.lng };
  const duration = 700;

  function frame(ts) {
    const t = Math.min((ts - start) / duration, 1);
    const lat = from.lat + (nextCoords.lat - from.lat) * t;
    const lon = from.lon + (nextCoords.lon - from.lon) * t;
    marker.setLatLng([lat, lon]);
    if (t < 1) {
      requestAnimationFrame(frame);
    }
  }

  requestAnimationFrame(frame);
  lastCoords = L.latLng(nextCoords.lat, nextCoords.lon);
}

function setState(valid) {
  gpsState.classList.remove("ok", "warn");
  if (valid) {
    gpsState.textContent = "GPS fix active";
    gpsState.classList.add("ok");
  } else {
    gpsState.textContent = "No GPS fix (simulated loss)";
    gpsState.classList.add("warn");
  }
}

async function refreshGps() {
  try {
    const response = await fetch("/api/gps", { cache: "no-store" });
    if (!response.ok) throw new Error("API request failed");
    const data = await response.json();

    signalValue.textContent = `${Math.max(0, data.signal ?? 0)}%`;
    satValue.textContent = `${data.satellites ?? 0}`;
    speedValue.textContent = `${(data.speed ?? 0).toFixed(1)} km/h`;
    batteryValue.textContent = `${(data.battery ?? 0).toFixed(2)} V`;
    updatedValue.textContent = `Last updated: ${data.timestamp ?? "--"}`;
    const fixOk = data.valid !== false;
    setState(fixOk);

    if (Number.isFinite(data.lat) && Number.isFinite(data.lon) && fixOk) {
      const next = { lat: data.lat, lon: data.lon };
      animateMarker(next);
      pathLine.addLatLng([next.lat, next.lon]);
    }
  } catch (error) {
    gpsState.classList.remove("ok");
    gpsState.classList.add("warn");
    gpsState.textContent = "Connection error";
  }
}

centerBtn.addEventListener("click", () => {
  const pos = marker.getLatLng();
  map.flyTo([pos.lat, pos.lng], Math.max(map.getZoom(), 15), {
    duration: 0.6,
    animate: true,
  });
});

refreshGps();
setInterval(refreshGps, 1500);
