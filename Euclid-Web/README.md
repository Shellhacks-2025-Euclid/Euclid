# Euclid-Web

**Euclid-Web** is the web frontend for the Euclid project. It provides the user interface. The repository is a monorepo; this app lives in `Euclid-Web/`. You’ll also find CI workflows and Kubernetes manifests at the repo root for deployment.

## Features

* Next.js + React UI (TypeScript-ready).
* Shadcn UI components.
* Containerized build (Docker) and Kubernetes deployment on DigitalOcean.
* CI/CD via GitHub Actions (build → push image to DOCR → update DOKS).

---

## Requirements

* **Node.js 20+**
* **npm** (or **pnpm/yarn** if you prefer—adjust commands accordingly)
* (Deployment) **Docker**, **kubectl**, and access to **DigitalOcean** registry/cluster

---

## Quick start (local dev)

```bash
# from repo root
git clone https://github.com/Shellhacks-2025-Euclid/Euclid.git
cd Euclid/Euclid-Web

# install deps
npm install

# run the app
npm run dev
# visit http://localhost:3000
```

---

## Scripts

```bash
npm run dev      # start Next.js in development
npm run build    # production build
npm run start    # run the production build locally
npm run lint     # lint the codebase
```

---

## Project structure (typical)

```
Euclid-Web/
├─ package.json
├─ public/                    # static assets
├─ next.config.ts             # Next.js config
└─ src
    ├─ app/                   # routes and pages (using App Router)
    ├─ components/            # shared UI components
    └─ lib/                   # utility functions
```

*(Folder names may vary slightly; check the repo for the exact structure.)*

---

## CI/CD & Deployment

This frontend is built and shipped via **GitHub Actions**, containerized with **Docker**, and deployed to a **DigitalOcean Kubernetes (DOKS)** cluster. Container images are pushed to **DigitalOcean Container Registry (DOCR)**. CI/CD config lives under `.github/workflows/`; Kubernetes manifests live under `.k8s/`. ([GitHub][1])

### Pipeline outline

1. **Docker build** the production image of `Euclid-Web`.
2. **Push** image to **DOCR** (auth via repo secrets).
3. **Deploy** by updating the image in the DOKS **Deployment** (rolling update).

> You’ll find workflow YAMLs in `.github/workflows/` and manifests in `.k8s/`. Adapt image names, registry, and namespace to your environment. ([GitHub][1])

### One-off local Docker run (optional)

```bash
# build
docker build -t euclid-web:local .

# run
docker run -p 3000:3000 euclid-web:local
```

---

## Contributing

1. Create a feature branch from `master`.
2. Keep `npm run build` and `npm run lint` clean.
3. Open a PR.
4. Once approved, merge to `master`.

---

## License

The project is released under **MIT** (see `LICENSE` at the repo root).