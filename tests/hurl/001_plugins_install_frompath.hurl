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
    "id": 1,
    "method": "plugins.install",
    "params": {
        "type": "path",
        "data": "/workspaces/porla/tests/plugins/basic",
        "config": "return { x = 1 }"
    }
}
HTTP 200
[Captures]
plugin-id: jsonpath "$['result']['id']"

POST http://localhost:1337/api/v1/jsonrpc
{
    "id": 2,
    "method": "plugins.uninstall",
    "params": {
        "id": {{plugin-id}}
    }
}
HTTP 200
