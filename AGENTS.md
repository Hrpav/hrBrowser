# AGENTS.md

This file is for AI Agents (like Codex, Claude Code, Opencode e.t.c.) and for specific rules for them.

## Purpose
This document serves as the primary technical specification and operational manifest for AI entities interacting with this repository. It defines the architectural boundaries, coding standards, and state management protocols required to maintain project integrity during autonomous or semi-autonomous development.

## Instructions

NEVER commit/push on git without my permission.

## Project

### Technical Architecture
- **Primary Language**: `C++`20
- **Build system**: `Cmake`
- **UI**: GTK3
- **Browser Rendering**: `WebKitGTK 4.1`
- **Error Handling**: Implement logs everywhere in the project so its easy to debug if something goes wrong.

### Current Session state
- **Current State**: Has desktop window + address bar + WebView

> Warning: Has not been tested in real desktop enviroment(s) (headless issues)

### Issues encountered (fixed)
1. CEF binary downloads were corrupted -> switched to WebKitGTK;
2. Headless testing shows white screen (needs real desktop);
3. GPU errors in headless: ZINK failed to choose pdev (expected - no GPU on WSL machine)

### What needs testing 
- Run on real desktop with X11/Waylnad to verify rendering works (ask me if you want to test)

## Build

To build the project (ask me), run this:
```bash
mkdir -p build && cd build
cmake ..
make
./hrBrowser
```

## File Structure (current)

```
hrBrowser/
├── src/
│   └── browser.cpp
│   └── browser.h
│   └── main.cpp
│   └── ui.cpp
│   └── ui.h
├── cef/                 (git ignored and git submodule - CEF headers, unused)
├── build/               (git ignored compiled output)
├── .gitattributes       (git attributes for files)
├── .gitignore           (tells git what files to ignore)
├── .gitmodules          (tells git what folders to make a submodule on e.g. cef folder)
├── AGENTS.md            (guide for AI Agents, this file)
├── CMakeLists.txt       (compiler options for CMake)
├── LICENSE.txt          (MIT license for the github project, name Hrpavi year 2026)
├── OPENCODE_SESSION.txt (git ignored, the opencode session that i use)
├── README.md            (file that tells the user how to use and compile the project)
```

## Git

### Repository

The current repository of this project is https://github.com/Hrpav/hrBrowser.

### History (`origin/main`)

1. 01bfca9 - Initial commit
2. 3337b6d - update README.md
3. 109f045 - Add CEF submodule
4. 27f7afd - update README.md

### Local HEAD

Local HEAD is at 27f7afd (same as `origin`)


