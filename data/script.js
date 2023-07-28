// Sample array data (replace with your own data)
let stations = [
  ["Veronica", "HTTP://veronical.nl", "Pop"],
  ["Item 2", "Value 2", "Pop"],
  ["Item 3", "Value 3", "Jazz"],
];
const categories = ["Jazz", "Chill", "Pop", "News", "Local"];
let currentCategory = categories[0];

// Function to create input fields for each row in the array
function renderStationFields(selectedCategory) {
  const stationsContainer = document.getElementById("stationsContainer");
  stationsContainer.innerHTML = "";
  stations.forEach((station, index) => {
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
  stations.push(["", "", currentCategory]);
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
      stations[stationNr][0] = name;
      stations[stationNr][1] = url;
    } else {
      stations.splice(stationNr, 1);
    }
    console.log(name, url, stationNr);
  });

  console.log(inputFields);
}

// Render the initial input fields on page load

// Function to create and populate the dropdown options
function populateCategories() {
  const categoryDropdown = document.getElementById("categoryDropdown");

  // Clear existing options
  categoryDropdown.innerHTML = "";

  // Create and add new options based on the array data
  categories.forEach((value) => {
    const option = document.createElement("option");
    option.text = value;
    categoryDropdown.add(option);
  });
}
function selectCategory() {
  updateStations();
  const categoryDropdown = document.getElementById("categoryDropdown");
  currentCategory = categoryDropdown.value;
  renderStationFields(currentCategory);
}

function Save() {
  console.log("Saving stations....");
}

// Call the function to populate the dropdown on page load
populateCategories();
renderStationFields(currentCategory);

// Handle form submission (you can customize this as needed)
document
  .getElementById("arrayForm")
  .addEventListener("submit", function (event) {
    event.preventDefault(); // Prevent the form from submitting for this example
    const formData = new FormData(event.target);

    // Display the submitted data (you can handle the data as per your requirements)
    for (const entry of formData.entries()) {
      console.log(entry);
    }
  });
