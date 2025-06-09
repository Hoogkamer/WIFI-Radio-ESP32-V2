// Sample array data (replace with your own data)

let data;
let currentCategory;
let apiUrl = "http://192.168.11.65";
apiUrl = "";

let maxID = 3;

async function getData() {
  const fallbackData = {
    stations: [["Veronica", "http://22343.live.streamtheworld.com/VERONICA.mp3", "Pop"]],
    categories: ["Jazz", "Chill", "Pop", "News", "Local"]
  };

  try {
    const response = await fetch(apiUrl + "/get-data");
    if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);

    const text = await response.text();
    return processData(text)
  }
   catch (e) {
    console.error("Error loading stations data:", e);
    return fallbackData;
  }
}
function processData(text){
    // Parse the text file format
    const stations = [];
    const categories = [];
    let currentCategory = null;

    text.split(/\r?\n/).forEach(line => {
      line = line.trim();
      if (!line) return; // skip empty lines

      if (line.startsWith('[') && line.endsWith(']')) {
        currentCategory = line.slice(1, -1);
        categories.push(currentCategory);
      } else {
        const eqIndex = line.indexOf('=');
        if (eqIndex > 0 && currentCategory) {
          const name = line.slice(0, eqIndex).trim();
          const url = line.slice(eqIndex + 1).trim();
          stations.push([name, url, currentCategory]);
        }
      }
    });

    // Add IDs to stations
    stations.forEach((r, i) => r.push(i));
    
    maxID = stations.length;

    return { stations, categories };

 
}


function swapStation(id1, id2) {
  const index1 = data.stations.findIndex((s) => s[3] === id1);
  const index2 = data.stations.findIndex((s) => s[3] === id2);
  console.log(
    "swapping",
    id1,
    id2,
    index1,
    index2,
    data.stations[index1],
    data.stations[index2]
  );

  if (index1 !== -1 && index2 !== -1) {
    [data.stations[index1], data.stations[index2]] = [
      data.stations[index2],
      data.stations[index1],
    ];
  }
}
// Function to create input fields for each row in the array
function renderStationFields(selectedCategory) {
  
  data.stations.forEach((station, index) => {
    station[3] = index;
  });
  console.log("render", data.stations);
  const stationsContainer = document.getElementById("stationsContainer");
  stationsContainer.innerHTML = "";
  const stationsWithThisCategory = data.stations.filter(
    (s) => s[2] === selectedCategory
  );
  if (!stationsWithThisCategory.length) return;
  stationsWithThisCategory.forEach((station, index) => {
    const stationContainer = document.createElement("div");
    stationContainer.id = station[3];
    stationContainer.className = "stationDiv";

    // Create the first column input field
    const input1 = document.createElement("input");
    input1.type = "text";
    //   input1.name = `item-${index}`;
    input1.name = "name";
    input1.value = station[0];
    input1.placeholder = "Name";
    input1.className = "inpname";
    stationContainer.appendChild(input1);
    // Create the second column input field
    const input2 = document.createElement("input");
    input2.type = "text";
    //   input2.name = `value-${index}`;
    input2.name = "url";
    input2.value = station[1];
    input2.placeholder = "URL";
    input2.className = "inpurl";
    stationContainer.appendChild(input2);
    stationsContainer.appendChild(stationContainer);
  });
}

// Function to add a new station to the array
function addStation() {
  updateStations();
  data.stations.push(["", "", currentCategory, maxID++]);
  renderStationFields(currentCategory);
}

function updateStations() {
  const div = document.getElementById("stationsContainer");
  const inputFields = div.querySelectorAll("div");
  let name, url;
  inputFields.forEach((stationDiv) => {
    let stationNr = parseInt(stationDiv.id);
    name = stationDiv.querySelectorAll('input[name="name"]')[0].value;
    url = stationDiv.querySelectorAll('input[name="url"]')[0].value;
    if (name && url) {
      let theStation = data.stations.find((s) => s[3] == stationNr);
      console.log(stationNr, theStation);
      theStation[0] = name;
      theStation[1] = url;
    } else {
      data.stations = data.stations.filter((s) => s[3] != stationNr);
    }
  });
}

// Render the initial input fields on page load

// Function to create and populate the dropdown options
function populateCategories() {
  const categoryDropdown = document.getElementById("categoryDropdown");

  // Clear existing options
  categoryDropdown.innerHTML = "";

  // Create and add new options based on the array data
  data.categories.forEach((value) => {
    if (!value) return;
    const option = document.createElement("option");
    option.text = value;
    categoryDropdown.add(option);
  });
  if (!currentCategory || !data.categories.includes(currentCategory))
    currentCategory = data.categories[0];

  categoryDropdown.value = currentCategory;
}
function selectCategory() {
  updateStations();
  const categoryDropdown = document.getElementById("categoryDropdown");
  currentCategory = categoryDropdown.value;
  renderStationFields(currentCategory);
}

function getSaveText() {
  let text = "";
  for (let category of data.categories) {
    text += `[${category}]\n`;
    const stationsInCategory = data.stations.filter(
      (station) => station[2] === category
    );
    for (let [name, url] of stationsInCategory.map(s => s.slice(0, 2))) {
      text += `${name}=${url}\n`;
    }
    text += `\n`; // Optional: space between categories
  }
  return text.trim(); // Remove trailing newline
}

function Save() {
  updateStations();
  renderStationFields(currentCategory);
  const text = getSaveText();
  console.log(text); // For debugging

  fetch(apiUrl + "/post", {
    method: "POST",
    headers: {
      "Content-Type": "text/plain",
    },
    body: text,
  })
    .then((response) => response.text())
    .then((data) => {
      alert("Stations saved");
    })
    .catch((error) => {
      alert("There is an error: " + error);
    });
}

function deleteCategory() {
  if (confirm("This will also delete the stations for this category")) {
    data.stations = data.stations.filter((s) => s[2] !== currentCategory);
    data.categories = data.categories.filter((c) => c !== currentCategory);
    populateCategories();
    renderStationFields(currentCategory);
  }
}
function addCategory() {
  const newcat = prompt("Category Name:");
  if (!data.categories.includes(newcat)) {
    data.categories.push(newcat);
  }
  currentCategory = newcat;
  populateCategories();
  renderStationFields(currentCategory);
}
// Call the function to populate the dropdown on page load

function hideExportStations() {
  const exportDiv = document.getElementById("exportdiv");
  exportDiv.style.display = "none";
}
function exportStations() {
  const importDiv = document.getElementById("importdiv");
  const exportDiv = document.getElementById("exportdiv");
  const pasteTextDiv = document.getElementById("pastetextdiv");

  if (importDiv) importDiv.style.display = "none";
  if (exportDiv) exportDiv.style.display = "block";
  if (pasteTextDiv) pasteTextDiv.textContent = getSaveText();
}
function hideImportStations() {
  const importDiv = document.getElementById("importdiv");
  importDiv.style.display = "none";
}
function importStations() {
  const exportdiv = document.getElementById("exportdiv");
  const importdiv = document.getElementById("importdiv");
  exportdiv.style.display = "none";  // hide export div
  importdiv.style.display = "block"; // show import div
}
function processInput() {
  const inputtextdiv = document.getElementById("inputtextdiv");
  const newdata = inputtextdiv.value; // get text from textarea
  data = processData(newdata);        // parse and update data

  console.log('1', data);
  populateCategories();
  renderStationFields(currentCategory);
}
function init() {
  data = getData().then((dt) => {
    data = dt;
    populateCategories();
    renderStationFields(currentCategory);
  });
}
init();
