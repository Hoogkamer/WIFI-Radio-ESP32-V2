# WIFI Radio ESP32 - User Manual

Welcome to your new WIFI Radio! This guide will help you set up and operate your device.

---

## 1. Getting Started

### Powering On
Plug the device into a 5V USB power source. The screen will display **"Starting..."** followed by the main radio interface.

### First-Time WiFi Setup
If the radio cannot find a known WiFi network, it will enter "Setup Mode":
1.  On your phone or computer, look for a WiFi network named **"WIFI_RADIO"**.
2.  Connect to it (no password required).
3.  A window should pop up automatically. If not, open your browser and go to `192.168.4.1`.
4.  Select your home WiFi network, enter your password, and click **Save**.
5.  The radio will restart and connect to your network.

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

## 3. Audio Settings (The Pro Menu)

To fine-tune your sound, you can access the **Audio Settings Menu**.

1.  **Enter Menu**: **Press and hold** the **Volume Knob** for 1 second.
2.  **Navigate**: Rotate the **Volume Knob** to move the **Blue Cursor** up and down.
3.  **Select**: Briefly press the **Volume Knob** on a setting. The cursor turns **Orange** (Edit Mode).
4.  **Adjust**: Rotate the knob to change the value (e.g., increase Bass).
5.  **Confirm**: Briefly press the knob again to save the change and return to the list.
6.  **Exit**: Press and hold the **Volume Knob** again to return to the radio.

**Available Settings:**
*   **Bass / Mid / Treble**: Adjust the tone of the music.
*   **Mode**: Switch between Stereo and Mono.
*   **Bal**: Adjust Balance between Left and Right speakers.
*   **Light**: Adjust the screen brightness.

---

## 4. Web Configuration (Advanced)

You can manage your station list using any web browser on the same WiFi.

1.  **Find your IP**: The IP address is displayed at the bottom of the radio screen (e.g., `192.168.1.50`).
2.  **Open Dashboard**: Type that IP address into your browser.
3.  **Edit Stations**: 
    - You can add new stream URLs.
    - Create or rename categories.
    - Reorder stations.
4.  **Save**: Click **Save** at the bottom. The radio will restart with your new list.

*Tip: Use HTTP links for streams. HTTPS links may sometimes cause audio stuttering on older ESP32 chips.*

---

## 5. Troubleshooting

*   **No Sound**: Check if the Volume is up and the device is not Muted.
*   **Stuttering Audio**: This is usually caused by a weak WiFi signal. Try moving the radio closer to your router or using an external antenna.
*   **"Not Connected"**: If your WiFi password changed, the radio will restart Setup Mode (see Section 1).
