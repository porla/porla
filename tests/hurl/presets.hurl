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
    "method": "presets.list",
    "params": {}
}
HTTP 200
[Asserts]
jsonpath "$.result.presets" count == 0

POST http://localhost:1337/api/v1/jsonrpc
{
    "id": 2,
    "method": "presets.add",
    "params": {
        "name": "test-preset"
    }
}
HTTP 200
[Captures]
preset-id: jsonpath "$.result.id"

POST http://localhost:1337/api/v1/jsonrpc
{
    "id": 2,
    "method": "presets.list",
    "params": {}
}
HTTP 200
[Asserts]
jsonpath "$.result.presets" count == 1
jsonpath "$.result.presets[0].id" == {{preset-id}}

POST http://localhost:1337/api/v1/jsonrpc
{
    "id": 2,
    "method": "presets.update",
    "params": {
        "id": {{preset-id}},
        "name": "test-preset-updated",
        "category": "test-category",
        "download_limit": 1337,
        "max_connections": 3,
        "max_uploads": 4,
        "metadata": {
            "magic": 80987
        },
        "session": "default",
        "save_path": "/tmp",
        "storage_mode": "sparse",
        "tags": ["foo", "bar"],
        "upload_limit": 1024
    }
}
HTTP 200

POST http://localhost:1337/api/v1/jsonrpc
{
    "id": 2,
    "method": "presets.get",
    "params": {
        "id": {{preset-id}}
    }
}
HTTP 200
[Asserts]
jsonpath "$.result.name" == "test-preset-updated"
jsonpath "$.result.category" == "test-category"
jsonpath "$.result.download_limit" == 1337
jsonpath "$.result.max_connections" == 3
jsonpath "$.result.max_uploads" == 4
jsonpath "$.result.metadata.magic" == 80987
jsonpath "$.result.tags" count == 2

POST http://localhost:1337/api/v1/jsonrpc
{
    "id": 2,
    "method": "presets.remove",
    "params": {
        "id": {{preset-id}}
    }
}
HTTP 200

POST http://localhost:1337/api/v1/jsonrpc
{
    "id": 2,
    "method": "presets.list",
    "params": {}
}
HTTP 200
[Asserts]
jsonpath "$.result.presets" count == 0
