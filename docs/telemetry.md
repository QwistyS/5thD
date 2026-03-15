# 5thD Telemetry and Security Event Schema

## Design Goals

- capture operational health and hostile behavior early
- keep security telemetry typed and queryable
- separate audit events from debug logs
- make phone alerts derive from structured events, not log scraping

## Event Classes

- `metric`
  Numeric time-series samples and counters.
- `trace`
  Request and workflow spans.
- `event`
  Structured domain facts.
- `audit`
  Security-sensitive actions with actor, target, and result.
- `anomaly`
  Derived detections produced by the security monitor.

## Common Event Envelope

Every event emitted inside 5thD should use the same envelope:

```json
{
  "event_id": "01HRZ2E4K7Y8W3P4M7N9Q0R1ST",
  "ts_utc": "2026-03-15T12:00:00Z",
  "class": "event",
  "name": "peer.invalid_frame",
  "severity": "high",
  "node_id": "node:4f2a...",
  "process": "5thd-peer",
  "component": "transport.frame_decoder",
  "module_id": null,
  "peer_id": "peer:3ac1...",
  "session_id": "sess:92b7...",
  "trace_id": "trace:17d2...",
  "span_id": "span:4aa0...",
  "outcome": "denied",
  "fields": {
    "frame_type": "route_announce",
    "reason": "length_mismatch",
    "remote_addr": "198.51.100.12:4040"
  }
}
```

## Severity

- `debug`
- `info`
- `low`
- `medium`
- `high`
- `critical`

Severity is for routing and alerting, not for replacing the event name.

## Required Security Event Families

### Pairing and Control Plane

- `security.phone_pairing_requested`
- `security.phone_pairing_succeeded`
- `security.phone_pairing_failed`
- `security.phone_command_denied`
- `security.phone_session_replayed`

### Peer and Transport

- `peer.connection_opened`
- `peer.connection_closed`
- `peer.auth_failed`
- `peer.invalid_frame`
- `peer.rate_limited`
- `peer.reputation_changed`

### Policy and Capabilities

- `security.policy_denied`
- `security.capability_denied`
- `security.module_signature_invalid`
- `security.module_install_blocked`

### Storage and Secrets

- `storage.db_open_failed`
- `storage.integrity_check_failed`
- `security.secret_access_denied`
- `security.secret_rotation_started`
- `security.secret_rotation_completed`

### Module Runtime

- `module.started`
- `module.stopped`
- `module.crashed`
- `module.restart_suppressed`
- `module.resource_limit_hit`

## Audit Events

Audit events are append-only and must include actor identity.

Required audit names:

- `audit.phone.login`
- `audit.phone.pair`
- `audit.module.install`
- `audit.module.remove`
- `audit.module.permission_change`
- `audit.policy.change`
- `audit.key.export_attempt`
- `audit.node.shutdown`

Audit fields must include:

- actor id
- actor type
- target object
- requested action
- decision
- reason

## Metrics Baseline

At minimum collect:

- peer connections active
- auth failures per peer
- invalid frames per peer and frame type
- module CPU and memory usage
- queue depth on internal bus
- DB latency and DB failure rate
- phone command latency
- restart counts by process and module

## Trace Boundaries

Trace these flows end to end:

- phone command to node action
- peer message receipt to module delivery
- module request to storage commit
- module install and upgrade workflow

## Alert Routing

The security monitor should escalate these directly to the phone HUD:

- repeated pairing failures
- invalid module signature
- critical integrity check failure
- repeated invalid frames from same peer set
- module crash loop
- secret access denial spike

## Logging Rule

Free-form logs are still useful for local debugging, but they are secondary output. Any event that matters for security, availability, or audit must be emitted as a structured event first.
