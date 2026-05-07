# 🌪️ TEXTPURE

**TEXTPURE** is a professional granular texture explorer designed to transform any audio source into rich, organic soundscapes, digital grit, or ethereal clouds. By decomposing audio into tiny "grains" and reassembling them in real-time, TEXTPURE allows you to manipulate the very fabric of sound.

## ✨ Features

- **Granular Engine**: High-performance grain spawning with real-time control over size, density, and pitch.
- **Texture Control**: Add warmth or aggressive saturation to your grains using a custom non-linear distortion algorithm.
- **Integrated Reverb**: A lush post-processing reverb to place your textures in vast spaces.
- **Intelligent Graining**: Dynamic grain management ensures a smooth, non-gating output even at extreme settings.
- **Preset System**: Includes 6 signature presets:
  - Default: Clean, balanced granular processing.
  - Dark Clouds: Vague, low-pitched atmospheric textures.
  - Digital Grit: Short grains with high density and texture for a "glitchy" feel.
  - Ghost Melodies: High-pitched, reverb-drenched ethereal tones.
  - Subtle Texture: Gentle layering to add depth to your tracks.
  - Trap Shimmer: Bright, fast grains perfect for modern production.

## 🎛️ Parameters

| Parameter | Description |
| :--- | :--- |
| **Grain Size** | Controls the duration of each individual grain (10ms - 500ms). |
| **Density** | Determines how many grains are spawned per second. |
| **Pitch** | Transposes the playback speed of the grains without affecting duration. |
| **Texture** | Adds saturation and "bite" to the granular output. |
| **Mix** | Blends between the dry input and the textured wet signal. |
| **Reverb** | Adjusts the amount of spatial depth. |

## 🛠️ Technical Details

- **Built with**: JUCE Framework (C++17)
- **Compatibility**: VST3, Standalone (Win/macOS/Linux)
- **Architecture**: Object-oriented granular engine with sample-accurate parameter smoothing.

## 🚀 Installation

1. Clone the repository:
   `ash
   git clone git@github.com:MaxDedecke/Textpure.git
   `
2. Open NewProject.jucer in the **Projucer**.
3. Select your preferred exporter (Visual Studio, Xcode, etc.).
4. Build the solution.

---
*Created by MaxDedecke - Elevating sound through texture.*
