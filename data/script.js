let data = { stations: [], categories: [] };
console.log("WIFI Radio Script v2.1 Loaded");
let currentCategory = "";
let apiUrl = "";
let hasChanges = false;

// Warn about unsaved changes
window.onbeforeunload = function() {
  if (hasChanges) return "You have unsaved changes. Are you sure you want to leave?";
};

async function getData() {
  const fallbackData = {
    stations: [["Veronica", "http://22343.live.streamtheworld.com/VERONICA.mp3", "Pop"]],
    categories: ["Pop"]
  };

  try {
    const response = await fetch(apiUrl + "/get-data");
    if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
    const text = await response.text();
    return processData(text);
  } catch (e) {
    console.error("Error loading stations data:", e);
    return fallbackData;
  }
}

function processData(text) {
  const stations = [];
  const categories = [];
  let currentCat = null;

  text.split(/\r?\n/).forEach(line => {
    line = line.trim();
    if (!line) return;

    if (line.startsWith('[') && line.endsWith(']')) {
      currentCat = line.slice(1, -1);
      if (!categories.includes(currentCat)) categories.push(currentCat);
    } else {
      const eqIndex = line.indexOf('=');
      if (eqIndex > 0 && currentCat) {
        const name = line.slice(0, eqIndex).trim();
        const url = line.slice(eqIndex + 1).trim();
        stations.push([name, url, currentCat, stations.length]);
      }
    }
  });

  return { stations, categories };
}

function renderStationFields() {
  const container = document.getElementById("stationsContainer");
  container.innerHTML = "";
  
  const filtered = data.stations.filter(s => s[2] === currentCategory);
  
  if (filtered.length === 0) {
    container.innerHTML = '<div style="padding: 20px; text-align: center; color: #888;">No stations in this category.</div>';
    return;
  }

  filtered.forEach((station, index) => {
    const div = document.createElement("div");
    div.className = "stationDiv";
    div.dataset.id = station[3];

    // Inputs
    const nameInp = `<input type="text" class="inpname" value="${station[0]}" placeholder="Name" onchange="markChanged()">`;
    const urlInp = `<input type="text" class="inpurl" value="${station[1]}" placeholder="URL" onchange="markChanged()">`;
    
    // Controls (Using SVGs for cross-browser compatibility)
    const upBtn = `<button class="button-9 btn-icon" onclick="moveStation(${station[3]}, -1)" title="Move Up">
      <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"><path d="m18 15-6-6-6 6"/></svg>
    </button>`;
    
    const downBtn = `<button class="button-9 btn-icon" onclick="moveStation(${station[3]}, 1)" title="Move Down">
      <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"><path d="m6 9 6 6 6-6"/></svg>
    </button>`;
    
    const playBtn = `<button class="button-9 btn-icon" style="background: #fb0" onclick="testStation('${station[1]}')" title="Test Stream">
      <svg width="20" height="20" viewBox="0 0 24 24" fill="currentColor"><path d="m7 3 14 9-14 9z"/></svg>
    </button>`;
    
    const delBtn = `<button class="button-9 btn-icon button-danger" onclick="deleteStation(${station[3]})" title="Delete">
      <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"><path d="M18 6 6 18M6 6l12 12"/></svg>
    </button>`;

    div.innerHTML = `
      ${nameInp}
      ${urlInp}
      <div class="controls-right">
        ${playBtn}
        ${upBtn}
        ${downBtn}
        ${delBtn}
      </div>
    `;
    container.appendChild(div);
  });
}

function updateDataFromInputs() {
  const container = document.getElementById("stationsContainer");
  const rows = container.querySelectorAll(".stationDiv");
  
  rows.forEach(row => {
    const id = parseInt(row.dataset.id);
    const name = row.querySelector(".inpname").value.trim();
    const url = row.querySelector(".inpurl").value.trim();
    
    const idx = data.stations.findIndex(s => s[3] === id);
    if (idx !== -1) {
      data.stations[idx][0] = name;
      data.stations[idx][1] = url;
    }
  });
}

function moveStation(id, dir) {
  updateDataFromInputs();
  const idx = data.stations.findIndex(s => s[3] === id);
  const currentStat = data.stations[idx];
  
  // Find relative position within category
  const catStations = data.stations.filter(s => s[2] === currentCategory);
  const catIdx = catStations.findIndex(s => s[3] === id);
  
  if (dir === -1 && catIdx > 0) {
    const prevStat = catStations[catIdx - 1];
    const realIdx1 = data.stations.indexOf(currentStat);
    const realIdx2 = data.stations.indexOf(prevStat);
    [data.stations[realIdx1], data.stations[realIdx2]] = [data.stations[realIdx2], data.stations[realIdx1]];
    hasChanges = true;
  } else if (dir === 1 && catIdx < catStations.length - 1) {
    const nextStat = catStations[catIdx + 1];
    const realIdx1 = data.stations.indexOf(currentStat);
    const realIdx2 = data.stations.indexOf(nextStat);
    [data.stations[realIdx1], data.stations[realIdx2]] = [data.stations[realIdx2], data.stations[realIdx1]];
    hasChanges = true;
  }
  
  renderStationFields();
}

function deleteStation(id) {
  if (confirm("Delete this station?")) {
    data.stations = data.stations.filter(s => s[3] !== id);
    hasChanges = true;
    renderStationFields();
  }
}

