// Sample array data (replace with your own data)

let data;
let currentCategory;
let apiUrl = "http://192.168.244.189";
//apiUrl = "";

async function getData() {
  let stations = [
    ["Veronica", "HTTP://veronical.nl", "Pop"],
    ["Item 2", "Value 2", "Pop"],
    ["Item 3", "Value 3", "Jazz"],
  ];
  let categories = ["Jazz", "Chill", "Pop", "News", "Local"];
  data = { stations: stations, categories: categories };

  const response = fetch(apiUrl + "/get-data");
  const result = await (await response).json();

  return result;
}
// Function to create input fields for each row in the array
function renderStationFields(selectedCategory) {
  const stationsContainer = document.getElementById("stationsContainer");
  stationsContainer.innerHTML = "";
  data.stations.forEach((station, index) => {
    if (station[2] === selectedCategory) {
      const stationContainer = document.createElement("div");
      stationContainer.id = index;
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
    }
  });
}

// Function to add a new station to the array
function addStation() {
  updateStations();
  data.stations.push(["", "", currentCategory]);
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
      data.stations[stationNr][0] = name;
      data.stations[stationNr][1] = url;
    } else {
      data.stations.splice(stationNr, 1);
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

function Save() {
  updateStations();
  console.log(data);
  let jsontext = JSON.stringify(data);
  console.log(jsontext);
  fetch(apiUrl + "/post-message", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: jsontext,
  })
    .then((response) => response.json())
    .then((data) => {
      console.log("Response from API:", data);
    })
    .catch((error) => {
      console.error("Error sending data:", error);
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

function exportStations() {
  alert(JSON.stringify(data, null, 2));
}
function importStations() {
  let newdata = window.prompt("Paste json", "");
  let newjson;
  try {
    newjson = JSON.parse(newdata);
  } catch (e) {
    // alert(
    //   "wrong json, use the same format as you get with [Export all] button"
    // );
  }

  if (!newjson || !newjson.categories || !newjson.stations) {
    alert(
      "wrong json, use the same format as you get with [Export all] button"
    );
    return;
  }
  data = newjson;
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
