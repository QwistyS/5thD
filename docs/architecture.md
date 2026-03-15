# 5thD Target Architecture

## Product Definition

5thD is a sovereign personal edge node: a home-hosted backend that is paired to a user's phone and gives that user ownership of data, services, and access policy.

The phone is the HUD and control plane. The home node is the long-running runtime. Feature sets such as chat, store, bids, streams, or future apps are modules running on top of the node.

## System Roles

- `5thd-node`: the main node runtime deployed on the user's home device.
- `5thd-phone-gateway`: secure control channel between the phone HUD and the node.
- `5thd-module-host`: isolated execution host for feature modules.
- `5thd-telemetryd`: metrics, traces, audit, anomaly detection, and alert fanout.
- `5thd-peer`: peer transport and routing worker for decentralized network traffic.
- `5thd-bus`: internal event and command bus between core services.

## Deployment Topology

```text
+-------------------+              +---------------------------+
| Phone HUD         | <----------> | 5thd-phone-gateway        |
| - settings        |  paired TLS  | - auth                    |
| - alerts          |              | - command validation      |
| - module control  |              | - sync                    |
+-------------------+              +-------------+-------------+
                                                 |
                                                 v
                                   +-------------+-------------+
                                   | 5thd-node                 |
                                   | - identity                |
                                   | - policy                  |
                                   | - storage                 |
                                   | - runtime supervisor      |
                                   +------+------+-------------+
                                          |      |
                         internal bus      |      | typed events
                                          v      v
                               +----------+--+  +----------------+
                               | 5thd-peer  |  | 5thd-telemetryd|
                               | transport  |  | metrics/audit   |
                               | routing    |  | anomaly engine  |
                               +------+-----+  +----------------+
                                      |
                                      v
                               decentralized network

                                   +----------------+
                                   | 5thd-module-host
                                   | - module sandbox
                                   | - quotas
                                   | - capability checks
                                   +----------------+
```

## Architectural Rules

1. Keep the core small. The core owns identity, transport, storage, policy, module hosting, and telemetry only.
2. All end-user features are modules. Chat, store, stream, bids, and future apps do not belong in the core runtime.
3. The phone HUD is never the source of truth for business state. The node owns state.
4. Modules do not get ambient access. Every privileged action requires a declared capability.
5. Telemetry and security are cross-cutting planes, not optional helper code.
6. Core services communicate through typed commands and typed events, not ad hoc logger strings.

## Core Domains

### Identity

Responsibilities:
- node identity and key hierarchy
- phone pairing
- peer trust material
- signing and decryption services
- backup and recovery hooks

Key interfaces:
- `IdentityService`
- `PairingService`
- `KeyStore`

### Transport

Responsibilities:
- peer sockets
- secure sessions
- NAT traversal and reachability
- packet framing
- backpressure and retries

Key interfaces:
- `PeerTransport`
- `SessionManager`
- `FrameCodec`

### Routing

Responsibilities:
- service discovery
- peer addressing
- message dispatch
- delivery policies
- peer reputation inputs

Key interfaces:
- `Router`
- `ServiceRegistry`
- `PeerDirectory`

### Storage

Responsibilities:
- encrypted local database
- blob/object storage
- module-owned state
- replication metadata
- retention policies

Key interfaces:
- `StateStore`
- `BlobStore`
- `SecretStore`

### Policy

Responsibilities:
- capability checks
- module install policy
- API exposure policy
- remote admin restrictions
- rate limits and abuse rules

Key interfaces:
- `PolicyEngine`
- `CapabilityGuard`
- `AdmissionController`

### Module Host

Responsibilities:
- load, start, stop, upgrade modules
- sandbox modules
- grant scoped handles
- enforce CPU, memory, storage, and network quotas

Key interfaces:
- `ModuleRegistry`
- `ModuleRuntime`
- `ModuleSupervisor`

### Telemetry and Security

Responsibilities:
- metrics and traces
- security event stream
- append-only audit trail
- anomaly detection
- phone notifications for critical events

Key interfaces:
- `TelemetrySink`
- `AuditLog`
- `SecurityMonitor`

## Process Boundaries

The initial implementation may keep some services in one process, but the interfaces must assume future split-out. The stable boundaries are:

- `5thd-node`
  Owns identity, policy, storage coordination, runtime supervision, and control APIs.
- `5thd-peer`
  Owns network sockets, routing, and peer lifecycle. It should be restartable without corrupting node state.
- `5thd-module-host`
  Runs modules with explicit capabilities and resource limits. A crashing module must not crash the node.
- `5thd-telemetryd`
  Accepts structured events from every process, persists audit records, and drives anomaly detection.
- `5thd-phone-gateway`
  Exposes a narrow control interface to the phone HUD. It is not a general-purpose app backend.

## Proposed Repository Layout

```text
5thD/
  apps/
    phone_hud/              # mobile-facing protocol, sync contracts, mock UI adapters
  services/
    node/                   # 5thd-node entrypoint and supervisor
    peer/                   # 5thd-peer process
    phone_gateway/          # 5thd-phone-gateway process
    telemetryd/             # 5thd-telemetryd process
    module_host/            # 5thd-module-host process
  core/
    identity/
    transport/
    routing/
    storage/
    policy/
    telemetry/
    security/
    bus/
    common/
  sdk/
    module_api/             # module-facing headers and manifest types
    telemetry_api/          # event schema helpers
  modules/
    chat/
    store/
    stream/
    marketplace/
  tests/
    unit/
    integration/
    adversarial/
    soak/
  docs/
    architecture.md
    module_contract.md
    telemetry.md
```

## Mapping From Current Repository

- `5thD_Peer/` should evolve into `services/peer/`.
- `5thD_Software_Bus/` should evolve into `core/bus/` or `services/node/` infrastructure depending on whether it remains in-process or becomes a sidecar.
- `core/common/` should shrink into shared primitives only.
- transport-specific code in `core/receiver.cpp`, `core/transmitter.cpp`, and `core/izmq.cpp` should move under `core/transport/`.
- database and key code should move under `core/storage/` and `core/identity/`.

## Implementation Order

1. Stabilize `core/identity`, `core/storage`, `core/transport`, and `core/bus`.
2. Introduce typed commands/events and stop using stringly coupled flows.
3. Add `sdk/module_api` with manifest parsing and capability enforcement.
4. Move peer runtime into `services/peer`.
5. Add `services/telemetryd` and make every subsystem emit structured events.
6. Build the first real module end to end on the new contract.

## Non-Negotiable Security Constraints

- private keys never leave the node unencrypted
- modules never receive raw master keys
- phone control channel must use mutual authentication
- security events must be append-only and tamper-evident
- malformed or hostile peer traffic must be observable as first-class telemetry
- every admin action must generate an audit event
