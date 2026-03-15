# 5thD Module Contract

## Purpose

Modules are the feature layer of 5thD. The core node provides identity, storage, transport, policy, and telemetry. Modules provide user-facing capabilities such as chat, store, bids, and streams.

The contract below is intentionally strict. Modules are untrusted until granted capabilities.

## Module Lifecycle

Every module implements the following lifecycle:

1. `probe`
   Validate binary compatibility and manifest schema.
2. `init`
   Receive granted handles for storage, networking, telemetry, and identity services.
3. `start`
   Begin serving APIs, timers, jobs, and subscriptions.
4. `quiesce`
   Stop accepting new work and drain in-flight requests.
5. `stop`
   Release resources.
6. `upgrade`
   Run manifest-validated migrations.
7. `remove`
   Clean module-owned resources according to retention policy.

## Module Manifest

Each module must ship with a signed manifest.

```yaml
module_id: "store"
display_name: "Store"
version: "0.1.0"
api_version: "1"
entrypoint: "module_store.so"
capabilities:
  - storage.kv
  - storage.blob
  - net.outbound
  - net.inbound.service
  - telemetry.emit
  - ui.notify
resources:
  memory_mb: 256
  cpu_shares: 512
  storage_mb: 2048
  outbound_kbps: 2048
storage:
  namespace: "store"
  retention: "persistent"
network:
  service_name: "store.api"
  listen_policy: "private"
telemetry:
  event_prefix: "module.store"
  pii_policy: "user-content-redacted"
upgrade:
  migration_api: "1"
permissions:
  phone_controls:
    - "module.start"
    - "module.stop"
    - "module.configure"
```

## Capability Model

Capabilities are explicit and deny-by-default.

Base capability classes:

- `storage.kv`
  Namespaced key-value storage for the module.
- `storage.blob`
  Namespaced object storage with quotas.
- `net.outbound`
  Outbound peer or internet connections according to policy.
- `net.inbound.service`
  Expose a service endpoint through the node router.
- `identity.sign`
  Request signatures from node-managed keys.
- `identity.decrypt`
  Request decryption through node-managed keys.
- `telemetry.emit`
  Emit structured metrics and events.
- `ui.notify`
  Send user-visible notifications to the phone HUD.
- `admin.workflow`
  Reserved; should almost never be granted to third-party modules.

Rules:

- modules never receive unrestricted filesystem access
- modules never bind arbitrary ports directly
- modules never get ambient network access
- modules never read another module's storage namespace
- `identity.decrypt` and `identity.sign` must be mediated through explicit service calls

## Required Interfaces

The host should expose a narrow SDK. A module talks to the host through interfaces similar to:

```cpp
struct ModuleContext {
    IStateStore* state_store;
    IBlobStore* blob_store;
    IRouter* router;
    IIdentityService* identity;
    ITelemetrySink* telemetry;
    IPolicyView* policy;
};

struct Module {
    virtual ModuleInfo probe() = 0;
    virtual VoidResult init(const ModuleContext& ctx) = 0;
    virtual VoidResult start() = 0;
    virtual VoidResult quiesce() = 0;
    virtual VoidResult stop() = 0;
    virtual VoidResult upgrade(const ModuleUpgradePlan& plan) = 0;
    virtual ~Module() = default;
};
```

The important part is not the exact C++ shape. The important part is that the module only sees granted handles, not global singletons.

## Internal Communication Model

There are two communication styles:

- commands
  Directed requests with an expected outcome, such as `module.start` or `peer.connect`.
- events
  Immutable facts that already happened, such as `peer.connected` or `security.policy_denied`.

Modules may subscribe only to allowed event classes and may publish only within their own namespace.

Examples:

- module command: `module.store.publish_item`
- core event: `peer.session_established`
- security event: `security.invalid_frame`
- telemetry event: `telemetry.resource_pressure`

## Isolation Policy

At minimum, each module must be isolated by:

- separate process or sandboxed runtime
- explicit memory and CPU quotas
- namespaced storage handles
- routed network access instead of raw socket ownership
- structured stderr/stdout capture into telemetry

If an untrusted module crashes, loops, or leaks resources, the module host must be able to kill and restart it without taking down the node.

## Phone HUD Interaction

The phone HUD controls modules but does not host them. The allowed phone actions are:

- install approved module
- enable or disable module
- configure module settings
- view health, quota use, and alerts
- approve privileged workflows defined by policy

The phone may never issue raw module-internal commands that bypass policy.

## Versioning Rules

- core SDK version must be semantically versioned
- module manifests must declare `api_version`
- upgrades that require data migration must register a migration step
- incompatible modules must fail during `probe`, not after partial start

## Example Feature Mapping

- `chat`
  Needs `storage.kv`, `net.outbound`, `net.inbound.service`, `telemetry.emit`, `ui.notify`
- `store`
  Needs `storage.kv`, `storage.blob`, `net.inbound.service`, `telemetry.emit`
- `stream`
  Needs `storage.blob`, `net.outbound`, `net.inbound.service`, `telemetry.emit`

## Security Hooks Required For Every Module

Every module must emit:

- `module.<id>.started`
- `module.<id>.stopped`
- `module.<id>.crashed`
- `module.<id>.permission_denied`
- `module.<id>.resource_limit_hit`
- `module.<id>.config_changed`

Every module must tolerate:

- capability revocation
- host restarts
- partial network availability
- replayed startup requests