function addStation() {
  updateDataFromInputs();
  const maxId = data.stations.length > 0 ? Math.max(...data.stations.map(s => s[3])) + 1 : 0;
  data.stations.push(["New Station", "http://", currentCategory, maxId]);
  hasChanges = true;
  renderStationFields();
}

function populateCategories() {
  const dropdown = document.getElementById("categoryDropdown");
  dropdown.innerHTML = "";
  data.categories.forEach(cat => {
    const opt = document.createElement("option");
    opt.value = opt.text = cat;
    dropdown.add(opt);
  });
  if (!currentCategory || !data.categories.includes(currentCategory)) {
    currentCategory = data.categories[0] || "";
  }
  dropdown.value = currentCategory;
}

function selectCategory() {
  updateDataFromInputs();
  currentCategory = document.getElementById("categoryDropdown").value;
  renderStationFields();
}

function addCategory() {
  const name = prompt("New Category Name:");
  if (name && !data.categories.includes(name)) {
    data.categories.push(name);
    currentCategory = name;
    populateCategories();
    renderStationFields();
    hasChanges = true;
  }
}

function deleteCategory() {
  if (confirm(`Delete category "${currentCategory}" and ALL its stations?`)) {
    data.stations = data.stations.filter(s => s[2] !== currentCategory);
    data.categories = data.categories.filter(c => c !== currentCategory);
    currentCategory = data.categories[0] || "";
    populateCategories();
    renderStationFields();
    hasChanges = true;
  }
}

function testStation(url) {
  if (!url || url === "http://") return alert("Enter a URL first");
  window.open(url, "_blank");
}

function markChanged() {
  hasChanges = true;
}

async function searchStations() {
  const q = document.getElementById("searchInput").value.trim();
  if (q.length < 3) return alert("Please enter at least 3 characters");
  
  const resultsDiv = document.getElementById("searchResults");
  resultsDiv.innerHTML = "Searching...";
  
  try {
    // Using de1 mirror which is generally more reliable
    const response = await fetch(`https://de1.api.radio-browser.info/json/stations/byname/${encodeURIComponent(q)}?limit=20`);
    
    if (!response.ok) throw new Error("API returned an error");
    
    const results = await response.json();
    
    resultsDiv.innerHTML = "";
    if (results.length === 0) {
      resultsDiv.innerHTML = "No results found.";
      return;
    }
    
    results.forEach(s => {
      const item = document.createElement("div");
      item.className = "search-item";
      item.innerHTML = `
        <div style="flex: 1; margin-right: 10px; overflow: hidden;">
          <strong style="white-space: nowrap; overflow: hidden; text-overflow: ellipsis; display: block;">${s.name}</strong>
          <small style="color: #666; display: block; white-space: nowrap; overflow: hidden; text-overflow: ellipsis;">${s.country || ''} ${s.tags ? '• ' + s.tags : ''}</small>
        </div>
        <button class="button-9 widthsmall button-success" onclick="addFoundStation('${s.name.replace(/'/g, "\\'")}', '${s.url_resolved}')">Add</button>
      `;
      resultsDiv.appendChild(item);
    });
  } catch (e) {
    console.error("Search error:", e);
    resultsDiv.innerHTML = `<div style="padding: 10px; color: #d9534f;">
      Error connecting to search API.<br>
      <small>Note: Ad-blockers or "Private Window" mode may block this search. Try disabling them.</small>
    </div>`;
  }
}

function addFoundStation(name, url) {
  if (!currentCategory) return alert("Select or create a category first!");
  const maxId = data.stations.length > 0 ? Math.max(...data.stations.map(s => s[3])) + 1 : 0;
  data.stations.push([name, url, currentCategory, maxId]);
  hasChanges = true;
  renderStationFields();
  alert(`Added ${name} to ${currentCategory}`);
}

function getSaveText() {
  let text = "";
  data.categories.forEach(cat => {
    text += `[${cat}]\n`;
    data.stations.filter(s => s[2] === cat).forEach(s => {
      text += `${s[0]}=${s[1]}\n`;
    });
    text += `\n`;
  });
  return text.trim();
}

function Save() {
  updateDataFromInputs();
  const text = getSaveText();
  
  fetch(apiUrl + "/post", {
    method: "POST",
    headers: { "Content-Type": "text/plain" },
    body: text,
  })
  .then(() => {
    hasChanges = false;
    alert("Stations saved successfully! The radio will now restart.");
  })
  .catch(err => alert("Error saving: " + err));
}

function exportStations() {
  updateDataFromInputs();
  document.getElementById("exportdiv").style.display = "block";
  document.getElementById("pastetextdiv").textContent = getSaveText();
}

function importStations() {
  document.getElementById("importdiv").style.display = "block";
}

function hideExportStations() {
  document.getElementById("exportdiv").style.display = "none";
  document.getElementById("importdiv").style.display = "none";
}

function processInput() {
  const text = document.getElementById("inputtextdiv").value;
  if (!text) return;
  data = processData(text);
  hasChanges = true;
  populateCategories();
  renderStationFields();
  hideExportStations();
}

// Initialize
getData().then(dt => {
  data = dt;
  populateCategories();
  renderStationFields();
});
