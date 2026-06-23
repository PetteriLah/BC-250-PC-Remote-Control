# WLED web and API contract

## Source and generation

- Source UI: `wled00/data/`.
- Generator: `tools/cdata.js`, invoked by `npm run build`.
- PlatformIO pre-build hook: `pio-scripts/build_ui.py`.
- Tests: `node --test`, principally `tools/cdata-test.js`.
- Generated `wled00/html_*.h` and `wled00/js_*.h` are build products and must not be edited or committed.

## Firmware endpoints

- `wled00/wled_server.cpp`: page routes, settings, uploads, OTA, `/json`, static content, PIN checks.
- `wled00/json.cpp`: JSON state/info serialization and deserialization.
- `wled00/ws.cpp`: WebSocket state ingress and broadcast.
- `wled00/set.cpp`: legacy/settings form processing.

## Shared-state constraints

JSON handling uses a shared buffer protected through `requestJSONBufferLock()`. Preserve lock acquisition and release paths. State-changing requests must fail safely when parsing, allocation, or authorization fails.

## UI conventions

- Tabs for HTML, JS, and CSS.
- Reuse `common.js`.
- Keep embedded pages functional without depending on an external backend.
- Preserve cache and ETag behavior unless the change explicitly targets caching.
