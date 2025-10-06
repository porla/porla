POST http://localhost:1337/api/v1/auth/init
{
    "username": "test",
    "password": "test"
}

POST http://localhost:1337/api/v1/auth/login
{
    "username": "test",
    "password": "test"
}
HTTP 200

POST http://localhost:1337/api/v1/jsonrpc
{
    "id": 2,
    "method": "sessions.list",
    "params": {}
}
HTTP 200
[Asserts]
jsonpath "$.result.sessions" count == 1
jsonpath "$.result.sessions[0].name" == "default"
jsonpath "$.result.sessions[0].torrents_total" == 0
[Captures]
default-session-id: jsonpath "$.result.sessions[0].id"

POST http://localhost:1337/api/v1/jsonrpc
{
    "id": 2,
    "method": "sessions.remove",
    "params": {
        "id": {{default-session-id}}
    }
}
HTTP 200
[Asserts]
jsonpath "$.error.code" == -2

POST http://localhost:1337/api/v1/jsonrpc
{
    "id": 2,
    "method": "sessions.add",
    "params": {
        "name": "test-session"
    }
}
HTTP 200
[Captures]
test-session-id: jsonpath "$.result.id"

POST http://localhost:1337/api/v1/jsonrpc
{
    "id": 2,
    "method": "sessions.list",
    "params": {}
}
HTTP 200
[Asserts]
jsonpath "$.result.sessions" count == 2

POST http://localhost:1337/api/v1/jsonrpc
{
    "id": 2,
    "method": "sessions.remove",
    "params": {
        "id": {{test-session-id}}
    }
}

HTTP 200
POST http://localhost:1337/api/v1/jsonrpc
{
    "id": 2,
    "method": "sessions.list",
    "params": {}
}
HTTP 200
[Asserts]
jsonpath "$.result.sessions" count == 1
