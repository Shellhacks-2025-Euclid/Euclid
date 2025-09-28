<img src="./Euclid-Web/public/euclid/euclid-logo.png" width="1000">

# Euclid

**Euclid** is a multi-app, cross-platform project that blends a modern 3D objects with native rendering/engine work. It
started during **ShellHacks 2025**.

---

## TL;DR:

<img src="./Euclid-Web/public/euclid/euclid-intro.webp" width="150">

### Who is Euclid?
Euclid was an ancient Greek mathematician active as a geometer and logician. 
Considered the "father of geometry", he is chiefly known for the Elements treatise, which established the foundations of geometry that largely dominated the field until the early 19th century according to Wikipedia. 

### How such a great person related to this project?
Euclid is a project that aims to provide a modern, cross-platform solution for 3D rendering and graphics applications. The name "Euclid" was chosen to reflect the project's focus on geometry and spatial reasoning, as well as its commitment to precision and clarity in design.


---

## Monorepo

This repository hosts several apps/libraries that work together:

- **Euclid-Web** — Next.js frontend for the user interface (see `Euclid-Web/README.md`).
- **Euclid-Lib** — Core native library (C/C++) built with Premake.
- **Euclid-Lib-Debug** — Debug/experimental targets for the native library.
- **Euclid-App** — Desktop application (C# / Avalonia) that integrates the core.
- **Dependencies** — Third-party libs and headers.
- **Vendor/Binaries/Premake** — Vendored Premake binaries for project generation.
- **.k8s** — Kubernetes manifests for cluster deployment.
- **.github/workflows** — CI/CD pipelines (GitHub Actions).
- **Build.lua** — Root Premake script describing native projects.
- **LICENSE / PREMAKE_LICENCE** — MIT for the project; BSD-3-Clause for Premake.

> The exact folders can evolve; use this as a navigation guide and check README for details.

---

## Features

- **Modern web UI** with Next.js/React for fast iteration.
- **Native core** for rendering/graphics or compute-heavy work (Premake-driven C/C++).
- **Desktop app** using Avalonia (cross-platform) for rich local tooling.
- **Cloud-ready** with Docker images and Kubernetes manifests.
- **Automated CI/CD** with GitHub Actions (build, package, deploy).

---

## Requirements

- Node.js 20+ (frontend).
- A C/C++ toolchain and platform SDKs (native).
- .NET SDK (desktop app).
- Docker (for container builds).
- kubectl/helm (optional; for cluster deploys).

---

## Contributors

- Niyaz Nassyrov
- Nikita Muzychenko
- Dmytro Parkhomenko
- Furkan Tellioğlu