# Progetto Python: Generazione Grafici

Questo progetto permette di generare grafici utilizzando i dati disponibili.
Qui trovi le istruzioni per creare l'ambiente virtuale, installare le dipendenze e avviare lo script `generate_graphs.py`.

---

## 1️⃣ Creare un ambiente virtuale

```bash
# Creazione dell'ambiente virtuale
python3 -m venv venv

# Attivazione dell'ambiente virtuale
# Linux / macOS
source venv/bin/activate

# Windows (cmd)
venv\Scripts\activate

# Windows (PowerShell)
venv\Scripts\Activate.ps1
```

## 2️⃣ Installare le dipendenze

```bash
pip install -r requirements.txt
```

> Questo comando legge il file `requirements.txt` e installa tutte le librerie elencate, con le versioni specificate.

## 3️⃣ Eseguire lo script principale

```bash
python3 generate_graphs.py
```

Lo script utilizzerà le librerie installate e produrrà i grafici nella cartella di output (di default).

## 4️⃣ Suggerimenti

* Assicurati di avere Python >= 3.10 installato.
* Per disattivare l'ambiente virtuale in qualsiasi momento:

```bash
deactivate
```

* Se aggiungi nuove librerie, aggiorna `requirements.txt` con:

```bash
pip freeze > requirements.txt
```