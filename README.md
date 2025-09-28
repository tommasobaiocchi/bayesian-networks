# Bayesian Network Inference Engine

This project implements exact inference in **Bayesian networks** using the **Variable Elimination** algorithm.  
It is capable of parsing `.bif` files to construct a probabilistic graphical model and compute marginal distributions for each variable in the network.

Written in C++17, the engine is fast, modular, and designed for educational, research, or experimental use.

---

## 🚀 Features

- 🔍 **Parses BIF** (Bayesian Interchange Format) files
- 🧠 **Builds** the Bayesian network from variable declarations and CPTs
- 📊 **Performs exact inference** via variable elimination
- 🧮 Calculates **marginal probabilities** for each node
- ⚙️ Fully **self-contained** C++ implementation
- 🧪 Includes **real-world BIF examples**

---

## 🧠 Motivation

Bayesian networks are powerful tools for modeling uncertainty, reasoning under incomplete information, and decision making.

This project was developed as an inference engine capable of reading structured probabilistic models from `.bif` files and executing efficient exact inference using variable elimination. The code is entirely written from scratch in C++ to highlight algorithmic clarity, data structure design, and recursion-based factor manipulation.

---

## 🏗️ Project Structure
```
bayesian-networks/
├── src/                     # (optional) Source files
│   └── main.cpp             # Main program (parses, infers, prints)
├── bif/                     # BIF files (Bayesian networks)
│   ├── cancer.bif
│   ├── asia.bif
│   ├── earthquake.bif
│   ├── diabetes.bif
│   └── water.bif
├── LICENSE
├── README.md                # This file
└── .gitignore
```

## ⚙️ Build Instructions

To compile the program, run:

```bash
g++ -std=c++17 main.cpp -o bayes

```
## 🧪 Example Output (`cancer.bif`)

```
Marginal Probability of Cancer:
P(Cancer=yes) = 0.448
P(Cancer=no)  = 0.552

Marginal Probability of XRay:
P(XRay=yes) = 0.568
P(XRay=no)  = 0.432

Marginal Probability of Dyspnoea:
P(Dyspnoea=yes) = 0.658
P(Dyspnoea=no)  = 0.342
```
## 📎 Included BIF Examples

This repository includes several real or benchmark Bayesian networks in `.bif` format for demonstration and testing:

- `cancer.bif` – Small diagnostic network (3–4 variables).
- `asia.bif` – Classic 8-node benchmark network used in literature.
- `earthquake.bif` – Simple alarm-based dependency model.
- `diabetes.bif` – Larger medical diagnosis network with realistic structure.
- `water.bif` – Environmental risk assessment network.

You can test any of these by replacing the input path passed to the executable.

---

## 📚 Features

- Parses `.bif` files to construct the Bayesian network.
- Represents variables, domains, parents, and conditional probability tables.
- Performs exact inference via **variable elimination**.
- Computes marginal probabilities for each variable.
- Supports networks with multiple parents and multiple values per variable.
- Handles any well-formed `.bif` file.

## 📄 License

This project is licensed under the MIT License. See the [LICENSE](./LICENSE) file for details.

---

## 👤 Author

**Tommaso Baiocchi**  
Bayesian Networks – Exact Inference in C++  
GitHub: [@tommasobaiocchi](https://github.com/tommasobaiocchi)
