---
name: wled-web-api
description: Develop and review WLED web UI, JSON API, WebSocket state, HTTP handlers, settings pages, and embedded asset generation. Use for wled00/data/, wled00/json.cpp, wled00/wled_server.cpp, wled00/ws.cpp, tools/cdata.js, generated-page mappings, API field changes, browser security, caching, authentication checks, or UI build tests.
---

# WLED web UI and JSON API

Change source assets and firmware contracts together while preserving the generated embedded UI pipeline.

## Workflow

1. Read the checkout's `AGENTS.md`, `docs/web.instructions.md`, security instructions, and `references/web-contract.md`.
2. Trace each field through UI source, HTTP or WebSocket ingress, JSON deserialization, internal state, and serialization.
3. Check configured PIN/auth enforcement for every state-changing route.
4. Edit only source assets under `wled00/data/`, never generated headers.
5. Reuse `common.js` helpers and use tab indentation in HTML, JS, and CSS.
6. Regenerate assets and run tests.

## Security and compatibility

- Validate and clamp state-changing values in firmware, not only in JavaScript.
- Treat fetched/config-derived strings as untrusted DOM input; prefer `textContent`.
- Do not add `eval`, `new Function`, string timers, unsafe redirects, or unchecked `postMessage`.
- Preserve JSON buffer locking and fail safely on parse or allocation errors.
- Do not expose values from `wsec.json` or password-like configuration.
- Preserve existing JSON field behavior unless a breaking change is explicitly approved and documented.

## Verification

- Run `npm ci`, `npm run build`, and `npm test`.
- Confirm expected generated header mapping without editing generated files.
- Exercise GET/POST and WebSocket paths for changed state fields.
- Check invalid, missing, oversized, and unauthorized inputs.
- Build at least `esp32dev` after generated asset or firmware-contract changes.
- Perform browser smoke tests at desktop and narrow viewport sizes when available.
