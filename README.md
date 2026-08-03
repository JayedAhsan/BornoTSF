# BornoTSF

**BornoTSF** is a lightweight, open-source Windows **Text Services Framework (TSF)** Input Method Editor (IME) for Bangla typing. Designed for ultra-fast, zero-overhead performance, BornoTSF integrates directly into the Windows OS input architecture without requiring any background executable (`.exe`) process or heavy graphical user interface.

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Platform](https://img.shields.io/badge/Platform-Windows%208.1%20%7C%2010%20%7C%2011-blue.svg)](https://microsoft.com/windows)
[![Language](https://img.shields.io/badge/Language-C%2B%2B-00599C.svg)](https://isocpp.org/)

---

## Key Features

* **Native Windows TSF Integration:** Registers directly into the Windows Language Bar and taskbar input selector (`Win + Space`).
* **Zero Overhead / No Background Process:** Operates purely as native system DLLs (`BornoTSF.dll`). No persistent background `.exe` processes consuming RAM or CPU.
* **Comprehensive Layout Support:**
  * **Borno Phonetic:** Avro Phonetic inspired, fast, rule-based phonetic parser.
  * **Avro Phonetic:** Most popular Bangla phonetic layout.
  * **Khipro:** The first compositional layout for Bangla.
  * **National (Jatiyo / UniBijoy):** BSTI (BDS 1738:2018) compliant standard Bangla layout.
  * **Probhat:** Traditional fixed layout.
* **Selective Layout Installation:** Choose only the keyboard layouts you need during installation (can be updated anytime).
* **Literal Text Escaping:** Effortlessly mix English and Bangla within phonetic mode using double backslashes (e.g., `facebook\\-এর` outputs `facebook-এর`).
* **Windows Spell Checker Compatibility:** Works seamlessly with native Windows Bangla spell checking.
* **Modern App & AI Agent Compatibility:** Fixed input stream buffering ensures text never breaks or scrambles in modern web applications, IDEs (VS Code), and AI chat interfaces (ChatGPT, Claude, etc.).
* **100% Offline & Private:** Zero network calls or telemetry. Your typing data never leaves your device.

---

## Layout Progress & Testing Status

| Layout | Type | Keymap | Status |
| :--- | :--- | :--- | :--- |
| **Borno Phonetic** | Phonetic Parser | [View Keymap](https://borno.codepotro.com/docs/guide/layout-borno.html) | Tested (OK) |
| **Avro Phonetic** | Phonetic Parser | [View Keymap](https://borno.codepotro.com/docs/guide/layout-avro.html) | Tested (OK) |
| **Khipro** | Compositional | [View Keymap](https://borno.codepotro.com/docs/guide/layout-khipro.html) | Tested (OK) |
| **National (Jatiyo)** | Fixed Layout | [View Keymap](https://borno.codepotro.com/docs/guide/layout-national.html) | Tested (WIP) |
| **Probhat** | Fixed Layout | [View Keymap](https://en.wikipedia.org/wiki/Bengali_input_methods#Probhat) | Tested (WIP) |

> **Note:** If you encounter any bugs or layout issues with National (Jatiyo) or Probhat, please open an issue!

---

## Installation & Usage

### Setup via Installer
1. Download the latest `BornoTSF_Setup_*.exe` from the [Releases](https://github.com/codepotro/BornoTSF/releases) section.
2. Run the installer and select your preferred keyboard layouts (e.g., Borno Phonetic, Avro Phonetic, National, etc.).
3. Accept the License Agreement and complete setup.
4. Press **`Win + Space`** or click the Language Bar on your Windows Taskbar to switch to your desired BornoTSF layout and start typing!

### System Tray & Keymaps
* Quick options are available in the system tray menu.
* Click the tray icon to quickly open keymap references for Phonetic or Fixed layouts.

---

## Building from Source

### Prerequisites
* **Visual Studio 2022** with Desktop Development with C++ workload installed.
* **Windows 10 / 11 SDK**
* **Boost C++ Libraries:** Required for layout regex processing (`boost/regex`). Define an environment variable `BOOST_ROOT` pointing to your Boost directory (e.g., `C:\boost_1_89_0` or `H:\boost_1_89_0`).

### Build Steps

1. **Clone the Repository:**
   ```bash
   git clone https://github.com/codepotro/BornoTSF.git
   cd BornoTSF
   ```

2. **Build DLL via MSBuild / Visual Studio:**
   Open `BornoTSF.vcxproj` or `BornoTSF.slnx` in Visual Studio 2022 and build for:
   * `Release | x64` (produces 64-bit `x64\Release\BornoTSF.dll`)
   * `Release | x86` (produces 32-bit `Release\BornoTSF.dll`)

3. **Register the DLLs:**
   Run `reg_bornoTSF.bat` as Administrator to copy required layout/dictionary assets and register the compiled DLLs:
   ```cmd
   reg_bornoTSF.bat
   ```

   To unregister and remove installed files, run:
   ```cmd
   unreg_bornoTSF.bat
   ```

---

## License

BornoTSF is licensed under the **GNU General Public License v3.0 (GPL-3.0)**. Free software for the community — modifications and redistributions must remain open-source under GPL v3.0.

---

## Credits & Acknowledgments

Special thanks to the Bangla computing community and developers whose work paved the way:

* **Avro Team & Dr. Mehdi Hasan Khan & Muhammad Mominul Huque** — For the Avro phonetic parser implementation.
* **Khipro Team** — For creating *Khipro*, the pioneer compositional layout for Bangla.
* **Omi Azad** — For key contributions to Ekushey and Bangla open-source development.
* **Borno Community** — For continuous feedback and layout recommendations.

---

## Sponsor

Want to sponsor this project or support open-source Bangla software development? You can support us through [Codepotro](https://www.codepotro.com) or by sponsoring our repositories on GitHub.

---

<p center="align">Maintained by <b>Jayed Ahsan Saad</b> • <a href="https://jayed.me">jayed.me</a> • <a href="https://www.codepotro.com">codepotro.com</a></p>
