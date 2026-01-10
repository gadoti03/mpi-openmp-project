# Project Instructions

This guide explains how to set up the environment, install dependencies, and run the script to generate graphs.

## 1. Create a virtual environment

It's recommended to use a virtual environment to avoid conflicts with system packages.

```bash
python3 -m venv venv
```

Activate the virtual environment:

* On Linux/macOS:

```bash
source venv/bin/activate
```

* On Windows (PowerShell):

```powershell
.\venv\Scripts\Activate.ps1
```

## 2. Install dependencies

Make sure you have a `requirements.txt` file in the project folder. Then install the required packages with:

```bash
pip install -r requirements.txt
```

## 3. Run the script

After activating the virtual environment and installing the dependencies, run:

```bash
python3 generate_graphs.py
```

This will generate the graphs as defined in the script.
