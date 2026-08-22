# 🕷️ Wind — Winkey × Tinky

> **A Windows-focused security research project built around low-level keyboard and system interaction.**

![Windows](https://img.shields.io/badge/platform-Windows-0078D6?style=for-the-badge\&logo=windows\&logoColor=white)
![C](https://img.shields.io/badge/language-C-A8B9CC?style=for-the-badge\&logo=c\&logoColor=black)
![Python](https://img.shields.io/badge/language-Python-3776AB?style=for-the-badge\&logo=python\&logoColor=white)
![Status](https://img.shields.io/badge/status-experimental-orange?style=for-the-badge)

---

## 🧠 About

**Wind** is a collaborative security-oriented project split into two complementary components:

* 🪟 **Winkey** — developed by **tren-chvl**
* 🐱 **Tinky** — developed by **Pixyde**

The project was created as a technical experiment to explore how software can interact with the Windows environment at a relatively low level.

The goal is primarily **learning, experimentation and security research**.

> ⚠️ **Educational / authorized-use only.**
>
> This project must only be used on systems you own or on systems where you have explicit authorization to perform security testing.

---

## 🧩 Project Architecture

```text
                 ┌─────────────────────┐
                 │        WIND         │
                 │  Security Research  │
                 └──────────┬──────────┘
                            │
             ┌──────────────┴──────────────┐
             │                             │
      ┌──────▼──────┐               ┌──────▼──────┐
      │   WINKEY    │               │    TINKY    │
      │   @tren     │               │   @Pixyde   │
      └──────┬──────┘               └──────┬──────┘
             │                             │
       Windows side                  Core / research
       & system logic                  component
```

The two components are maintained separately inside the same repository.

---

# 🪟 Winkey

**Winkey** is the Windows-oriented component of the project.

It was developed by **tren-chvl** and contains the Windows-side implementation and supporting tooling.

### Structure

```text
winkey/
├── process/
├── src/
├── .gitignore
├── Makefile
├── updater.py
└── winkey.h
```

### Main areas

| Directory / File | Purpose                            |
| ---------------- | ---------------------------------- |
| `src/`           | Main source code                   |
| `process/`       | Process-related components         |
| `winkey.h`       | Project declarations / definitions |
| `Makefile`       | Build configuration                |
| `updater.py`     | Supporting Python tooling          |
| `.gitignore`     | Git configuration                  |

---

# 🐱 Tinky

**Tinky** is the second major component of Wind.

It was developed by **Pixyde** and focuses on the core implementation side of the project.

### Structure

```text
tinky/
├── include/
├── src/
├── .gitignore
└── Makefile
```

### Main areas

| Directory / File | Purpose                |
| ---------------- | ---------------------- |
| `src/`           | Main implementation    |
| `include/`       | Headers and interfaces |
| `Makefile`       | Build configuration    |
| `.gitignore`     | Git configuration      |

---

# 👥 Contributors

## 🪟 tren-chvl

**Winkey developer**

> Responsible for the Windows-oriented side of the project and the Winkey component.

GitHub: [@tren-chvl](https://github.com/tren-chvl)

---

## 🐱 Pixyde

**Tinky developer**

> Responsible for the Tinky component and its core implementation.

GitHub: [@Pixyde](https://github.com/Pixyde)

---

# 🛠️ Technologies

Depending on the component, the project uses:

* **C** — low-level implementation
* **Python** — supporting tooling
* **Make** — build automation
* **Windows APIs / system interfaces** — Windows-specific interaction
* **Git / GitHub** — version control and collaboration

---

# 📂 Repository

```text
keylogger-Wind/
│
├── 🪟 winkey/
│   ├── process/
│   ├── src/
│   ├── Makefile
│   ├── updater.py
│   └── winkey.h
│
└── 🐱 tinky/
    ├── include/
    ├── src/
    ├── Makefile
    └── .gitignore
```

---

# 🎯 Project Goals

The project is mainly intended to explore:

* Low-level Windows programming
* Input and system interaction
* C programming
* Process-oriented programming
* Build systems
* Collaboration between multiple developers
* Security research concepts
* Understanding how defensive software can detect suspicious input-monitoring behavior

---

# 🔬 Security Research

Input-monitoring software is an important topic in cybersecurity because similar techniques can be abused to capture sensitive information.

For this reason, **Wind should only be executed in controlled environments** such as:

* 🧪 A personal test machine
* 🖥️ A dedicated virtual machine
* 🔐 An authorized security laboratory
* 🎓 An educational environment

**Never use the project to monitor another person without their explicit authorization.**

---

# 🚧 Project Status

> 🟠 **Experimental**

# 🌌 Why "Wind"?

Two components.

Two developers.

One project.

**Winkey × Tinky → Wind**

```text
       🪟 WINKEY
          +
       🐱 TINKY
          │
          ▼
       ┌───────┐
       │ WIND  │
       └───────┘
```

---
### Built by

**tren-chvl × Pixyde**

> *"Two codebases. One experiment."* 🕷️
