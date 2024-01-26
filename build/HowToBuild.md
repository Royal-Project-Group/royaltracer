# How to build the Royal Tracer
## Backend
- Simply build C++ project in CLion.
- Copy `cmake-build-debug` folder into build folder.

## Frontend
### Bundle Frontend
- Go into `editor/src/`
- Run `npm install`
- Run `npx webpack --config webpack.config.cjs`
### Create Frontent Executer
- Have `pyinstaller` installed (`pip install pyinstaller`)
- Go into  `editor/`
- Run `pyinstaller --onefile editor.py`
### Insert to Build
- Create `editor` folder in build folder.
- Copy the follwing files/directories into the editor folder:
  - `editor/dist/editor.exe`
  - `editor/src/dist`
  - `editor/src/media`
  - `editor/src/favicon.ico`
  - `editor/src/index.html`
  - `editor/src/config.json`
  - `editor/src/style.css`

## Execute
- Place `build/royal-tracer.bat` in build folder.
- Double-click to execute.