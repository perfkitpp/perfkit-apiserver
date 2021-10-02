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
    "some-session-id": {
      "ip": "192.168.2.2",
      "pid": "31119",
      "host-name": "DESKTOP-3GX199S",
      "require-auth": false,
      "description": "some description"
    },
    "some-other-session": {
      "...": null
    }
  }
}
```

### `GET /shell/<session>/<sequence>`

- Response

```json
{
  "begin": 131,
  "end": 114511,
  "content": "string contents ...."
}
```

### `POST /shell/<session>`

- Response

```json

```

### `GET /config/all/<session>`

- Response

```json
{
  "configs": {
    "category-1": {
      "type": "category",
      "category-2": {
        "type": "category",
        "config-1": {
          "hash": 13948171,
          "type": "int",
          "value": 1331,
          "description": "some var",
          "attribute": {
            "min": 0,
            "one-of": [
              1,
              2,
              14,
              1331
            ]
          }
        }
      }
    }
  }
}
```

### `PATCH /config/<session>/<hash>`

Target can be an object.

```json
{
  "value": "new-value"
}
```

### `GET /config/patch/<session>/<last-sequence-id>`

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

### `GET /trace/list/<session>`

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

### `GET /trace/<session>/<group>/<last-sequence-id>`

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

### `PUT /trace/<session>/<hash>`

```json
{
  "fold": true,
  "subscribe": false
}
```

### `GET /trace-image/<session>/<hash>`

```json
"base64-jpeg-data"
```

### `PUT /trace-image-interaction/<session>/<hash>`

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
