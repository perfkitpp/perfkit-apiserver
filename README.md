# Perfkit API Server

## Mechanism

- Sequence IDs: Used to identify to find if request result is cached in API server. Otherwise, the request will be
  forwarded to the provider.

## Progress: Provider - API Server Communication

- [ ] Session Registering
- [ ] Shell IO Transfer
- [ ] Configuration Transfer
- [ ] Trace Trasnfer

## Progress: API Server

## Progress: WebAPI

[List of APIs](doc/webapi.md)

- [ ] Authentication
- [ ] Session Browse API
- [ ] Shell
    - [ ] Keystroke Transfer
    - [ ] Line output stream
- [ ] Configs
    - [ ] Browse All
    - [ ] Fetch Patches
    - [ ] Modify
- [ ] Traces
    - [ ] List Groups
    - [ ] Fetch Group
    - [ ] Subscribe
    - [ ] Interactive Images
        - [ ] Fetch Jpeg
        - [ ] Post Interaction

