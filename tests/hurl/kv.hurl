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
    "method": "kv.get",
    "params": {
        "keys": [
            "test.foo"
        ]
    }
}
HTTP 200
[Asserts]
jsonpath "$.result.values['test.foo']" == null

POST http://localhost:1337/api/v1/jsonrpc
{
    "id": 2,
    "method": "kv.set",
    "params": {
        "values": {
            "test.foo": "bar"
        }
    }
}
HTTP 200

POST http://localhost:1337/api/v1/jsonrpc
{
    "id": 2,
    "method": "kv.get",
    "params": {
        "keys": [
            "test.foo"
        ]
    }
}
HTTP 200
[Asserts]
jsonpath "$.result.values['test.foo']" == "bar"
