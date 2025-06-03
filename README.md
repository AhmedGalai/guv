# guv - Global UV Environment Manager

**guv** is a lightweight, cross-platform CLI tool that manages global Python virtual environments using [`uv`](https://github.com/astral-sh/uv). It mimics basic `conda`-like features and works seamlessly on **Linux**, **WSL**, and **Windows PowerShell**.

## ✨ Features

- ✅ Create isolated Python environments with `uv venv`
- ✅ Clone environments with frozen dependencies
- ✅ Activate environments with shell-aware output
- ✅ Reset or delete specific or all environments
- ✅ Linux and Windows path support
- ✅ Automatic detection of missing `uv` with install instructions

## 📦 Requirements

- A C++17-capable compiler:
  - Linux: `g++ >= 8.1`
  - Windows: MinGW or MSVC
- [`uv`](https://github.com/astral-sh/uv) installed:
  - **Linux/macOS**:
    ```bash
    curl -Ls https://astral.sh/uv/install.sh | sh
    ```
  - **Windows**:
    ```powershell
    python -m pip install uv
    ```

## 📂 Environment Storage

All environments are stored at:

```
~/.guv/envs/
```

or on Windows:

```
C:\Users\<You>\.guv\envs\
```

## 🛠 Build Instructions

### Linux / WSL

```bash
g++ -std=c++17 guv.cpp -o guv
```

### Windows (MinGW / PowerShell)

```powershell
g++ -std=c++17 guv.cpp -o guv.exe
```

## 💻 Usage

```bash
guv venv <env>         # Create new environment
guv list               # List all environments
guv activate <env>     # Print activation command
guv clone <src> <dst>  # Clone env with installed packages
guv reset <env>        # Delete one environment
guv reset --all        # Delete all environments
guv config             # (Not implemented yet)
guv help               # Show usage help
guv version            # Show version
```

## 🧠 Activating an Environment

`guv` cannot modify the current shell environment directly. You must **run the printed command manually**.

### PowerShell

```powershell
guv activate base
# then copy the proposed command which follows the format:
# Invoke-Expression -Command "& '~/.guv/envs/$envname/Scripts/Activate.ps1'"
```

Or define a function in your PowerShell profile:

```powershell
function guv {
    $out = & "C:\path\to\guv.exe" @args
    if ($args[0] -eq "activate") {
        Invoke-Expression -Command "& $out"
    } else {
        Write-Output $out
    }
}
```

### Bash / WSL

```bash
eval "$(./guv activate base)"
```

## 🧪 Example Workflow

```bash
guv venv base
source ~/.guv/envs/base/bin/activate
uv pip install requests rich
guv clone base ml
guv list
guv activate ml
source ~/.guv/envs/ml/bin/activate
guv reset ml
```

## 📬 TODO

- [ ] `guv config` for global preferences
- [ ] `guv info <env>` to show env metadata
- [ ] Shell completion scripts (bash, zsh, PowerShell)
- [ ] Package management: list, remove, upgrade

## 📝 License

MIT License — free for personal or commercial use.

## 💡 Inspiration

This tool was built to provide a **portable, fast, zero-friction environment manager** using [`uv`](https://github.com/astral-sh/uv) and C++, with a UX similar to `conda`, but without the overhead.
