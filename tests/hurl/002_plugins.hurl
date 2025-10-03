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
    "method": "plugins.list",
    "params": {}
}
HTTP 200
[Asserts]
jsonpath "$.result.plugins" count == 0

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
plugin-id: jsonpath "$.result.id"

POST http://localhost:1337/api/v1/jsonrpc
{
    "id": 3,
    "method": "plugins.list",
    "params": {}
}
HTTP 200
[Asserts]
jsonpath "$.result.plugins" count      == 1
jsonpath "$.result.plugins[0].id"      == {{plugin-id}}
jsonpath "$.result.plugins[0].name"    == "basic"
jsonpath "$.result.plugins[0].version" == "0.1"

POST http://localhost:1337/api/v1/jsonrpc
{
    "id": 3,
    "method": "plugins.get",
    "params": {
        "id": {{plugin-id}}
    }
}
HTTP 200
[Asserts]
jsonpath "$.result.id"       == {{plugin-id}}
jsonpath "$.result.name"     == "basic"
jsonpath "$.result.version"  == "0.1"
jsonpath "$.result.config"   == "return { x = 1 }"
jsonpath "$.result.metadata" == null

POST http://localhost:1337/api/v1/jsonrpc
{
    "id": 2,
    "method": "plugins.uninstall",
    "params": {
        "id": {{plugin-id}}
    }
}
HTTP 200

POST http://localhost:1337/api/v1/jsonrpc
{
    "id": 2,
    "method": "plugins.list",
    "params": {}
}
HTTP 200
[Asserts]
jsonpath "$.result.plugins" count == 0

##
# Installing a plugin from an archive
##

POST http://localhost:1337/api/v1/jsonrpc
{
    "id": 1,
    "method": "plugins.install",
    "params": {
        "type": "archive",
        "data": "UEsDBBQAAgAIAAJSQltPu1F9NQAAAD0AAAANABwAbWFuaWZlc3QudG9tbFVUCQADwzTeaMU03mh1eAsAAQT1AQAABBQAAADLTczMU7BVUCrIKU3PzNPLKU1U4uKKhvBiufISc1NBskmJxZnJSlxlqUXFmflg9QZ6hkpcAFBLAwQUAAIACABoUEJbBdudtDoAAABpAAAACgAcAHBsdWdpbi5sdWFVVAkAA8Qx3mjFMd5odXgLAAEE9QEAAAQUAAAASyvNSy7JzM9TKMgvyknUy8zLLNHQ5FIAgoKizLwSBSWEuBJXal4KF1caqo6U1OKSovxKbJqgUhB9AFBLAQIeAxQAAgAIAAJSQltPu1F9NQAAAD0AAAANABgAAAAAAAEAAACkgQAAAABtYW5pZmVzdC50b21sVVQFAAPDNN5odXgLAAEE9QEAAAQUAAAAUEsBAh4DFAACAAgAaFBCWwXbnbQ6AAAAaQAAAAoAGAAAAAAAAQAAAKSBfAAAAHBsdWdpbi5sdWFVVAUAA8Qx3mh1eAsAAQT1AQAABBQAAABQSwUGAAAAAAIAAgCjAAAA+gAAAAAA"
    }
}
HTTP 200
[Captures]
plugin-id: jsonpath "$.result.id"

POST http://localhost:1337/api/v1/jsonrpc
{
    "id": 2,
    "method": "plugins.list",
    "params": {}
}
HTTP 200
[Asserts]
jsonpath "$.result.plugins" count == 1