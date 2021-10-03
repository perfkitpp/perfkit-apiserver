# List of APIs

### `POST /login`

Login to API server. It'll return an auth token.

- Request:

```json
{
  "id": "some-id",
  "pw": "SHA256 hashed password"
}
```

- Response:

```json
{
  "success": true,
  "token": "jwt-token"
}
```

### `GET /sessions`

- Response

```json
{
  "sessions": {
    "some-net_session-id": {
      "ip": "192.168.2.2",
      "pid": "31119",
      "machine-name": "DESKTOP-3GX199S",
      "epoch": 101231411,  
      "require-auth": false,
      "description": "some description"
    },
    "some-other-net_session": {
      "...": null
    }
  }
}
```

### `GET /shell/<net_session>/<sequence>`

- Response

```json
{
  "begin": "integer; character index begin",
  "end": "integer; character index end",
  "content": "string contents ...."
}
```

### `POST /shell/<net_session>`

```json
{
  "is_invoke": "bool; if set false, request suggest",
  "content": "string; user input"
}
```

```json
{
  "suggested": "?string; only valid if is_invoke==false"
}
```

### `GET /config/all/<net_session>`

- Response

```json
{
  "category-root": {
    "name": "root",
    "categories": [
      {
        "name": "sub1",
        "categories": [
          {
            "name": "entity1",
            "type": "string|object|number_int|number_float|array|object",
            "description": "description of this entity",
            "attribute": {
              "min": "some-minimal-value",
              "one-of": "some-table-entities",
              "max": "some-max-value"
            }
          }
        ]
      }
    ]
  }
}
```

### `PATCH /config/<net_session>/<hash>`

Target can be an object.

```json
{
  "value": "new-value"
}
```

### `GET /config/patch/<net_session>/<last-sequence-id>`

- Updates: `[[HASH, VALUE], [...]]`

```json
{
  "sequence": 141,
  "updates": [
    [
      13948171,
      1515
    ],
    [
      3148811,
      "updated value"
    ]
  ]
}
```

### `GET /trace/list/<net_session>`

```json
{
  "groups": [
    "list-name 1",
    "list-name 2",
    "list-name 3",
    "list-name 4"
  ]
}
```

### `GET /trace/<net_session>/<group>/<last-sequence-id>`

`GET /trace/example/list-name-1/1658`

```json
{
  "sequence": 1661,
  "entities": [
    {
      "name": "trace entity name",
      "hash": 102041040150,
      "subscribing": false,
      "type": "string",
      "value": "trace entity value",
      "subnodes": [
        {
          "name": "subnode 1 name",
          "type": "double",
          "hash": 34214111151,
          "subscribing": true,
          "value": 31.1141,
          "subnodes": {
            "...": "..."
          }
        }
      ]
    }
  ]
}
```

### `PUT /trace/<net_session>/<hash>`

```json
{
  "fold": true,
  "subscribe": false
}
```

### `GET /trace-image/<net_session>/<hash>`

```json
"base64-jpeg-data"
```

### `PUT /trace-image-interaction/<net_session>/<hash>`

Order is `[TYPE, VALUE]`

- `["KEY", "UP"|"DN", KEYCODE, [ATTRIBUTES: "SHIFT"|"ALT"|"CTRL"]]`
- `["MOUSE", "UP|DN|MV", "|M|L|R", [X, Y], [ATTRIBUTES]]`

```json
{
  "sequence": [
    [
      "KEY",
      "DOWN",
      65,
      []
    ],
    [
      "KEY",
      "DOWN",
      67,
      [
        "SHIFT"
      ]
    ],
    [
      "MOUSE",
      "MV",
      "",
      [
        314,
        113
      ],
      [
        "CTRL"
      ]
    ]
  ]
}
```
