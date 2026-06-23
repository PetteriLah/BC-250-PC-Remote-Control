---
name: bc250-web-ui
description: Develop and review the BC-250 embedded HTTP API and LittleFS web interface. Use for routes in web_server.h; HTML, CSS, JavaScript, SVG, setup, status, power controls, or OTA UI under data/; and changes that must keep frontend fetch calls synchronized with firmware methods, JSON fields, and side effects.
---

# BC-250 web UI and API

Maintain the offline embedded UI and its firmware contract as one unit.

## Workflow

1. Read `references/http-contract.md`.
2. Search both `server.on(` and `fetch(` before changing a route.
3. Define method, request body, response type, status codes, and physical side effect.
4. Update firmware and all consumers together.
5. Preserve operation without external internet dependencies.
6. Keep polling bounded and avoid overlapping requests where practical.
7. Make UI success reflect accepted or confirmed device state accurately.

## API rules

- Use explicit HTTP methods for mutating routes.
- Validate JSON and required fields before side effects.
- Keep response content types correct.
- Never expose Wi-Fi credentials.
- Treat power and OTA routes as privileged and note the current lack of authentication.
- Do not rename a field or route without updating every consumer.

## Verification

- Match all `fetch()` calls to a registered route and method.
- Verify JSON fields used by `index.html`, `setup.html`, and `update.html`.
- Test AP mode without internet.
- Check mobile layout, keyboard operation, loading states, and errors.
- Check OTA selection, progress, server errors, and restart messaging.
- Run `git diff --check`; use browser testing when available.
