# Perfkit API Server

```text
          -  xxxx   xxxxx  xxxx     xxxxxx  xx  xx   xxxx xxxxxx
         -  xx  xx xx     xx  xx   xx      xx  xx    xx    xx
        -  xxxxxx xxxxx  xx xx    xxxxx   xx xx     xx    xx
       -  xx     xx     xx  xx   xx      xx   x    xx    xx
      -  xx     xxxxxx xx    xx xx      xx    xx xxxx   xx
    ───────────────────────────────────────────────────────────

   <FRONTEND>

  ┌────────────────┬─┬┬─────────────────────┐
  │PERFKIT         │ ││ Session List Viewer ├─────────────────────────────────┐
  │ Client Instance│ └┴─────────────────────┘                                 │
  └────────────────┤                                                          │
                   └─┬┬────────────────┐                                      │
                     ││ Session Viewer │                                      │
                     └┴───▲────────────┘                                      │
                          │              ┌───────────────┐                    │
                          │              │ <<interface>> │    <<impl>>        │
                          ├──────<<by>>──┤  bytestream   ◄──┬───────┐         │
                          │              └───────────────┘  │       │         │
   <BACKEND>              │                                 │       │         │
                       ┌──▼───────────────────────┐  ┌──────┴───┐ ┌─┴───────┐ │
                       │   <<interface>>          │  │tcp client│ │websocket│ │
                       │    session management    │  └──┬───────┘ │ client  │ │
                       │     bytestrem protocol   │     │         └─┬───────┘ │
   ┌────────────────┐  └▲─────────────────────────┘     │           │         │
   │    perfkit     │   │                               │           │         │
   │    instance    │   │                               │           │         │
   ├────────────────┘   │<<impl>>                       │           │         │
   │                    │                               │           │         │
   │                    │                               │           │         │
   ├┬──────────────────┬┘                               │           │         │
   ││  tcp socket      │                                │           │         │
   ││  session server  │◄───────────────────────────────┘           │         │
   └┴──────▲───────────┘                                            │         │
           │                                                        │         │
           │                                                        │         │
           │    ┌────────────┐                                      │         │
           └────┤ perfkit    │    ┌─────────────────────────────────┘         │
                │  apiserver │    │                                           │
                └────────┬───┘    │                                           │
                         │        │                                           │
                         ├┬───────▼───┐                                       │
                         ││ websocket │                                       │
                         ││  server   │                                       │
                         ├┴───────────┘                                       │
                         │                                                    │
                         ├┬─────────────┐                                     │
                         ││ RESTful     ◄─────────────────────────────────────┘
                         ││   API server│
                         └┴─────────────┘
```

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

[List of APIs](third/perfkit/doc/net-api.md)
[List of WEBAPIs](doc/webapi.md)

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

