# hrBrowser

A minimal web browser built with C++ and WebKitGTK.

![hrBrowser Screenshot](https://via.placeholder.com/800x600?text=hrBrowser)

## Features

- 🖥️ **Desktop window** using GTK3
- 🌐 **Embedded browser** using WebKitGTK
- 🔍 **Address bar** with navigation
  - Enter any URL (e.g., `google.com`)
  - Search terms automatically search via Google
  - Auto-adds `https://` if missing

## Quick Start (Ubuntu)

### 1. Clone the repository

```bash
git clone https://github.com/Hrpavi/hrBrowser.git
cd hrBrowser
```

### 2. Initialize submodules (CEF headers)

```bash
git submodule update --init --recursive
```

### 3. Install dependencies

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libgtk-3-dev \
    libwebkit2gtk-4.1-dev
```

### 4. Build

```bash
mkdir -p build
cd build
cmake ..
make -j4
```

### 5. Run

```bash
./hrBrowser
```

## Usage

1. **Enter a URL** (e.g., `google.com`, `github.com`, `wikipedia.org`) in the address bar
2. Press **Enter** to navigate
3. To quit, close the window or press **Ctrl+C** in the terminal

### Address Bar Features

- URL without protocol: Auto-adds `https://`
- Plain text (no dots): Treated as Google search
- Full URLs: Navigates directly

## Project Structure

```
hrBrowser/
├── src/
│   └── main.cpp           # Browser source code
├── cef/                    # CEF submodule (headers)
├── cef_binary/             # CEF binary distribution
├── build/                 # Build output
│   └── hrBrowser          # Compiled executable
├── CMakeLists.txt         # Build configuration
├── LICENSE.txt            # MIT License
├── README.md              # This file
└── .gitmodules           # Git submodules config
```

## Requirements

### System Packages (Ubuntu 22.04+)

```bash
sudo apt-get install build-essential cmake pkg-config libgtk-3-dev libwebkit2gtk-4.1-dev
```

### For Older Ubuntu

```bash
# Ubuntu 20.04 (uses webkit2gtk-4.0)
sudo apt-get install libwebkit2gtk-4.0-dev
```

## Building with CEF (Optional)

The original goal was using Chromium Embedded Framework (CEF), but the official binary downloads were corrupted. To use CEF properly:

1. Download CEF from https://cef-builds.spotifycdn.com/index.html
2. Extract to `cef_binary/`
3. Copy generated headers to `cef/include/`

For more details, see [CEF Documentation](https://chromiumembedded.github.io/).

## License

MIT License - See [LICENSE.txt](LICENSE.txt)

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---

**Note:** This project uses WebKitGTK instead of CEF because the official CEF binary distributions were corrupted during download. WebKitGTK is also much simpler to integrate and has fewer dependencies.