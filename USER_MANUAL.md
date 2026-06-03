# WIFI Radio ESP32 - User Manual

Welcome to your WIFI Radio! This guide will help you set up and operate your device.

---

## 1. Getting Started

### Powering On
Plug the device into a 5V USB power source. The screen will display **"Starting..."** followed by the main radio interface.

### First-Time WiFi Setup
If the radio cannot find a known WiFi network, it will enter "WiFi Setup Mode":
1.  The radio screen will show **WiFi Config** instructions and a **QR Code**.
2.  On your phone or tablet, scan the **QR Code** on the radio screen OR connect to the WiFi network named **"WIFI_RADIO"** (no password required).
3.  If you scanned the QR code or your browser opened, you will see the configuration portal. Otherwise, open a browser and go to `http://192.168.4.1`.
4.  Select your home WiFi network, enter your password, and click **Save**.
5.  The radio will restart and connect to your home WiFi.

---

## 2. Operation (Rotary Knobs)

Your radio has two main controls: the **Volume Knob** (Left) and the **Tuner Knob** (Right).

### Basic Listening
*   **Adjust Volume**: Turn the **Volume Knob** left or right.
*   **Switch Stations**: Turn the **Tuner Knob** to browse stations in the current category.
*   **Mute**: Briefly press (short click) the **Volume Knob**. Rotate it to unmute.
*   **Turn Off Screen**: Briefly press (short click) the **Tuner Knob** to dim/turn off the screen for night mode.

### Browsing Categories
1.  Briefly press the **Tuner Knob** once to enter **Category Mode**.
2.  Rotate the **Tuner Knob** to scroll through categories (e.g., Jazz, News, Pop).
3.  Press the knob again to enter that category and browse its stations.

### Saving a Favorite Station
If you want the radio to always start with a specific station:
1.  Tune to the station you like.
2.  **Press and hold** the **Tuner Knob** for 1 second.
3.  The screen will show "Station Saved".

---

## 3. Operation (Remote Control)

If your device is equipped with an Infrared (IR) receiver, you can also use a remote control (standard Apple-style remote mapping).

*   **Menu**: Enter or Exit the **Settings Menu**.
*   **Center Button**: Toggle Screen / Edit Setting.
*   **Up / Down / Left / Right**: 
    - *Radio Mode*: Change Station or Category.
    - *Settings Mode*: Navigate list or adjust values.
*   **Play/Pause**: Mute / Unmute (or Toggle Edit Setting).
*   **Hold Play/Pause**: Save current station as favorite.

---

## 4. Settings Menu

To adjust settings, you can access the **Settings Menu**.

1.  **Enter Menu**: **Press and hold** the **Volume Knob** for 1 second.
2.  **Navigate**: Rotate the **Volume Knob** to move the **Blue Cursor** up and down.
3.  **Select/Edit**: Briefly press the **Volume Knob** on a setting. The cursor turns **Orange** (Edit Mode).
4.  **Adjust**: Rotate the knob to change the value (e.g., increase Bass).
5.  **Confirm**: Briefly press the knob again to apply changes and return to the list.
6.  **Exit**: Press and hold the **Volume Knob** again to return to the radio. **All settings are automatically saved when you exit this menu.**

**Available Settings:**
*   **Bass / Mid / Treble**: Adjust the tone of the music.
*   **Mode**: Switch between Stereo and Mono.
*   **Bal**: Adjust Balance between Left and Right speakers.
*   **Light**: Adjust the screen brightness.
*   **Web UI**: Displays a **"Show QR"** label. Select this option (short click) to clear the screen and show the **System Details** page, featuring the radio's IP address and a **QR Code** to easily open the web portal on your phone. Press any button to return to the settings list.

---

## 5. Web Configuration (Advanced)

You can manage your station list using any web browser on the same WiFi.

1.  **Open Portal**: Select the **Web UI** option under **Settings** on the radio screen to display the **QR Code**, and scan it with your phone, OR type the radio's IP address (shown at the bottom of the details page or main screen) into your browser.
2.  **Edit Stations**: 
    - **Find & Add Stations**: Search thousands of online streams from `radio-browser.info` directly from the search tab and click "Add". 
    **Go back to "Manage Stations" to save changes to the device**
    - **Reorder**: Use the **↑ (Up)** and **↓ (Down)** buttons to arrange stations.
    - **Manage Categories**: Add or delete categories of music.
3.  **Backup**: Use **Export Config** to copy your station list to a text file. Use **Import Config** to paste it back.
4.  **Save**: Click **SAVE ALL TO RADIO**. The radio will restart with your new list.

> [!WARNING]
> **Rebuilding / uploading the filesystem** (`uploadfs` target in PlatformIO) will overwrite `/stations.txt` on the ESP32. Export your station backup first and import it back after uploading the filesystem!

---

## 6. Local Development & Testing

You can develop, test, and run the website completely offline on your computer:
1.  Navigate to the project root directory.
2.  Run `npm start` (requires Node/NPM) or `npx http-server data -p 8080`.
3.  Open `http://localhost:8080` in your browser.
4.  The page will load in **Demo Mode**. Any changes you make will be saved in your browser's local storage for easy visual testing!

---

## 7. Troubleshooting

*   **No Sound**: Check if the Volume is up and the device is not Muted.
*   **Stuttering Audio**: This is usually caused by a weak WiFi signal. Try moving the radio closer to your router or using an external antenna.
*   **"Not Connected"**: If your WiFi password changed, the radio will restart Setup Mode (see Section 1).
