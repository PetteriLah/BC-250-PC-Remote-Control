---
name: embedded-security-review
description: Perform authorized defensive security review for BC-250-PC-Remote-Control and WLED firmware, APIs, web UI, OTA, Wi-Fi provisioning, Bluetooth or MAC filtering, filesystem configuration, network protocols, dependencies, and CI workflows. Use for threat modeling, hardening, vulnerability review, security testing plans, or remediation across either repository.
---

# Embedded security review

Review the actual embedded trust boundaries and fail-safe behavior with low-noise, evidence-backed findings.

## Workflow

1. Confirm the repositories and local test instances in scope.
2. Read each repository's `AGENTS.md`; for WLED also read its hardening and secure-code instructions.
3. Read `references/threat-model.md`.
4. Identify first untrusted ingress points and trace data to state, memory, filesystem, GPIO, OTA, or workflow side effects.
5. Check authorization, validation, bounds, parse/allocation failure, secret handling, integrity, and recovery.
6. Separate confirmed vulnerabilities, design risks, and unverified hypotheses.
7. Report severity, file and line, prerequisites, impact, evidence, and remediation.

## Boundaries

- Remain local and read-only unless explicit remediation or active-test authorization is provided.
- Do not scan public hosts, third-party devices, GitHub, or the upstream project.
- Do not upload firmware, erase filesystems, change credentials, operate relays, persist access, exfiltrate data, or run disruptive load tests.
- Do not impose generic TLS requirements where the project's documented threat model does not require them.

## Review priorities

- Memory safety and integer/bounds validation.
- Authentication and authorization for state-changing HTTP/JSON paths.
- OTA integrity and firmware/filesystem target separation.
- Secret storage, API serialization, logs, and browser exposure.
- DOM injection and dynamic JavaScript execution.
- Fail-closed behavior on malformed input and allocation failure.
- Dependency and GitHub Actions supply-chain safety.
