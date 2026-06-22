# Arch Linux build and install

This project can be packaged on Arch Linux so the app starts with the `chess` command.

## Build

From the repository root:

```bash
chmod +x scripts/package-linux.sh
./scripts/package-linux.sh
```

The build produces:

- `build/FlameBot` - the C++ engine
- `gui/dist/FlameBot-Chess-linux-x64/` - the Electron app bundle

The packaged app bundle includes `install.sh`.

## Install on the Arch machine

Copy the whole `gui/dist/FlameBot-Chess-linux-x64/` folder to the Arch machine, then run:

```bash
cd FlameBot-Chess-linux-x64
chmod +x install.sh
sudo ./install.sh
```

This installs the app to `/opt/flamebot-chess` and creates a `chess` command in `/usr/local/bin`.

## Run

After installation:

```bash
chess
```

