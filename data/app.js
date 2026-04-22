// Delhi — matches firmware dummy origin (central Delhi / Connaught Place area).
const DELHI_LAT = 28.6139;
const DELHI_LON = 77.209;

const signalValue = document.getElementById("signalValue");
const satValue = document.getElementById("satValue");
const speedValue = document.getElementById("speedValue");
const batteryValue = document.getElementById("batteryValue");
const updatedValue = document.getElementById("updatedValue");
const gpsState = document.getElementById("gpsState");
const centerBtn = document.getElementById("centerBtn");
const mapContainer = document.getElementById("map");

let map = null;
let marker = null;
let pathLine = null;
let lastCoords = { lat: DELHI_LAT, lng: DELHI_LON };
let latestFix = { lat: DELHI_LAT, lon: DELHI_LON };
let mapAvailable = false;

function initMapIfAvailable() {
  if (typeof window.L === "undefined") {
    mapContainer.innerHTML =
      "<div style='padding:14px;color:#9ba7be'>Map unavailable in offline AP mode. GPS data is still live below.</div>";
    centerBtn.textContent = "Center unavailable";
    centerBtn.disabled = true;
    centerBtn.style.opacity = "0.55";
    centerBtn.style.cursor = "not-allowed";
    return false;
  }

  map = L.map("map", {
    zoomControl: true,
    attributionControl: true,
  }).setView([DELHI_LAT, DELHI_LON], 13);

  const delhiBounds = L.latLngBounds(
    [28.45, 77.05],  // southwest
    [28.78, 77.36],  // northeast
  );
  map.setMaxBounds(delhiBounds);

  L.tileLayer("/tiles/{z}/{x}/{y}.png", {
    minZoom: 12,
    maxZoom: 14,
    attribution: "Offline Delhi tile pack",
    noWrap: true,
  }).addTo(map);

  marker = L.marker([DELHI_LAT, DELHI_LON], { title: "Device location" }).addTo(map);
  pathLine = L.polyline([], { color: "#55a6ff", weight: 3, opacity: 0.7 }).addTo(map);
  lastCoords = marker.getLatLng();
  return true;
}

function animateMarker(nextCoords) {
  if (!marker) return;
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
      latestFix = next;
      animateMarker(next);
      if (pathLine) {
        pathLine.addLatLng([next.lat, next.lon]);
      }
    }
  } catch (error) {
    gpsState.classList.remove("ok");
    gpsState.classList.add("warn");
    gpsState.textContent = "Connection error";
  }
}

centerBtn.addEventListener("click", () => {
  if (!mapAvailable) {
    return;
  }
  const pos = marker ? marker.getLatLng() : { lat: latestFix.lat, lng: latestFix.lon };
  map.flyTo([pos.lat, pos.lng], Math.max(map.getZoom(), 15), {
    duration: 0.6,
    animate: true,
  });
});

mapAvailable = initMapIfAvailable();
refreshGps();
setInterval(refreshGps, 1500);
