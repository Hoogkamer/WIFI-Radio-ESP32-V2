// Sample array data (replace with your own data)

let data;
let currentCategory;
let apiUrl = "http://192.168.11.65";
apiUrl = "";

let maxID = 3;

async function getData() {
  let stations = [
    ["Veronica", "http://22343.live.streamtheworld.com/VERONICA.mp3", "Pop"],
  ];
  let categories = ["Jazz", "Chill", "Pop", "News", "Local"];
  data = { stations: stations, categories: categories };
  let result;
  const response = fetch(apiUrl + "/get-data");
  try {
    result = await (await response).json();
  } catch (e) {
    console.log(e);
    result = "";
  }
  console.log(result);
  result.stations = result.stations.map((r, i) => [...r, i]);
  maxID = result.stations.length;
  if (result) return result;
  else return data;
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
  console.log("render", data.stations);
  const stationsContainer = document.getElementById("stationsContainer");
  stationsContainer.innerHTML = "";
  const stationsWithThisCategory = data.stations.filter(
    (s) => s[2] === selectedCategory
  );
  if (!stationsWithThisCategory.length) return;
  const lastId =
    stationsWithThisCategory[stationsWithThisCategory.length - 1][3];
  const firstId = stationsWithThisCategory[0][3];
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
    if (stationContainer.id != firstId) {
      const button = document.createElement("button");
      button.className = "button-9 widthup1";
      button.textContent = "U";
      button.title = "Move station up";
      button.onclick = function () {
        updateStations();
        swapStation(
          stationsWithThisCategory[index][3],
          stationsWithThisCategory[index - 1][3]
        );
        renderStationFields(currentCategory);
      };
      stationContainer.appendChild(button);
    }
    if (stationContainer.id != lastId) {
      const button = document.createElement("button");
      button.className = "button-9 widthup1";
      button.textContent = "D";
      button.title = "Move station down";
      button.onclick = function () {
        updateStations();
        swapStation(
          stationsWithThisCategory[index][3],
          stationsWithThisCategory[index + 1][3]
        );
        renderStationFields(currentCategory);
      };
      stationContainer.appendChild(button);
    }
    stationsContainer.appendChild(stationContainer);
  });
}

// Function to add a new station to the array
function addStation() {
  updateStations();
  data.stations.push(["", "", currentCategory, maxID++]);
  renderStationFields(currentCategory);
}
function moveCategoryUp() {
  const index = data.categories.findIndex((cat) => cat === currentCategory);

  if (index >= 0) {
    const element = data.categories[index];
    data.categories.splice(index, 1); // Remove the element from its current position

    if (index === 0) {
      data.categories.push(element); // Move the element to the last position
    } else {
      data.categories.splice(index - 1, 0, element); // Insert the element before the previous element
    }
  }
  populateCategories();
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

function getSaveJSON() {
  const tosave = {
    categories: data.categories,
    stations: data.stations.map((station) => station.slice(0, -1)),
  };
  let jsontext = JSON.stringify(tosave);
  return jsontext;
}
function Save() {
  updateStations();
  renderStationFields(currentCategory);
  const jsontext = getSaveJSON();
  console.log(jsontext);
  fetch(apiUrl + "/post", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: jsontext,
  })
    .then((response) => response.text())
    .then((data) => {
      alert("Stations saved");
    })
    .catch((error) => {
      alert("There is an error:" + error);
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
  exportdiv.style.display = "none";
}
function exportStations() {
  const exportdiv = document.getElementById("exportdiv");
  exportdiv.style.display = "block";
  const pastetextdiv = document.getElementById("pastetextdiv");
  pastetextdiv.textContent = getSaveJSON();
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
