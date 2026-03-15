# 5thD

5thD is a sovereign personal edge node: a home-hosted backend paired to a user's phone that gives the user ownership of data, services, and policy. The phone acts as the HUD and control plane. The node runs the real backend runtime and exposes modular features such as chat, store, bids, streams, and future user-defined services.

The project target is not a single-purpose chat server. It is a modular decentralized runtime with a strong security and telemetry plane.

## Features

- **User-owned backend:** the node runs in the user's environment and keeps the user in control of data and feature exposure.
- **Phone HUD:** the phone is the control surface for health, policy, alerts, and module management.
- **Modular feature runtime:** user-facing capabilities such as chat, store, and streams are modules, not core runtime code.
- **Decentralized networking:** peer transport and routing are built around decentralized communication rather than a central service.
- **Security-first operation:** identity, policy, capability checks, audit, and anomaly detection are first-class concerns.
- **Heavy telemetry:** metrics, traces, audit events, and hostile-traffic signals are part of the platform design.

## Architecture Docs

- [Target Architecture](docs/architecture.md)
- [Module Contract](docs/module_contract.md)
- [Telemetry and Security Event Schema](docs/telemetry.md)

## Getting Started

To use 5thD, follow these steps:

1. Clone the repository:
   ```bash
   git clone git@github.com:QwistyS/5thD.git
   ```

   ```bash
   git submodule update --init --recursive
   ```
