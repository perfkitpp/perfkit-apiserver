# Web API spec for perfkit-apiserver

1. perfkit의 net provider에서 모든 데이터 형식 + 시퀀스 번호만 정의 및 분류. API 서버는 json object 형태로만 
    데이터를 다루게 된다. Websocket으로 업데이트 있을 때마다 모든 클라이언트에 데이터 갱신 지속적으로 쏴주다가,
    신규 연결 또는 reset 시 전체 데이터를 보내는 식
2. net provider는 JSON 또는 BSON으로 데이터 송신 모드 설정 가능. 
3. 즉, net provider와 apiserver는 기본적으로 같은 데이터를 송신한다. 단, net provider는 연결 시 
    데이터를 consistent하게 client에게 보내고, 송신 마친 데이터는 flush -> 즉, 로컬에 캐시되지 않음, apiserver는
    데이터를 캐시하고 + web interface 제공
4. 따라서, API 스펙은 먼저 net-provider에 대해 완전해야 하며, API server 없이도 순수 소켓 연결을 통해 client가
    동작할 수 있어야 한다. 
5. net provider는 최초 생성 시 tcp server mode 또는 apiserver provider mode 선택 가능. 후자를 선택 시 
    apiserver에 연결 시도하고, 연결 성공하기 전까지는 fallback으로 tcp server mode로 동작

> 따라서 perfkit 쪽은 단일 연결만 유지하는 tcp 서버 하나 두고, 연결 갱신 또는 reset 요청마다 시퀀스 리셋하고
>  송신한 뒤 이후부터 업데이트 보내는 방식으로 구현

> 멀티 유저를 구현하려면 서버 쪽에도 로직이 추가되어야 하는데, 귀찮으므로 ... 그냥 서버는 중계기 역할만 하고,
>  (즉, 웹소켓으로 TCP 소켓 입출력을 단순히 forwarding) 
>  



## API: Session
BSON/JSON 통신 가능하며, 오브젝트 단위로 메시지 넘어간다

실제 데이터 형식은 BSON/JSON으로 퍼블리시되며, 편의상 

### HEADER
메시지 공통: 모든 메시지는 8바이트 ASCII 문자열로 시작한다.

첫 4바이트는 식별을 위한 헤더, 다음 4바이트는 Base64 인코딩 된 메시지 페이로드 길이

    <- 4 byte header -><- 4 byte base64 buffer length ->  
    o ` P %            c x Z 3

#### CLIENT -> SERVER
```json
{
  "type": "string; e.g. cmd:reset_cache",
  "parameter": {
    "key": "value"
  }
}
```
#### SERVER -> CLIENT
```json
{
  "type": "string; e.g. update:shell",
  "cache_fence": "integer; ",
  "payload": {
    "key": "value"
  }
}
```

### *rpc:login*
아이디/암호 페어로 로그인 시도

**REQ**
```yaml
parameter:
  id: string
  pw: string; sha-256 hashed
```

**REP**
```yaml
payload:
  success: boolean
```

### *cmd:reset_cache*
내부 캐시 상태를 업데이트한다.

```yaml
parameter:
  # none
```

### *cmd:push_command*
커맨드 푸시 요청
```yaml
parameter:
  command: string
```

### *rpc:suggest_command*
명령행 자동 완성 요청
**REQ**
```yaml
parameter:
  reply_to: integer; a hash to recognize rpc reply
  command: string
```

**REP**
```yaml
payload:
  reply_to: integer
  new_command: string
  candidates: list<string>
```

### *update:session_info*
세션 정보 
```yaml
payload:
  
```

### *update:session_state*


### *update:shell_output*
```yaml
payload:
  content: string
```

### *update:new_config_class*
```yaml
payload:
  key: string
  root: category_scheme

category_scheme:
  name: string
  children: list<category_scheme|entity_scheme>

entity_scheme:
  name: string
  key: integer; a unique key in a config class scope
  value: any; current value 
  metadata: 
    description: string
    default_value: any
    min: any
    max: any
    ...
```

### *update:config_entity*
```yaml
payload:
  class_key: string; name of config class
  content: list<entity_scheme>; 

entity_scheme:
  key: integer; unique key from 'update:new_config_class'
  value: any; new value
```

### *update:trace_class_list*
추가/제거된 트레이스 클래스가 있을 때, 전체 리스트를 다시 보냄
```yaml
payload:
  content: list<string>; list of new trace classes
```

### *cmd:signal_fetch_traces*
트레이스는 fetch를 위해 먼저 signaling이 필요 ... signaling 대상이 될 트레이스 리스트를 보낸다.
```yaml
parameter:
  targets: list<string>; list of trace classes 
```

### *update:traces*
```yaml
payload:
  class_name: string; name of trace class
  root: node_scheme
  
node_scheme:
  name: string
  key: integer; unique key in trace class scope
  subscribe: boolean; if subscribing this node
  children?: list<node_scheme>; if folded, field itself is skipped. 
```

### *cmd:control_trace*
각 트레이스 노드의 상태 전환 
```yaml
parameter:
  class_name: string; name of trace clsas
  
```

### *cmd:*
